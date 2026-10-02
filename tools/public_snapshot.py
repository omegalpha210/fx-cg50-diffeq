#!/usr/bin/env python3
"""Prepare an audited source-only snapshot in the existing public checkout.

Dry-run by default. This never commits, pushes, rewrites history or moves tags.
Build/test/package the exact resulting commit before publishing a binary.
"""
import argparse
from pathlib import Path
import re
import subprocess

ROOT_FILES = {
    '.gitignore', '.gitattributes', 'CMakeLists.txt', 'README.md', 'README_KO.md',
    'LICENSE', 'THIRD_PARTY_NOTICES.md', 'CONTRIBUTING.md', 'CHANGELOG.md',
    'DEVELOPMENT.md', 'VERSION', 'PROJECT_STRUCTURE.md',
}
DIRECTORIES = ('src/', 'include/', 'tests/', 'tools/', 'assets/', '.github/',
               'docs/audits/', 'docs/captures/', 'docs/licenses/', 'docs/release/',
               'docs/archive/')
DEVELOPMENT_DOCS = {
    'ARCHITECTURE.md', 'FEATURE_PLAN.md', 'NAVIGATION_CALL_AUDIT.md',
    'WORKFLOW_SPEC.md', 'CODE_MAP.md',
}
PRIVATE_ARCHIVES = ('docs/archive/build-logs/',
                    'docs/archive/ENVIRONMENT_HISTORY.md',
                    'docs/archive/DEVELOPMENT_STATUS_BETA9.md')
PATTERNS = (
    rb'/(?:Users|home)/[A-Za-z0-9_.-]+/',
    rb'-----BEGIN (?:RSA |OPENSSH |EC |DSA )?PRIVATE KEY-----',
    rb'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})',
    rb'(?:AKIA[A-Z0-9]{16}|sk-[A-Za-z0-9]{32,}|xox[baprs]-[A-Za-z0-9-]{20,})',
)


def git(root, *args):
    return subprocess.check_output(['git', '-C', str(root), *args])


def public_path(path):
    if path.startswith(PRIVATE_ARCHIVES):
        return False
    if path.startswith('examples/hello/'):
        return (path == 'examples/hello/CMakeLists.txt' or
                path.startswith(('examples/hello/src/', 'examples/hello/assets-cg/')))
    if path.startswith('docs/development/'):
        return Path(path).name in DEVELOPMENT_DOCS
    return (path in ROOT_FILES or path.startswith(DIRECTORIES) or
            path in {'docs/README.md', 'docs/ACCEPTANCE.md', 'docs/USER_GUIDE.md',
                     'docs/HARDWARE_RETEST.md', 'docs/UI_CONVENTIONS.md',
                     'docs/EVENTS.md', 'docs/THIRD_PARTY.md', 'docs/USB_LIFECYCLE_AUDIT.md'})


def check_content(path, data):
    if Path(path).suffix.lower() in {'.pdf', '.g3a', '.elf', '.o', '.obj', '.log',
                                     '.map', '.su', '.ppm'}:
        raise ValueError(f'excluded artifact: {path}')
    if any(re.search(pattern, data) for pattern in PATTERNS):
        raise ValueError(f'private path or credential pattern: {path}')


def prepare(source, destination, revision='HEAD', apply=False):
    source = source.resolve(strict=True)
    destination = destination.resolve(strict=True)
    for root in (source, destination):
        if Path(git(root, 'rev-parse', '--show-toplevel').decode().strip()).resolve() != root:
            raise ValueError('source/destination must each be a Git repository root')
    if source == destination or not (destination / '.git').exists():
        raise ValueError('destination must be a separate existing checkout')
    if git(destination, 'branch', '--show-current').strip() != b'main':
        raise ValueError('public checkout must be on main')
    remote = git(destination, 'remote', 'get-url', 'origin').decode().strip()
    if remote.rstrip('/').removesuffix('.git') not in {
        'https://github.com/omegalpha210/fx-cg50-diffeq',
        'git@github.com:omegalpha210/fx-cg50-diffeq',
    }:
        raise ValueError('unexpected public origin')
    if git(destination, 'status', '--porcelain').strip():
        raise ValueError('public checkout has uncommitted changes')

    commit = git(source, 'rev-parse', revision + '^{commit}').decode().strip()
    selected = {}
    for record in git(source, 'ls-tree', '-rz', commit).split(b'\0'):
        if not record:
            continue
        metadata, name = record.split(b'\t', 1)
        path = name.decode()
        if not public_path(path):
            continue
        mode, kind, oid = metadata.decode().split()
        if kind != 'blob' or mode not in {'100644', '100755'}:
            raise ValueError(f'non-regular source entry: {path}')
        data = git(source, 'cat-file', 'blob', oid)
        check_content(path, data)
        selected[path] = (data, int(mode, 8) & 0o777)

    if not ROOT_FILES <= selected.keys():
        raise ValueError('required root project/license documents missing')
    for path in ('LICENSE', 'THIRD_PARTY_NOTICES.md'):
        if (destination / path).read_bytes() != selected[path][0]:
            raise ValueError(f'original license/notice changed: {path}')
    old = set(git(destination, 'ls-files', '-z').decode().rstrip('\0').split('\0'))
    removed = sorted(old - selected.keys())
    changed = [p for p, (data, _) in selected.items()
               if not (destination / p).is_file() or (destination / p).read_bytes() != data]
    # Reject symlinks/unsafe ancestors before performing any mutation.
    for path in [*removed, *selected]:
        target = destination / path
        for part in (target, *target.parents):
            if part == destination:
                break
            if part.is_symlink():
                raise ValueError(f'symlink in destination: {path}')
    print(f'Source {commit}: {len(selected)} public files; '
          f'{len(changed)} additions/updates, {len(removed)} removals')
    for path in removed:
        print('REMOVE' if apply else 'WOULD REMOVE', path)
    if apply:
        for path in removed:
            (destination / path).unlink()
        for path, (data, mode) in selected.items():
            target = destination / path
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
            target.chmod(mode)
    return selected


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--destination', required=True, type=Path)
    parser.add_argument('--revision', default='HEAD')
    parser.add_argument('--apply', action='store_true')
    args = parser.parse_args()
    try:
        prepare(Path(__file__).resolve().parents[1], args.destination,
                args.revision, args.apply)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(1, f'Snapshot refused: {error}\n')


if __name__ == '__main__':
    main()
