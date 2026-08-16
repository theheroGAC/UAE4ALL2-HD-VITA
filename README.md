# UAE4LL2 HD — PlayStation Vita

A cleaned PlayStation Vita build of UAE4ALL2 HD, an Amiga emulator based on the UAE4ALL2 project.

This package contains the Vita source tree and the final release package:

```text
uae4ll2hd.vpk
```

## Features in this Vita build

- Amiga OCS, ECS and AGA emulation
- 68000 and 68020 CPU modes
- ADF, ADZ and ZIP floppy images
- IPF floppy images through the bundled CAPS decoder
- LHA/LZH archives containing Amiga disk images
- HDF hard-disk support
- Savestates and thumbnails
- Virtual keyboard, touch controls and analog mouse
- Vita shaders and aspect-ratio scaling
- Vita menu with floppy, hardware, display, controls and system settings

## Installation

1. Install `uae4ll2hd.vpk` on a PlayStation Vita.
2. Copy legally obtained Kickstart ROMs to:

```text
ux0:/data/uae4all/kickstarts/
```

At minimum, use:

```text
kick13.rom
kick31.rom
```

Kickstart ROMs are copyrighted. Do not distribute them with this project.

3. Copy Amiga disk images to a folder on `ux0:` or `uma0:`.
4. Start the emulator and select the image from the Floppy tab.

## Building for Vita

Install VitaSDK and its SDL, SDL_image, zlib, libarchive, PNG and JPEG development packages. The included Vita2D and shader libraries are linked by CMake.

```bash
export VITASDK=/usr/local/vitasdk
mkdir build
cd build
cmake .. -DBUILD_PSP2=ON -DCMAKE_BUILD_TYPE=Release
ninja uae4all2.vpk
```

The build output is a VPK named `uae4all2.vpk`. Rename it to `uae4ll2hd.vpk` for the release package if required.

## Supported disk images

The browser recognizes common Amiga disk formats including ADF, ADZ, DMS, IPF, ZIP, LHA and LZH. ZIP and LHA/LZH archives must contain a supported disk image. IPF images are decoded through the included CAPS image library and are read-only.

## Credits and acknowledgements

This project is a derivative work and would not exist without the original UAE4ALL and Vita ports. Full credit and thanks go to the original authors and contributors:

- Chui
- john4p
- TomB
- notaz
- Bernd Schneider
- Toni Wilen
- Pickle
- smoku
- AnotherGuest
- Anonymous engineer
- finkel
- Lubomyr
- pelya
- Cpasjuste for the original Vita port, SDL-Vita work, shader support and performance improvements
- rsn8887 for the Vita/Switch work and the UAE4ALL2 improvements
- ScHlAuChi for testing, ideas and virtual-keyboard contributions
- wronghands for the menu font, keyboard styles and design ideas
- CrashMidnick for the French virtual keyboard
- Xerpi and frangarCJ for Vita2D and shader-library work
- The VitaSDK Team

Please preserve the original project credits and license notices when redistributing or modifying this project.

## Legal notice

UAE4ALL2 HD is intended for use with legally acquired Amiga software and Kickstart ROMs. Amiga, UAE4ALL and related trademarks belong to their respective owners. This repository does not include copyrighted Kickstart ROMs, commercial games or commercial disk images.

## License

See `copying` and the original source files for the applicable license notices.
