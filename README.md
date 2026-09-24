# UAE4ALL2 HD — PlayStation Vita 1.11

A cleaned PlayStation Vita build of UAE4ALL2 HD, an Amiga emulator based on the UAE4ALL2 project.

Release documentation: [`CHANGELOG.md`](CHANGELOG.md). The repository license is available in [`LICENSE`](LICENSE); original license notices remain in the source tree.

This package contains the Vita source tree and the final release package:

```text
uae4all2hd.vpk
```

## Features in this Vita build

- Amiga OCS, ECS and AGA emulation
- 68000 and 68020 CPU modes
- ADF, ADZ, DMS, IPF and ZIP floppy images
- IPF floppy images through the latest upstream CAPS decoder
- ZIP, LHA and LZH archives containing Amiga disk images
- HDF hard-disk images and HD directories (4 HDF slots, boot order selection)
- Integrated HDF Manager (create, format, and prepare FFS hard-disk images from 50 MB to 8192 MB)
- Dedicated WHDLoad tab with one-click game launch and automatic A1200 AGA preset configuration
- Automatic alphabetical (A-Z) library sorting for installed WHDLoad games with support for up to 256 games
- Custom WHDLoad Arguments editor with native PS Vita OSK keyboard support
- High-performance, bit-perfect native LHA decompressor (-lh5-, -lh4-, -lh0-) with real-time UI progress bar
- Pre-bundled official WHDLoad binaries (C:WHDLoad, CD32, DIC, Patcher, RawDIC, WArc, VFS, S:WHDLoad.prefs)
- Automatic Amiga directory navigation (CD) into game folders for full resource locking
- Hidden system directories (S, C, Libs, Devs, L, etc.) from the WHDLoad game browser
- Savestates and thumbnails, savestate slots 1-4 with per-game naming
- Virtual keyboard, touch controls and analog mouse
- Advanced Autofire with configurable speeds (Slow, Medium, Turbo) and Trigger modes (Hold Fire Button / Continuous)
- Vita shaders and aspect-ratio scaling (4:3, 5:4, 16:9)
- **Auto Display & Auto Center**: Dynamic hardware DIW and copper tracking enabled by default, eliminating manual offset adjustments and black borders with perfect on-screen centering
- Centered 320x286 Full Frame PAL display mode with complete overscan coverage (Project-X HUD, score and power bars visible simultaneously)
- Unified high-contrast UI highlighting across Floppy (DF0-DF3) and Hard Disk (HDF1-HDF4) slots
- In-game hotkeys: `R + START + D-Pad Up/Down` for fine scanline panning (with real-time OSD toast overlay) and `R + START + D-Pad Left/Right` for live resolution switching (safely protected when Auto Display is active)
- Seamless game launch: clean black transition screen displaying "Caricamento in corso..." and game title, eliminating menu ghosting and 1-second freezes during WHDLoad loading
- Fixed START button handling: toggles the virtual keyboard (VKBD) in non-CD32 games without triggering CD32 Play/Pause
- Streamlined on-screen indicators: removed redundant top-right activity boxes so only the native UAE status bar renders when enabled
- Vita menu with game library, floppy, hard disk, WHDLoad, presets, hardware, display, controls, savestates and system tabs
- Game Library tab with recursive scan of the configured folders, cached index, automatic stale check and per-type filtering (All, Favourites, Floppy, WHDLoad, Hard Disk, CD32)
- One-press game launch from the Library for ADF/IPF, HDF, WHDLoad, LHA and CD32 images, plus TRIANGLE favourites shared with the WHDLoad tab
- Custom library folders through `ux0:/data/uae4all/library_roots.txt` (up to 8 roots, one per line)
- Animated Boing Ball startup splash with real initialization stages and progress bar
- About screen with version 1.11 and automatic scrolling credits
- CD32 Akiko CD controller with native CHD (Compressed Hunks of Data), ISO, raw BIN, multi-track CUE images and M3U playlists for multi-disc games
- CD32 in-game status bar with active blue "CD" read/write activity indicator and timeout
- CD32 internal 1KB NVRAM (24C08 I2C EEPROM) emulation for native in-game saves (`ux0:/data/uae4all/saves/cd32.nvram`)
- CD32 Quick Menu and 10-slot Savestates with preview thumbnails
- Custom ROM preset in Presets tab with 68020 AGA and 8MB Fast RAM
- Quick Menu (L trigger in-game) with resume, save/load state, eject DF0, eject CD32 and screenshot
- Presets saved to memory only: configuration is written only from `System -> Save Game Configuration`
- FTP file transfer using the VitaSDK `ftpvita` service with 512 KB buffer size and full filesystem access
- Dedicated FTP screen showing the Vita IP address and port `1337`
- FTP starts when entering the FTP screen and stops automatically with Circle or Cross when leaving it
- Full FTP access to all PS Vita partitions (`ux0:`, `ur0:`, `uma0:`, `app0:`, `gro0:`, etc.)
- This HD release uses the separate Title ID `UAE4ALLHD` and does not overwrite legacy UAE4ALL2 installations

## Installation

1. Install `uae4all2hd.vpk` on a PlayStation Vita.
2. Copy legally obtained Kickstart ROMs to:

```text
ux0:/data/uae4all/kickstarts/
```

3. Copy Amiga disk images to a folder on `ux0:` or `uma0:`.
4. Start the emulator and select the image from the Floppy tab, or the HDF/WHDLoad tab.
5. Press **START** to boot.

### FTP file transfer

Open the **System** tab and select **FTP File Transfer**. The dedicated FTP screen starts the VitaSDK `ftpvita` service and displays the connection address. Connect from a PC with FileZilla or another FTP client using the displayed IP address and port `1337`.

The service exposes all partitions (`ux0:`, `ur0:`, `uma0:`, `app0:`, etc.). It remains active while the FTP screen is open; press **Circle** or **Cross** to stop FTP and return to the System tab. Use standard FTP only on a trusted local network because it does not encrypt traffic.

### WHDLoad relocation files

Some WHDLoad slaves require a relocation file such as `kick34005.A500.RTB`. These files are not included in the repository and are never downloaded by the application. Obtain them legally and copy them manually to:

```text
ux0:/data/uae4all/kickstarts/
```

The application copies a user-provided RTB file into the WHDLoad `Devs/Kickstarts` directory when a game is launched.

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

The classic installation path is also supported on the Vita itself: mount an empty HDF, boot the Workbench Install ADF in DF0, use HDToolBox with `SCSI_DEVICE_NAME=uaehf.device` (set via the Workbench Icon Information window), create `DH0:`, format it, and run HDSetup. The emulated `uaehf.device` does not answer the SCSI INQUIRY command, so define the drive geometry manually; the HDF Manager screen shows the exact values to enter (blocks per track, surfaces, reserved blocks, block size and cylinders) for the currently mounted image. Blocks are always 512 bytes, there are 32 sectors per track and 2 reserved blocks; the number of surfaces scales with capacity (1 below 1 GB, 2 up to 2 GB, 4 up to 4 GB, 8 up to 8 GB, 16 from 8 GB).

An empty or unformatted HDF is not bootable, so a floppy image can stay in DF0 while the HDF is mounted as a secondary disk: press **START** with the Workbench/installer ADF in DF0 and the blank HDF mounted to boot from the floppy, partition and format the HDF from HDToolBox, and install onto it. The eject prompt only appears when hard-disk boot is enabled (**Boot HD** set to *HD Directory* or *HDF Files*), a mounted HDF already contains a valid bootblock (`DOS\x`) or partition table (`RDSK`), and a floppy is inserted, because only then does the emulator need the floppy drives free.

The HDF container can be up to 8192 MB (8 GB). Note that the emulated Amiga hard-disk device interface addresses the device with a 32-bit byte offset, so a single Amiga partition remains limited to 4 GB, as on classic UAE/WinUAE and real Amiga hardware. An 8 GB image is valid as a container and can hold multiple partitions created and managed from HDToolBox.

HDF activity is shown by the hard-disk LED and mixed with the emulator audio as a synthesized drive sound.

## WHDLoad Integration

The dedicated **WHDLoad** tab automates game installation, configuration, and one-click execution:

1. Obtain WHDLoad game archives (`.lha` / `.lzh`) from [Aminet](https://aminet.net) or [whdload.de](http://www.whdload.de).
2. Copy the `.lha` files to any accessible folder on your Vita (`ux0:` or `uma0:`).
3. Open the **WHDLoad** tab and select **Install Game from LHA**.
4. Choose the archive: a live progress bar displays real-time extraction percentage, file count, and the active file name.
5. The game is extracted into `ux0:/data/uae4all/whdload/<GameName>/`.
6. Selecting the extracted game from the list:
   - automatically configures the Amiga 1200 hardware profile (68020 CPU, Kickstart 3.1, 2 MB Chip RAM, 4 MB Fast RAM);
   - mounts `ux0:/data/uae4all/whdload/` as hard drive volume `DH0:`;
   - deploys bundled official WHDLoad binaries (`C:WHDLoad`, `C:WHDLoadCD32`, `S:WHDLoad.prefs`, etc.) on first launch;
   - prepares an optimized `Startup-Sequence` that changes directory (`CD`) into the game folder and runs `C:WHDLoad "<Game.slave>" PRELOAD`;
   - launches the game immediately in 1 click!

7. **Use WHDLoad Directory** mounts the library as `DH0:` for manual exploration or custom Workbench use.

## Supported disk images

The browser recognizes common Amiga disk formats including ADF, ADZ, DMS, IPF, ZIP, LHA and LZH. ZIP and LHA/LZH archives selected from the Floppy tab must contain a supported disk image. LHA archives selected from the WHDLoad tab are extracted as game files. DMS archives are decompressed to a temporary ADF when they are mounted, so the game can read and write the disk during the session but changes are not written back into the archive; password-protected archives are refused. IPF images are decoded through the included CAPS image library as read-only media, with protected and variable-density data handled where compatible with the Vita floppy timing path. Compatibility still depends on the quality and variant of the IPF dump.

## CD32 support

- Open `Hardware -> CD32 CD Image` and select an `.iso`, raw `.bin`, `.cue`, `.chd` or `.m3u`.
- Mounting a CD image automatically selects the CD32 Kickstart pair, AGA chipset, 2 MiB chip RAM, 68020 CPU mode and one floppy drive; the settings can still be changed before rebooting.
- CUE files can reference separate data and audio BIN files and can contain multiple tracks with INDEX 00/01, PREGAP and POSTGAP.
- M3U playlists behave like WinUAE: every line lists a CD image (`.cue`, `.bin`, `.iso` or `.chd`), relative paths are resolved against the playlist folder and empty lines, `#` comments and missing entries are skipped. The item shows `Disc n/N` and LEFT / RIGHT switch disc while the game keeps running.
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
- START: boot / resume in menu; in-game toggles virtual keyboard (VKBD) in non-CD32 games, or acts as CD32 Play/Pause in CD32 games

### In-Game Hotkeys

- `R + START + D-Pad Up / Down`: Adjust fine scanline panning (vertical screen offset). When Auto Display is enabled, the viewport is automatically centered and displays an OSD notification (`Auto Display: Centered`).
- `R + START + D-Pad Left / Right`: Live display resolution switching. In Auto Display mode, this shortcut is safely locked to prevent display corruption, displaying an OSD overlay notification (`Auto Display: Locked`).
- `L trigger`: Open Quick Menu (savestates, disk swapping, screenshot, resume).

The Controls tab covers joystick port, autofire rate (Slow, Medium, Turbo), autofire trigger mode (Hold Button / Continuous), mouse multiplier, mouse emulation, stylus offset, tap delay, custom control sets (including the Pinball Dreams / Slam Tilt presets) and the Floppy/HDF sound volume.

## Display

The Display tab provides shaders, video preset modes (200 to 286 lines, NTSC/PAL, 5:4 and fullscreen variants), footer size, screen offset, cut left/right, frameskip, background, font, and the virtual keyboard language and style.

### Auto Display & Auto Center

UAE4ALL2 HD introduces an intelligent **Auto Display** engine (enabled by default) that tracks the Amiga hardware Display Window (`DIWSTRT` / `DIWSTOP`) registers and copper lists in real time:

- **Automatic Centering & Overscan**: Dynamically detects the active picture area generated by games and demos, removing black borders and automatically centering the image vertically and horizontally on the PlayStation Vita screen.
- **Menu Display Hints**: When `Auto Display` is set to `Auto`, manual resolution selection and screen offset in the Display tab automatically reflect their active managed state (`[Auto Dynamic]` and `[Auto Centered]`).
- **No Surface Recreation Freezes**: Changing display settings or returning from the menu maintains the hardware video surface, completely avoiding black flashes or emulation freezes.
- **Manual Mode**: If you prefer full manual control, set `Auto Display` to `Off`. This unlocks manual resolution presets (from 200 to 286 lines), independent `Screen Offset` (-32 to +32 lines), `Cut Left/Right`, and the manual `Auto Center` toggle.

## Disk activity lights and sounds

The in-game overlay provides a clean, native UAE status bar showing floppy drive (DF0-DF3) and hard disk (HDF) activity. Floppy lights follow motor and DMA activity. The HDF light indicates read and write operations. The status bar can be configured in the Display menu (Bottom Bar with FPS/tracks, Top Bar, Clean Screen, or Vertical Bar). Redundant top-right activity boxes have been removed to keep the game screen uncluttered.

The audio backend loads short segments from the bundled Ogg samples for floppy and hard-disk activity, then mixes them with the normal emulator audio. The Controls tab includes a separate 0-100% volume setting for these disk sounds. If a sample cannot be opened, the mixer falls back to the built-in synthesized effect. The floppy sample is `Floppy drive sounds.ogg` by AlepouTheFox, released under CC0 1.0. The hard-disk sample is `WD bad heads click of death.ogg` by Zzptichka, released into the public domain.

The original sample sources are:

- https://commons.wikimedia.org/wiki/File:Floppy_drive_sounds.ogg
- https://commons.wikimedia.org/wiki/File:WD_bad_heads_click_of_death.ogg

The packaged copies are `psp2data/data/sounds/floppy_drive.ogg` and `psp2data/data/sounds/hard_drive.ogg`. The Vita build links Vorbis decoding support so the samples can be loaded from `app0:/data/sounds/`.

## System tab

- **Save Game Configuration**: writes `ux0:/data/uae4all/conf/uaeconfig.conf` (or a per-game config derived from the mounted disk name). This is the only automatic write point in the Vita menu.
- **Restore Default Settings**: resets CPU, chipset, memory, Kickstart, floppies, HDFs, CD, display, audio and controls to factory defaults in memory (does not write any file).
- **Reboot Amiga Emulation**: hard resets the Amiga with the current settings.
- **Take Screenshot**: captures the next emulated frame as a PNG.
- **About**: version 1.11 with scrolling credits.
- **Startup**: displays `Loading UAE4ALL2 HD...` before the main interface is opened.
- **Release notes**: see [`CHANGELOG.md`](CHANGELOG.md).

## Building for Vita

Install VitaSDK and its SDL, SDL_image, zlib, libarchive, PNG and JPEG development packages. The included Vita2D and shader libraries are linked by CMake.

```bash
export VITASDK=/usr/local/vitasdk
mkdir build
cd build
cmake .. -DBUILD_PSP2=ON -DCMAKE_BUILD_TYPE=Release
ninja uae4all2.vpk
```

The build output is `uae4all2hd.vpk`. The Vita package uses Title ID `UAE4ALLHD` and application version `01.11`, so it installs separately from the legacy UAE4ALL2.

The FTP implementation links the VitaSDK `ftpvita` library, matching the service integration used by VitaArchive. Ensure the VitaSDK installation includes the `ftpvita` development library before building.

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

- **Chui**, **john4p**, **TomB**, **notaz**, **Bernd Schneider**, **Toni Wilen**, **Pickle**, **smoku**, **AnotherGuest**, **Anonymous engineer**, **finkel**, **Lubomyr**, **pelya** (Original UAE4ALL / UAE4ALL2 authors)
- **Cpasjuste** for the original Vita port, SDL-Vita work, shader support and performance improvements
- **rsn8887** for the Vita/Switch work and the UAE4ALL2 improvements (https://github.com/rsn8887/uae4all2)
- **theheroGAC** for the UAE4ALL2 HD Vita project, HD menu design, WHDLoad integration, CD32 & HDF enhancements (https://github.com/theheroGAC/UAE4ALL2-HD-VITA)
- **ScHlAuChi** for testing, ideas and virtual-keyboard contributions
- **wronghands** for the menu font, keyboard styles and design ideas
- **CrashMidnick** for the French virtual keyboard
- **Xerpi** and **frangarCJ** for Vita2D and shader-library work
- **The VitaSDK Team** for VitaSDK toolchain and libraries
- **Bert Jahn (Wepl)** for WHDLoad (http://www.whdload.de)
- **Aminet** for the Amiga software and WHDLoad game archive (https://aminet.net)
- **SPS (Software Preservation Society)** for CAPS / IPF image decoding support

Please preserve the original project credits and license notices when redistributing or modifying this project.

## Legal notice

UAE4ALL2 HD is intended for use with legally acquired Amiga software and Kickstart ROMs. Amiga, UAE4ALL and related trademarks belong to their respective owners. This repository does not include copyrighted Kickstart ROMs, commercial games or commercial disk images.

## License

See `copying` and the original source files for the applicable license notices.
