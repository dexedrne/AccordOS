#!/bin/bash
# Starts Volta OS using the most recently built executable in build/.
DIR="$(cd "$(dirname "$(readlink -f "$0")")" && pwd)"
BIN=$(ls -t "$DIR"/build/VoltaOS "$DIR"/build/*/VoltaOS 2>/dev/null | head -1)
if [ -z "$BIN" ]; then
    echo "VoltaOS not found: build it first (see README.md)" >&2
    exit 1
fi
export VOLTA_DIR="$DIR"
export LD_LIBRARY_PATH="$DIR/build/runtime/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
for VOLTA_QT_ROOT in "$DIR/.qt/6.11.1/gcc_64" "$HOME/Qt/6.11.1/gcc_64"; do
    if [ -d "$VOLTA_QT_ROOT/lib" ]; then
        export LD_LIBRARY_PATH="$VOLTA_QT_ROOT/lib:$LD_LIBRARY_PATH"
        break
    fi
done
export QT_QUICK_CONTROLS_STYLE=Basic
export QT_QPA_PLATFORM=xcb
if [ -x "$DIR/.venv/bin/python" ]; then
    # Upstream services pin /usr/bin/python3; expose this checkout's installed packages.
    VOLTA_SITE_PACKAGES=$("$DIR/.venv/bin/python" -c 'import sysconfig; print(sysconfig.get_path("purelib"))')
    export PYTHONPATH="$VOLTA_SITE_PACKAGES${PYTHONPATH:+:$PYTHONPATH}"
fi
if [ "$#" -eq 0 ]; then set -- --window=1280x800; fi
exec "$BIN" "$@"
