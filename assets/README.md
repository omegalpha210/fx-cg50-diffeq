# DIFF EQ menu icons

Original, code-generated solution family and slope field for y'=1-y². Both PNGs are 92×64 RGB, with an empty lower strip for the OS add-in name. Regenerate with `source tools/env.sh` then `python3 tools/make_icons.py`.

The installed fxSDK `GenerateG3A.cmake` consumes `icon-uns.png` and `icon-sel.png` through its `ICONS` option. It invokes upstream fxgxa and tracks PNG changes as link dependencies. No hand-edited G3A headers or alternate packager is used. Extracted final icons have been visually reviewed; the actual menu appearance is **HARDWARE TEST REQUIRED**.
