# Unofficial EGG Mouse Config

This is an unofficial configuration tool for EGG XM2 and OP1 8k gaming mice.

## Features

- Configure CPI (50-26000 in 50 CPI steps)
- Toggle angle snapping and ripple control
- Adjust lift-off distance (1-2mm)
- Set polling rate (1000Hz, 2000Hz, 4000Hz, 8000Hz or custom)
- Enable/disable motion sync (when available)
- Configure button debounce settings
- Set SPDT modes (for left/right buttons)
- Enable/disable slamclick and motion jitter filters
- Backup/restore mouse configuration
- Factory reset capability

## Supported Mice

Currently supported models:
- EGG XM2 8k (PID: `0x1966`)
- EGG OP1 8k Purple Frost (PID: `0x1976`)
- EGG OP1 8k v2 (PID: `0x1978`)

## Compilation

### Prerequisites

- CMake (≥3.16)
- C++20 compatible compiler
- HIDAPI library (libhidapi-hidraw on Linux)
- OpenGL
- Git (for version detection)

For Linux desktop builds:
- GLFW
- Freetype (for font rendering)

For Emscripten builds:
- Emscripten SDK
- SDL2

### Build Instructions

#### Preparation

```bash
git clone https://github.com/niansa/UnofficialEGGMouseConfig.git
cd UnofficialEGGMouseConfig
git submodule update --init --depth 1 --recursive
mkdir build
cd build
```

#### Linux Build

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo cp EGGMouseConfig /usr/local/bin/
```

#### Linux Static Build (Portable but bigger)

```bash
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -DBUILD_STATIC=ON ..
make StripAndCompress
```

#### Web Build (Emscripten)

```bash
emsdk activate latest
emcmake cmake -DCMAKE_BUILD_TYPE=Release ..
emmake make -j$(nproc)
python3 -m http.server 8000
```
Access via `http://localhost:8000` in Chrome/Edge

## Usage

### Linux Desktop Version

1. Run the application:
```bash
EGGMouseConfig
```

2. If you encounter permission issues, run as root (avoid):
```bash
sudo EGGMouseConfig
```

3. The application will automatically detect connected compatible mice.

### Web/EMSCRIPTEN Version

1. Build the application as described above
2. Serve the generated HTML/JS files through a web server
3. Open in a modern browser

Note: The web version requires browser support for WebHID.

## Udev rule

You can add the following udev rules to grant members of `plugdev` group and users physically present on the PC full access to the mouse configuration:

/etc/udev/rules.d/30-egg.rules
```udev
SUBSYSTEM=="usb", ATTRS{idVendor}=="3367", GROUP="plugdev", MODE="0660", TAG+="uaccess"
SUBSYSTEM=="hidraw", ATTRS{idVendor}=="3367", GROUP="plugdev", MODE="0660", TAG+="uaccess"
```

Then apply new rules:

```
sudo udevadm control --reload-rules && sudo udevadm trigger
```

## Troubleshooting

- If the mouse isn't detected:
  - Ensure you have proper permissions (add udev rule or try running as root or add user to `input` group)
  - Check that the mouse is connected via USB
  - Verify your mouse model is supported

- For build issues:
  - Ensure all dependencies are installed
  - Check your compiler supports C++20
  - Verify CMake version is ≥3.16

## Safety Notice

Experimental features can potentially brick your mouse! In this case, factory reset using the official application might be required.

## Credits

- **Main developer:** [niansa (@tuxifan)](https://github.com/niansa)
- **Contributor:** [elin_lyze](https://discord.com/users/324575119571681281)
- **[HIDAPI library for cross-platform HID access](https://github.com/libusb/hidapi):** [libusb team](https://github.com/libusb)
- **[Dear ImGui for the UI framework](https://github.com/ocornut/imgui):** [ocornut](https://github.com/ocornut)
