# Source from bash or zsh. All actual SDK files live in this workspace.
export DIFFEQ_SDK_ROOT="$HOME/.local/diffeq-sdk"
export PATH="$DIFFEQ_SDK_ROOT/prefix/bin:$DIFFEQ_SDK_ROOT/prefix/share/fxsdk/sysroot/bin:$DIFFEQ_SDK_ROOT/venv/bin:/opt/homebrew/opt/texinfo/bin:/opt/homebrew/opt/gnu-getopt/bin:/opt/homebrew/bin:$PATH"
export PKG_CONFIG_PATH="/opt/homebrew/opt/ncurses/lib/pkgconfig:/opt/homebrew/opt/libpng/lib/pkgconfig:/opt/homebrew/opt/libusb/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
