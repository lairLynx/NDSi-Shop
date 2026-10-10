# DSi-Shop (NDSi-Shop)

> An alternative shop for the Nintendo DS / DSi family of systems.

## Features

- Browse and search databases of NDS games (with boxart)
- Download `.nds` directly to SD card
- ZIP file extraction
- Database loading via URL or file
- Multiple database support
- In-app update check (from the Info screen)
- 7 customizable color schemes
- Multi-language support (EN, FR, NL, IT)

## Download

### DSi

- Download `DSi-Shop.nds` from the [latest release](https://github.com/lairLynx/NDSi-Shop/releases/latest).
- Place it anywhere on your SD card. App data (settings, databases, cache) is stored in `/NDSi-Shop` on the SD card.
- Launch via TWiLight Menu++ or Unlaunch.

## Building

### DSi

Requires [Wonderful Toolchain](https://wonderful.asie.pl/) + BlocksDS.

#### Windows (MSYS2)

```bat
compile.bat
clean.bat
```

#### Linux / macOS / WSL

**Option A — CLI bootstrap (recommended for CI / headless)**

```bash
# Install Wonderful Toolchain
sudo mkdir -p /opt/wonderful
sudo chown -R $USER:$USER /opt/wonderful
curl -fL https://wonderful.asie.pl/bootstrap/wf-bootstrap-x86_64.tar.gz | tar -xz -C /opt/wonderful
/opt/wonderful/bin/wf-pacman -Syu --noconfirm wf-tools

# Setup BlocksDS repo & toolchain
wf-config repo enable blocksds
wf-pacman -Syu --noconfirm
wf-pacman -S --noconfirm blocksds-toolchain blocksds-libcurl blocksds-mbedtls toolchain-gcc-arm-none-eabi-extra toolchain-gcc-arm-none-eabi-zlib
sudo ln -s /opt/wonderful/thirdparty/blocksds /opt/blocksds
```

**Option B — Graphical installer (GUI)**

```bash
# Download and run the installer
curl -sL https://wonderful.asie.pl/bootstrap/wf-installer.sh -o wf-installer.sh
chmod +x wf-installer.sh
./wf-installer.sh
# Follow on-screen instructions

# Then load the environment
source ~/.wonderful/env
```

**Build (common to both options)**

```bash
source /opt/wonderful/bin/wf-env -a
python build.py

# Release build (outputs to release/)
python build.py release
```

## Changes from upstream

This fork diverges from [NDS-Shop-Homebrew/NDS-Shop-DSi](https://github.com/NDS-Shop-Homebrew/NDS-Shop-DSi) in the following ways:

- Rebuilt GUI renderer.
- Renamed to DSi-Shop; the ROM is `DSi-Shop.nds` and app data lives in `/NDSi-Shop` on the SD card. Settings from the old `/NDS-Shop` folder are still read if no new settings file exists.
- The in-app update now replaces the ROM you launched, instead of saving a copy at the SD card root.
- Updates are only offered when the published version is newer than the running one (numeric comparison), and a failed check now shows an error.
- The update check on startup was removed (it was unreliable on slow connections); updates are checked manually from the Info screen. Network requests now have timeouts so they can no longer hang.
- Settings loading is more forgiving: a malformed line no longer discards the rest, and download paths containing spaces are read correctly.
- The app version comes from `version.txt` in the repo root rather than git tags.

## Screenshots

### DSi

<table>
<tr>
<td align="center"><b>Home</b></td>
<td align="center"><b>Browse</b></td>
<td align="center"><b>Settings</b></td>
</tr>
<tr>
<td><img src="ressources/screenshoots/DSi/NDS-Shop-DSi_1_EN.png" width="256"></td>
<td><img src="ressources/screenshoots/DSi/NDS-Shop-DSi_2_EN.png" width="256"></td>
<td><img src="ressources/screenshoots/DSi/NDS-Shop-DSi_3_EN.png" width="256"></td>
</tr>
</table>

## Credits

This project is based on and builds on the original work from [NDS-Shop-Homebrew/NDS-Shop-DSi](https://github.com/NDS-Shop-Homebrew/NDS-Shop-DSi), created and maintained mainly by [TheRinzler65](https://github.com/TheRinzler65).
The original project is released under the [MIT License](https://opensource.org/licenses/MIT), and this version continues that attribution.

Additional inspiration and related work:
- [Kekatsu DS](https://github.com/cavv-dev/Kekatsu-DS) by cavv-dev (MIT licensed)
- Database: [UDB-Kekatsu-DS](https://github.com/cavv-dev/UDB-Kekatsu-DS)
- Concept inspired by [pkgi-psp](https://github.com/bucanero/pkgi-psp) and [Universal-Updater](https://github.com/Universal-Team/Universal-Updater)