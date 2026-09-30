#!/usr/bin/env python3
"""Cache and public-snapshot guard regressions in disposable Git fixtures."""
import contextlib
import importlib.util
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


root = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('clean_builds', root / 'tools/clean_builds.py')
cleanup = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cleanup)
snapshot_spec = importlib.util.spec_from_file_location(
    'public_snapshot', root / 'tools/public_snapshot.py')
snapshot = importlib.util.module_from_spec(snapshot_spec)
snapshot_spec.loader.exec_module(snapshot)


class CleanupGuards(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()
        subprocess.run(['git', 'init', '-q', str(self.root)], check=True)
        (self.root / 'tests').mkdir()
        (self.root / 'dist').mkdir()
        (self.root / 'dist/DIFFEQ.g3a').write_bytes(b'final-output')
        (self.root / 'docs/reference').mkdir(parents=True)
        (self.root / 'docs/reference/manual.pdf').write_bytes(b'reference')

    def make_cache(self, name='build-host', source='tests'):
        cache = self.root / name
        (cache / 'CMakeFiles').mkdir(parents=True)
        (cache / 'CMakeCache.txt').write_text(
            f'CMAKE_HOME_DIRECTORY:INTERNAL={self.root / source}\n')
        (cache / 'CMakeFiles/compiled.o').write_bytes(b'generated-object')
        return cache

    def test_dry_run_preserves_cache_and_protected_outputs(self):
        cache = self.make_cache()
        with contextlib.redirect_stdout(io.StringIO()) as output:
            cleanup.clean_builds(self.root, ['build-host'])
        self.assertIn('WOULD REMOVE build-host', output.getvalue())
        self.assertTrue(cache.exists())
        self.assertEqual((self.root / 'dist/DIFFEQ.g3a').read_bytes(), b'final-output')

    def test_cli_defaults_to_dry_run_without_path_arguments(self):
        cache = self.make_cache()
        tool = self.root / 'tools/clean_builds.py'
        tool.parent.mkdir()
        tool.write_text((root / 'tools/clean_builds.py').read_text())
        result = subprocess.run([sys.executable, str(tool)],
                                capture_output=True, text=True, check=True)
        self.assertIn('WOULD REMOVE build-host', result.stdout)
        self.assertTrue(cache.exists())

    def test_apply_removes_only_exact_cache(self):
        cache = self.make_cache()
        retained = self.make_cache('build-cg', '.')
        with contextlib.redirect_stdout(io.StringIO()):
            cleanup.clean_builds(self.root, ['build-host'], apply=True)
        self.assertFalse(cache.exists())
        self.assertTrue(retained.exists())
        self.assertEqual((self.root / 'docs/reference/manual.pdf').read_bytes(), b'reference')

    def test_root_source_sdk_dist_and_traversal_are_rejected(self):
        for name in ('.', 'tests', '.local/src', '.local/prefix', 'dist',
                     'docs/reference', 'build-host/../tests', 'build*', '/tmp'):
            with self.subTest(path=name), self.assertRaises(ValueError):
                cleanup.inspect_build(self.root, name)

    def test_missing_cache_markers_are_rejected(self):
        (self.root / 'build-host').mkdir()
        with self.assertRaisesRegex(ValueError, 'CMake cache'):
            cleanup.inspect_build(self.root, 'build-host')

    def test_wrong_source_root_is_rejected(self):
        self.make_cache(source='.')
        with self.assertRaisesRegex(ValueError, 'source directory'):
            cleanup.inspect_build(self.root, 'build-host')

    def test_tracked_cache_file_is_rejected(self):
        self.make_cache()
        subprocess.run(['git', '-C', str(self.root), 'add',
                        'build-host/CMakeFiles/compiled.o'], check=True)
        with self.assertRaisesRegex(ValueError, 'tracked'):
            cleanup.inspect_build(self.root, 'build-host')

    def test_symlink_candidate_ancestor_and_contents_are_rejected(self):
        original = self.make_cache('build-cg', '.')
        (self.root / 'build-host').symlink_to(original, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            cleanup.inspect_build(self.root, 'build-host')
        (self.root / 'build-host').unlink()
        cache = self.make_cache()
        (cache / 'link').symlink_to(self.root / 'dist/DIFFEQ.g3a')
        with self.assertRaisesRegex(ValueError, 'symlink'):
            cleanup.inspect_build(self.root, 'build-host')
        (self.root / 'build').symlink_to(original, target_is_directory=True)
        with self.assertRaisesRegex(ValueError, 'symlink'):
            cleanup.inspect_build(self.root, 'build/host')

    def test_all_paths_validate_before_any_deletion(self):
        cache = self.make_cache()
        self.make_cache('build-cg', 'tests')
        with self.assertRaises(ValueError):
            cleanup.clean_builds(self.root, ['build-host', 'build-cg'], apply=True)
        self.assertTrue(cache.exists())

    def test_nested_repository_is_rejected(self):
        cache = self.make_cache()
        (cache / 'nested/.git').mkdir(parents=True)
        with self.assertRaisesRegex(ValueError, 'Git repository'):
            cleanup.inspect_build(self.root, 'build-host')


class SnapshotGuards(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.workspace = Path(self.temporary.name).resolve()
        self.source = self.workspace / 'source'
        self.destination = self.workspace / 'public'
        for directory in (self.source, self.destination):
            directory.mkdir()
            self.git(directory, 'init', '-q', '-b', 'main')
            for name in snapshot.ROOT_FILES:
                self.write(directory, name, f'Fixture document: {name}\n'.encode())
            self.write(directory, '.gitattributes', b'* text=auto\n')
            self.write(directory, 'LICENSE', b'MIT fixture notice\n')
            self.write(directory, 'THIRD_PARTY_NOTICES.md', b'Original fixture notice\n')

        self.write(self.source, 'src/main.c', b'int fixture(void) { return 1; }\n')
        self.write(self.source, 'examples/hello/CMakeLists.txt', b'# retained smoke target\n')
        self.write(self.source, 'examples/hello/src/main.c', b'int main(void) { return 0; }\n')
        self.write(self.source, 'examples/hello/assets-cg/icon-uns.png', b'fixture-image')
        self.write(self.source, 'examples/hello/DIFFEQ-hello.g3a', b'private-final-package')
        self.write(self.source, 'examples/hello/build-cg/CMakeCache.txt', b'private-cache')
        self.write(self.source, 'docs/reference/manual.pdf', b'protected-reference')
        self.write(self.source, 'docs/archive/build-logs/old.log', b'private-build-log')
        self.write(self.source, 'docs/archive/ENVIRONMENT_HISTORY.md', b'private-environment')
        self.write(self.source, '.local/secret.txt', b'private-local-file')
        self.write(self.destination, 'src/obsolete.c', b'old-public-source\n')
        self.commit(self.source)
        self.commit(self.destination)
        self.git(self.destination, 'remote', 'add', 'origin',
                 'https://github.com/omegalpha210/fx-cg50-diffeq.git')
        self.git(self.destination, 'tag', 'fixture-old-release')

    @staticmethod
    def git(directory, *arguments):
        return subprocess.check_output(
            ['git', '-C', str(directory), *arguments], stderr=subprocess.PIPE)

    def commit(self, directory):
        self.git(directory, 'add', '-A')
        self.git(directory, '-c', 'user.name=Fixture', '-c',
                 'user.email=fixture@example.invalid', 'commit', '-qm', 'fixture')

    @staticmethod
    def write(directory, name, data):
        path = directory / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path

    def files(self, directory):
        return {str(path.relative_to(directory)): path.read_bytes()
                for path in directory.rglob('*')
                if path.is_file() and '.git' not in path.relative_to(directory).parts}

    def prepare(self, **options):
        with contextlib.redirect_stdout(io.StringIO()):
            return snapshot.prepare(self.source, self.destination, **options)

    def rejected_without_mutation(self, message):
        before = self.files(self.destination)
        revision = self.git(self.destination, 'rev-parse', 'HEAD')
        tags = self.git(self.destination, 'show-ref', '--tags')
        with self.assertRaisesRegex(ValueError, message):
            self.prepare(apply=True)
        self.assertEqual(self.files(self.destination), before)
        self.assertEqual(self.git(self.destination, 'rev-parse', 'HEAD'), revision)
        self.assertEqual(self.git(self.destination, 'show-ref', '--tags'), tags)

    def test_dry_run_reads_committed_source_and_preserves_destination(self):
        before = self.files(self.destination)
        committed = (self.source / 'src/main.c').read_bytes()
        self.write(self.source, 'src/main.c', b'uncommitted-edit\n')
        self.write(self.source, 'src/untracked.c', b'untracked-user-edit\n')
        selected = self.prepare()
        self.assertEqual(selected['src/main.c'][0], committed)
        self.assertNotIn('src/untracked.c', selected)
        self.assertEqual(self.files(self.destination), before)
        self.assertEqual(self.git(self.destination, 'status', '--porcelain'), b'')
        self.assertEqual((self.source / 'src/main.c').read_bytes(), b'uncommitted-edit\n')

    def test_hello_sources_are_allowed_and_private_outputs_are_excluded(self):
        selected = self.prepare()
        for name in ('examples/hello/CMakeLists.txt', 'examples/hello/src/main.c',
                     'examples/hello/assets-cg/icon-uns.png'):
            self.assertIn(name, selected)
        for name in ('examples/hello/DIFFEQ-hello.g3a',
                     'examples/hello/build-cg/CMakeCache.txt',
                     'docs/reference/manual.pdf', 'docs/archive/build-logs/old.log',
                     'docs/archive/ENVIRONMENT_HISTORY.md', '.local/secret.txt'):
            self.assertNotIn(name, selected)

    def test_apply_preserves_commits_tags_notices_and_executable_mode(self):
        script = self.write(self.source, 'tools/fixture.sh', b'#!/bin/sh\nexit 0\n')
        script.chmod(0o755)
        self.commit(self.source)
        source_revision = self.git(self.source, 'rev-parse', 'HEAD')
        public_revision = self.git(self.destination, 'rev-parse', 'HEAD')
        tags = self.git(self.destination, 'show-ref', '--tags')
        selected = self.prepare(apply=True)
        self.assertFalse((self.destination / 'src/obsolete.c').exists())
        self.assertEqual((self.destination / 'src/main.c').read_bytes(),
                         selected['src/main.c'][0])
        self.assertEqual((self.destination / 'tools/fixture.sh').stat().st_mode & 0o777,
                         0o755)
        for name in ('LICENSE', 'THIRD_PARTY_NOTICES.md'):
            self.assertEqual((self.destination / name).read_bytes(), selected[name][0])
        self.assertEqual(self.git(self.source, 'rev-parse', 'HEAD'), source_revision)
        self.assertEqual(self.git(self.destination, 'rev-parse', 'HEAD'), public_revision)
        self.assertEqual(self.git(self.destination, 'show-ref', '--tags'), tags)

    def test_dirty_destination_is_rejected(self):
        self.write(self.destination, 'README.md', b'user-public-edit\n')
        self.rejected_without_mutation('uncommitted')

    def test_unexpected_origin_is_rejected(self):
        self.git(self.destination, 'remote', 'set-url', 'origin',
                 'https://example.invalid/unrelated.git')
        self.rejected_without_mutation('unexpected public origin')

    def test_non_main_destination_is_rejected(self):
        self.git(self.destination, 'checkout', '-qb', 'unpublished-work')
        self.rejected_without_mutation('must be on main')

    def test_changed_license_or_notice_is_rejected(self):
        for name in ('LICENSE', 'THIRD_PARTY_NOTICES.md'):
            with self.subTest(document=name):
                original = (self.source / name).read_bytes()
                self.write(self.source, name, b'changed-original-notice\n')
                self.commit(self.source)
                self.rejected_without_mutation('original license/notice changed')
                self.write(self.source, name, original)
                self.commit(self.source)

    def test_destination_symlink_ancestor_is_rejected(self):
        outside = self.workspace / 'outside'
        outside.mkdir()
        self.write(outside, 'sentinel', b'outside-preserved')
        (self.destination / 'examples').symlink_to(outside, target_is_directory=True)
        self.commit(self.destination)
        self.rejected_without_mutation('symlink in destination')
        self.assertEqual((outside / 'sentinel').read_bytes(), b'outside-preserved')

    def test_source_symlink_is_rejected(self):
        (self.source / 'src/link').symlink_to(self.source / 'LICENSE')
        self.commit(self.source)
        self.rejected_without_mutation('non-regular source entry')

    def test_artifact_inside_allowed_directory_is_rejected(self):
        self.write(self.source, 'src/accidental.o', b'generated-object')
        self.commit(self.source)
        self.rejected_without_mutation('excluded artifact')

    def test_credentials_in_committed_source_are_rejected(self):
        self.write(self.source, 'src/accidental.c', b'ghp_' + b'x' * 36)
        self.commit(self.source)
        self.rejected_without_mutation('private path or credential')

    def test_private_content_and_artifact_patterns_are_rejected(self):
        # Assemble synthetic markers at runtime so these safe tests do not
        # themselves embed a complete credential or personal absolute path.
        private = (b'/Users/' + b'fixture/private',
                   b'/home/' + b'fixture/private',
                   b'-----BEGIN ' + b'PRIVATE KEY-----',
                   b'github_pat_' + b'x' * 45,
                   b'AKIA' + b'A' * 16,
                   b'sk-' + b'x' * 36,
                   b'xoxb-' + b'x' * 25)
        for marker in private:
            with self.subTest(marker=marker[:8]), self.assertRaises(ValueError):
                snapshot.check_content('src/fixture.c', marker)
        for extension in ('pdf', 'g3a', 'elf', 'o', 'obj', 'log', 'map', 'su', 'ppm'):
            with self.subTest(extension=extension), self.assertRaises(ValueError):
                snapshot.check_content(f'assets/fixture.{extension}', b'fixture')

    def test_incomplete_source_is_rejected(self):
        (self.source / 'PROJECT_STRUCTURE.md').unlink()
        self.commit(self.source)
        self.rejected_without_mutation('required root')

    def test_same_or_nested_repository_root_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'separate existing checkout'):
            snapshot.prepare(self.source, self.source)
        with self.assertRaisesRegex(ValueError, 'repository root'):
            snapshot.prepare(self.source, self.destination / 'src')


if __name__ == '__main__':
    unittest.main()
