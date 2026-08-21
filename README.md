# UAE4LL2 HD — PlayStation Vita 1.03

A cleaned PlayStation Vita build of UAE4ALL2 HD, an Amiga emulator based on the UAE4ALL2 project.

This package contains the Vita source tree and the final release package:

```text
uae4all2hd.vpk
```

## Features in this Vita build

- Amiga OCS, ECS and AGA emulation
- 68000 and 68020 CPU modes
- ADF, ADZ, DMS, IPF and ZIP floppy images
- IPF floppy images through the bundled CAPS decoder
- ZIP, LHA and LZH archives containing Amiga disk images
- HDF hard-disk images and HD directories (4 HDF slots, boot order selection)
- WHDLoad tab with safe LHA extraction, game list and startup-script generation
- Savestates and thumbnails, savestate slots 1-4 with per-game naming
- Virtual keyboard, touch controls and analog mouse
- Vita shaders and aspect-ratio scaling
- Vita menu with floppy, hard disk, WHDLoad, presets, hardware, display, controls, savestates and system tabs
- About screen with version 1.03 and automatic scrolling credits
- CD32 Akiko CD controller with ISO, raw BIN and multi-track CUE images
- CD32 data tracks, CD audio playback, subcode data, DMA and controller state
- CD32-aware savestates including the mounted image and playback position
- Automatic per-game configuration files with CD32 profile application when a CD image is mounted
- PNG screenshots saved to `ux0:/data/uae4all/screenshots/` at 960x544 with preserved aspect ratio and black bars when required
- Automatic CUE handling for multiple FILE entries, INDEX 00/01, PREGAP and POSTGAP
- Live DF0-DF3 and HDF activity lights with synthesized disk-access sounds
- Quick Menu (L trigger in-game) with resume, save/load state, eject DF0, eject CD32 and screenshot
- Presets saved to memory only: configuration is written only from `System -> Save Game Configuration`

## Installation

1. Install `uae4all2hd.vpk` on a PlayStation Vita.
2. Copy legally obtained Kickstart ROMs to:

```text
ux0:/data/uae4all/kickstarts/
```

3. Copy Amiga disk images to a folder on `ux0:` or `uma0:`.
4. Start the emulator and select the image from the Floppy tab, or the HDF/WHDLoad tab.
5. Press **START** to boot.

## Kickstart ROMs

The Hardware tab accepts both the classic UAE4ALL names and the Amiga Forever / TOSEC names. The emulator probes the aliases in order and uses the first file found in `ux0:/data/uae4all/kickstarts/`.

Kickstart ROMs are copyrighted. Do not distribute them with this project. Verify your files with the official MD5 checksums below.

| System | Version | Filename (UAE4ALL) | Amiga Forever / TOSEC alias | Size | MD5 |
|---|---|---|---|---|---|
| A1000 | KS v1.1 rev 31.034 NTSC | `kick31034.A1000` | `amiga-os-110-ntsc.rom` | 262144 | `0b8442c311caa54fb12ec88eaaa9facf` |
| A1000 | KS v1.1 rev 32.034 PAL | `kick32034.A1000` | `amiga-os-110-pal.rom` | 262144 | `1fa1f93d3d7b51271dd1356b8b2b45a9` |
| A500-A2000 | KS v1.2 rev 33.180 | `kick12.rom`, `kick33180.A500` | `amiga-os-120.rom` | 262144 | `85ad74194e87c08904327de1a9443b7a` |
| A500-A2000 | KS v1.3 rev 34.005 | `kick13.rom`, `kick34005.A500` | `amiga-os-130.rom` | 262144 | `82a21c1890cae844b3df741f2762d48d` |
| A500+ | KS v2.04 rev 37.175 | `kick20.rom`, `kick37175.A500` | `amiga-os-204.rom` | 524288 | `dc10d7bdd1b6f450773dfb558477c230` |
| A600 | KS v2.05 rev 37.350 | `kick37350.A600` | `amiga-os-205-a600.rom` | 524288 | `465646c9b6729f77eea5314d1f057951` |
| A600-A2000 | KS v3.1 rev 40.063 | `kick40063.A600` | `amiga-os-310-a600.rom` | 524288 | `e40a5dfb3d017ba8779faba30cbd1c8e` |
| A1200 | KS v3.0 rev 39.106 | `kick39106.A1200` | `amiga-os-300-a1200.rom` | 524288 | `b7cc148386aa631136f510cd29e42fc3` |
| A1200 | KS v3.1 rev 40.068 | `kick31.rom`, `kick40068.A1200` | `amiga-os-310-a1200.rom` | 524288 | `646773759326fbac3b2311fd8c8793ee` |
| A4000 | KS v3.0 rev 39.106 | `kick39106.A4000` | `amiga-os-300-a4000.rom` | 524288 | `9b8bdd5a3fd32c2a5a6f5b1aefc799a5` |
| A4000 | KS v3.1 rev 40.068 | `kick40068.A4000` | `amiga-os-310-a4000.rom` | 524288 | `9bdedde6a4f33555b4a270c8ca53297d` |
| CD32 | KS + Extended v3.1 rev 40.060 | `kick40060.CD32` | combined 1 MiB image | 1048576 | `f2f241bf094168cfb9e7805dc2856433` |
| CD32 | KS v3.1 rev 40.060 | `kick40060.CD32` | `amiga-os-310-cd32.rom` | 524288 | `5f8924d013dd57a89cf349f4cdedc6b1` |
| CD32 | Extended ROM rev 40.060 | `kick40060.CD32.ext` | `amiga-os-310-cd32-ext.rom` | 524288 | `bb72565701b1b6faece07d68ea5da639` |
| CDTV | CDTV Extended ROM v1.0 | `kick34005.CDTV` | `amiga-os-130-cdtv-ext.rom` | 262144 | `89da1838a24460e4b93f4f0c5d92d48d` |

For the CD32 preset, both ROMs are required:

```text
kick40060.CD32
kick40060.CD32.ext
```

`kick40060.CD32` is the 512 KiB main CD32 Kickstart ROM and `kick40060.CD32.ext` is the 512 KiB CD32 Extended ROM. The extended ROM is what selects the CD32 hardware path and maps Akiko at `$B80000`; an A1200 `kick31.rom` cannot replace either CD32 file. A single combined 1 MiB `kick40060.CD32` image is also recognized. The filenames are case-sensitive on some Vita storage setups.

The emulator also accepts `kickcustom.rom` / `custom.rom` for a user ROM, and `aros-amiga-m68k-rom.bin` / `aros.rom` for the AROS fallback ROM (`aros-amiga-m68k-ext.bin` is the matching extended ROM).

## Presets

The Presets tab applies a complete machine profile in memory:

| Preset | CPU | Chipset | Kickstart | RAM |
|---|---|---|---|---|
| Amiga 500 | 68000 | OCS | 1.3 | 512 KiB Chip + 512 KiB Slow |
| Amiga 500+ | 68000 | ECS | 2.04 | 1 MiB Chip + 1 MiB Fast |
| Amiga 600 | 68000 | ECS | 2.05 | 2 MiB Chip + 8 MiB Fast |
| Amiga 1200 | 68020 | AGA | 3.1 | 2 MiB Chip + 4 MiB Fast |
| Amiga CD32 | 68020 | AGA | 3.1 CD32 + Extended | 2 MiB Chip + Akiko |

Presets only change the in-memory configuration. To keep them after restarting the app, use `System -> Save Game Configuration`.

## Hard disks (HDF)

The Hard Disk tab provides four HDF slots and a Boot HD selector with three modes:

- **Off** - no hard disk
- **HD Directory** - mounts a directory (also used by the WHDLoad library)
- **HDF Files** - mounts the four configured `.hdf` images

To use an HDF:

1. Create and prepare the HDF with WinUAE (Amiga 1200, Kickstart 3.1, partition `DH0:`, install Workbench with HDSetup).
2. Copy the finished `.hdf` to the Vita, for example:

```text
ux0:/data/uae4all/roms/workbench.hdf
```

3. In the Hard Disk tab insert it in **HDF1** and set **Boot HD -> HDF Files**.
4. Press **START**. The emulator boots `DH0:` from the HDF.

The classic installation path is also supported on the Vita itself: mount an empty HDF, boot the Workbench Install ADF in DF0, use HDToolBox with `SCSI_DEVICE_NAME=uaehf.device` (set via the Workbench Icon Information window), create `DH0:`, format it, and run HDSetup. The emulated `uaehf.device` does not answer the SCSI INQUIRY command, so define the drive geometry manually (32 blocks per track, 1 surface, 2 reserved blocks, 512-byte blocks).

HDF activity is shown by the hard-disk LED and mixed with the emulator audio as a synthesized drive sound.

## WHDLoad and LHA

The WHDLoad tab automates game installation and launching:

1. Copy a WHDLoad game archive (`.lha` / `.lzh`) to the Vita.
2. Open the **WHDLoad** tab and choose **Install Game from LHA**.
3. Select the archive. It is extracted safely (path-traversal guarded) into:

```text
ux0:/data/uae4all/whdload/<GameName>/
```

4. The installed folders appear in the game list. Selecting a game:

   - finds the `.slave` file recursively;
   - backs up the existing `S:Startup-Sequence` to `S:Startup-Sequence.uae4all` (once);
   - writes `S:UAE4ALL-WHDLoad` containing `C:WHDLoad "DH0:<GameName>/<slave>"`;
   - injects `Execute S:UAE4ALL-WHDLoad` into `S:Startup-Sequence`;
   - mounts the library as the HD directory and reboots the emulation.

5. **Use WHDLoad Directory** selects the library as the HD directory without launching a game.

A Workbench environment, the user-supplied `WHDLoad` executable and legally obtained game files are still required. LHA extraction requires `ux0:/data/uae4all/whdload/` to be writable and uses libarchive.

## Supported disk images

The browser recognizes common Amiga disk formats including ADF, ADZ, DMS, IPF, ZIP, LHA and LZH. ZIP and LHA/LZH archives selected from the Floppy tab must contain a supported disk image. LHA archives selected from the WHDLoad tab are extracted as game files. IPF images are decoded through the included CAPS image library as read-only media, with protected and variable-density data handled where compatible with the Vita floppy timing path. Compatibility still depends on the quality and variant of the IPF dump.

## CD32 support

- Open `Hardware -> CD32 CD Image` and select an `.iso`, raw `.bin`, or `.cue`.
- Mounting a CD image automatically selects the CD32 Kickstart pair, AGA chipset, 2 MiB chip RAM, 68020 CPU mode and one floppy drive; the settings can still be changed before rebooting.
- CUE files can reference separate data and audio BIN files and can contain multiple tracks with INDEX 00/01, PREGAP and POSTGAP.
- CD audio is mixed through the Vita SDL audio output.
- The Hardware tab can eject or replace the mounted image while the emulator is paused in the menu.
- Savestates include the mounted CD image and the playback position.

## Controls

- D-Pad / left analog: joystick
- Cross: fire 1 (and UI confirm)
- Circle: fire 2 (and UI back)
- Square / Triangle: configurable extra buttons
- Left trigger: Quick Menu (in-game)
- Touch screen: mouse emulation (one finger = left button, two fingers = right button)
- START: boot / resume

The Controls tab covers joystick port, autofire, mouse multiplier, mouse emulation, stylus offset, tap delay, custom control sets (including the Pinball Dreams / Slam Tilt presets) and the Floppy/HDF sound volume.

## Display

The Display tab provides shaders, video preset modes (200 to 270 lines, NTSC/PAL, 5:4 and fullscreen variants), footer size, screen offset, cut left/right, frameskip, background, font, and the virtual keyboard language and style.

## Disk activity lights and sounds

The in-game overlay shows one activity light for each floppy drive and one for hard-disk access. Floppy lights follow motor and DMA activity. The HDF light is blue during reads and red during writes. The audio backend loads short segments from the bundled Ogg samples for floppy and hard-disk activity, then mixes them with the normal emulator audio. The Controls tab includes a separate 0-100% volume setting for these disk sounds. If a sample cannot be opened, the mixer falls back to the built-in synthesized effect. The floppy sample is `Floppy drive sounds.ogg` by AlepouTheFox, released under CC0 1.0. The hard-disk sample is `WD bad heads click of death.ogg` by Zzptichka, released into the public domain.

The original sample sources are:

- https://commons.wikimedia.org/wiki/File:Floppy_drive_sounds.ogg
- https://commons.wikimedia.org/wiki/File:WD_bad_heads_click_of_death.ogg

The packaged copies are `psp2data/data/sounds/floppy_drive.ogg` and `psp2data/data/sounds/hard_drive.ogg`. The Vita build links Vorbis decoding support so the samples can be loaded from `app0:/data/sounds/`.

## System tab

- **Save Game Configuration**: writes `ux0:/data/uae4all/conf/uaeconfig.conf` (or a per-game config derived from the mounted disk name). This is the only automatic write point in the Vita menu.
- **Restore Default Settings**: resets CPU, chipset, memory, Kickstart, floppies, HDFs, CD, display, audio and controls to factory defaults in memory (does not write any file).
- **Reboot Amiga Emulation**: hard resets the Amiga with the current settings.
- **Take Screenshot**: captures the next emulated frame as a PNG.
- **About**: version 1.03 with scrolling credits.

## Building for Vita

Install VitaSDK and its SDL, SDL_image, zlib, libarchive, PNG and JPEG development packages. The included Vita2D and shader libraries are linked by CMake.

```bash
export VITASDK=/usr/local/vitasdk
mkdir build
cd build
cmake .. -DBUILD_PSP2=ON -DCMAKE_BUILD_TYPE=Release
ninja uae4all2.vpk
```

The build output is `uae4all2hd.vpk`.

The optional CD-ROM backend test target is enabled in a native build with:

```bash
cmake .. -DBUILD_CDROM_TESTS=ON
cmake --build . --target cdrom_tests
ctest --output-on-failure
```

The Vita build uses the cross-compiled emulator and VPK target; native CD-ROM tests require a host C and C++ compiler.

## UAEGFX and RTG status

UAEGFX/Picasso96 is not enabled in this build. The source contains partial Zorro graphics-memory scaffolding, but a usable UAEGFX implementation also needs the Zorro III board, framebuffer registers, Picasso96-compatible Amiga-side driver or ROM, RTG mode switching, blitting, palette handling, and Vita display integration. It is possible to add, but it should be implemented as a separate phase after CD32 validation rather than enabled by only defining `PICASSO96`.

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
