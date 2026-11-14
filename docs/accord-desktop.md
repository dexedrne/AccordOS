# Accord OS desktop

Run `./start.sh` in this checkout. It opens a 1280×800 desktop window by default;
an explicit `--window=1600x900` selects another size. The desktop opens directly
to the car. Configure API keys later under Settings → System → Guided setup.
No vehicle hardware or telemetry is connected. Speed is unavailable and the
door/hood/trunk controls animate the model only.

The first launch imports available models from `assets/garage/catalog.json`,
with the Accord selected first. Use the garage dropdown to switch cars and
**Add car** to import more. Imports preserve the existing models and each car's
paint, finish, setup and layout. Original files are never changed. The garage is
stored under `~/.config/VoltaOS/Accord OS/car/`, separate from upstream VoltaOS.
Missing catalog GLBs are skipped. The catalog only seeds an empty garage.

The catalog supports the Accord, four Civics, Toyota 4Runner, and Lexus
IS 300, LS 400, LS 430 and ES 300. Model files are not distributed in this fork.
Supply your own licensed GLBs and retain their source credits in `assets/garage/`.

## Builds

Use Qt **6.11.1** to match the included MapLibre plugin. The SDK is local to
`.qt/6.11.1/gcc_64` or `~/Qt/6.11.1/gcc_64`. Include `qtimageformats` for WebP
textures embedded in GLB models.

```sh
cmake -S . -B build/desktop -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$PWD/.qt/6.11.1/gcc_64"
cmake --build build/desktop -j3
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
./start.sh
```

The launcher exposes the virtual environment's packages to upstream's system
Python services. It selects Basic controls and XWayland to avoid incompatible
global desktop Qt themes. The MapLibre plugin also needs ICU 76; this installation
can use Debian's `libicu76` runtime extracted under `build/runtime/`, without
changing the system's ICU. Its upstream licence is included in that package.

On Arch, OBEX is supplied by `bluez-obex`, not Ubuntu's `bluez-obexd`.
Native Windows is not supported. The supplied Windows launcher assumes an Ubuntu
WSL distribution named `jobs` and a checkout at `~/projects/VoltaOS`; adjust those
paths for your installation. Its executable is `build/Release/VoltaOS`, with the
Python environment in `.venv/`.

## Verification

```sh
cmake -S tests/cars -B build/car-tests
cmake --build build/car-tests -j3
ctest --test-dir build/car-tests --output-on-failure
QT_QPA_PLATFORM=offscreen qmltestrunner -input tests/qml -o -,txt
```

Tests cover preserving cars and settings, persistent selection, one-time
seeding, Accord part names, rejected truncated GLBs, and appearance isolation.
The first review found appearance restoration and truncated-import defects;
regression tests reproduced them before fixes.

Maps and online music need their own API/service setup. Bluetooth needs an
available adapter and system services; WSL may not expose either. Native Android
Auto requires a certificate/key pair, absent from the upstream checkout. The
desktop builds without that optional key. No car-installation testing is included.
