# Local tools

- `bootstrap.sh`: reconstruct the pinned SDK on macOS with Homebrew. Installs host dependencies, creates one whitespace-free symlink at `~/.local/diffeq-sdk`, and keeps SDK sources/builds/install inside workspace `.local/`. No sudo and no shell startup edits.
- `env.sh`: source into bash/zsh to activate tools for this shell only.
- `build-toolchain.sh`: manual upstream SH binutils/GCC/OpenLibm/fxlibc/gint build, after host fxSDK installation. Logs under `docs/build-logs/`; detailed binutils logs in `.local/src/sh-elf-binutils/build/`.
- `build.sh --hello`: build the retained minimal gint add-in.
- `build.sh`: build the application (or minimal add-in before the root project exists).
- `verify_g3a.py FILE`: independently validate container fields and both checksums; based on fxSDK `fxgxa/{g3a.h,edit.c,util.c}`. This does not emulate code or certify hardware operation.
- `toolchain-lock.json`: exact upstream repository revisions. Downloaded GNU releases are binutils 2.42 and GCC 14.1.0, as selected by those upstream scripts.
- `patches/`: minimal local macOS fixes. GCC additionally receives the upstream soft-float patch already shipped in its installer repository.

The GCC build includes C and C++ compilers, but only libgcc is required here. This C application does not need a libstdc++ build. fxlink's Linux-only UDisks2 integration and optional SDL2 viewer are disabled. USB driver support remains compiled in.
