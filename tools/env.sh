# Source from bash or zsh. Keep the existing whitespace-free local SDK alias;
# contributors may point DIFFEQ_SDK_ROOT at another already installed SDK.
export DIFFEQ_SDK_ROOT="${DIFFEQ_SDK_ROOT:-$HOME/.local/diffeq-sdk}"
export PATH="$DIFFEQ_SDK_ROOT/prefix/bin:$DIFFEQ_SDK_ROOT/prefix/share/fxsdk/sysroot/bin:$DIFFEQ_SDK_ROOT/venv/bin:/opt/homebrew/opt/texinfo/bin:/opt/homebrew/opt/gnu-getopt/bin:/opt/homebrew/bin:$PATH"
export PKG_CONFIG_PATH="/opt/homebrew/opt/ncurses/lib/pkgconfig:/opt/homebrew/opt/libpng/lib/pkgconfig:/opt/homebrew/opt/libusb/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
