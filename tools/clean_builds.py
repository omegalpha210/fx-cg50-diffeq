#!/usr/bin/env python3
"""Inspect or remove exact, known CMake build caches after a cleanup audit.

Dry-run is the default. This tool does not establish whether an untracked file
is user-owned: inventory, uniqueness review and fresh rebuild must precede apply.
"""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys


# Each entry is an exact relative cache path and its expected source directory.
# SDK builds, dist, reference files, release evidence and generic tmp directories
# are deliberately absent. Current caches are available only when named.
LEGACY_BUILDS = {
    'build-host': 'tests',
    'build-cg': '.',
    'build-asan': 'tests',
    '.local/build-host-audit': 'tests',
    '.local/build-host-busy': 'tests',
    '.local/build-host-ic': 'tests',
    '.local/build-host-math': 'tests',
    '.local/public/fx-cg50-diffeq/build-host': '.local/public/fx-cg50-diffeq/tests',
    '.local/public/fx-cg50-diffeq/build-cg': '.local/public/fx-cg50-diffeq',
    'examples/hello/build-cg': 'examples/hello',
}
KNOWN_BUILDS = {
    **LEGACY_BUILDS,
    'build/host': 'tests',
    'build/target': '.',
    'build/hello-target': 'examples/hello',
}


def git_output(directory, *arguments):
    return subprocess.check_output(
        ['git', '-C', str(directory), *arguments], stderr=subprocess.PIPE)


def inspect_build(root, relative):
    """Return (path, file count, bytes), rejecting source and ambiguous caches."""
    if relative not in KNOWN_BUILDS:
        raise ValueError('path is not an exact known build cache')
    root = root.resolve(strict=True)
    repository = Path(git_output(root, 'rev-parse', '--show-toplevel').decode().strip())
    if repository.resolve() != root:
        raise ValueError('project root must be the Git repository root')

    path = root / relative
    for component in (path, *path.parents):
        if component == root:
            break
        if component.is_symlink():
            raise ValueError('build path or ancestor is a symlink')
    if not path.exists():
        return None
    resolved = path.resolve(strict=True)
    if resolved == root or root not in resolved.parents or not resolved.is_dir():
        raise ValueError('build path must be a directory strictly inside the project')

    cache = resolved / 'CMakeCache.txt'
    if not cache.is_file() or not (resolved / 'CMakeFiles').is_dir():
        raise ValueError('expected CMake cache and CMakeFiles are missing')
    entries = cache.read_text().splitlines()
    homes = [line.split('=', 1)[1] for line in entries
             if line.startswith('CMAKE_HOME_DIRECTORY:INTERNAL=')]
    expected = (root / KNOWN_BUILDS[relative]).resolve(strict=True)
    if len(homes) != 1 or Path(homes[0]).resolve() != expected:
        raise ValueError('CMake source directory does not match the known cache')

    owner = Path(git_output(resolved, 'rev-parse', '--show-toplevel').decode().strip()).resolve()
    tracked = git_output(owner, 'ls-files', '-z').split(b'\0')
    for name in tracked:
        if name:
            tracked_path = owner / name.decode('utf-8', errors='surrogateescape')
            if tracked_path == resolved or resolved in tracked_path.parents:
                raise ValueError('build directory contains a Git tracked path')

    files = []
    for entry in resolved.rglob('*'):
        if entry.is_symlink():
            raise ValueError('build directory contains a symlink')
        if entry.name == '.git':
            raise ValueError('build directory contains a Git repository')
        if entry.is_file():
            files.append(entry)
    return resolved, len(files), sum(entry.stat().st_size for entry in files)


def clean_builds(root, selected, apply=False):
    # Validate every selected path before performing the first deletion.
    inspected = [(relative, inspect_build(root, relative)) for relative in selected]
    for relative, details in inspected:
        if details is None:
            print(f'ABSENT {relative}')
        else:
            path, count, size = details
            action = 'REMOVE' if apply else 'WOULD REMOVE'
            print(f'{action} {relative}: {count} files, {size} bytes')
            if apply:
                # Repeat guards immediately before this exact path mutation.
                inspect_build(root, relative)
                shutil.rmtree(path)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument('--dry-run', action='store_true', help='inspect only (default)')
    mode.add_argument('--apply', action='store_true', help='remove audited caches')
    parser.add_argument('paths', nargs='*', metavar='PATH',
                        help='exact known paths; defaults to obsolete caches only')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    selected = list(dict.fromkeys(args.paths or LEGACY_BUILDS))
    try:
        clean_builds(root, selected, args.apply)
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f'Cleanup refused: {error}', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
