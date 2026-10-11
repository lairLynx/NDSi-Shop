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

Compared with [NDS-Shop-Homebrew/NDS-Shop-DSi](https://github.com/NDS-Shop-Homebrew/NDS-Shop-DSi), this fork changes:

**Repo mode**
- The app starts on a Repositories screen listing hardcoded GitHub repositories (currently [lairLynx/Lynxibear-DS](https://github.com/lairLynx/Lynxibear-DS)). Selecting one downloads the `.nds` from its latest release into your downloads directory, replacing the previous copy, and remembers the installed version. The database browser is still available from the Browse tab.

**Interface**
- Startup splash: the lairLynx Studios logo and credit on the top screen and "Made using BlocksDS" on the bottom screen, shown while the app initializes (any key or a touch skips it once ready). Initialization errors are reported on the bottom screen.
- Rebuilt GUI renderer: a software-rendered framebuffer replaces the sub-screen sprite/bitmap setup, with theme colors resolved by the renderer.
- Box art is downloaded at a reduced size (128 px) and cached by URL under `cache/boxart`. If that fails it falls back to the original image, then to a thumbnail looked up by game name. A "Loading box art..." indicator is shown meanwhile.
- The results list shows a live box art preview on the top screen for the highlighted game.
- The top screen uses a gradient per color scheme instead of a flat dark background with the brick logo.
- Folder picker for choosing the downloads directory.
- New settings to show or hide box art entirely, and to cache it or not (both on by default). With caching off, images are deleted right after being displayed.

**Networking**
- Wi-Fi is connected on demand and waited for (up to 15 s) instead of at startup. Download and database errors now clearly report "Wi-Fi connection unavailable".
- Box art downloads keep the UI responsive, and version checks have timeouts so they can no longer hang.

**Databases**
- Universal-DB is included in the default databases and is re-added to `databases.txt` if missing.

**Updates**
- The in-app update replaces the ROM you launched, instead of saving a copy at the SD card root.
- An update is only offered when the published version is newer than the running one (numeric comparison). Failed checks show an error.
- The check on startup (and its setting) was removed. Updates are checked manually from the Info screen.
- The app version comes from `version.txt` in the repo root instead of git tags.

**Naming and data**
- Renamed to DSi-Shop: the ROM is `DSi-Shop.nds`, app data lives in `/NDSi-Shop`, and the Info screen credits this fork. Settings from the old `/NDS-Shop` folder are still read if no new settings file exists.

**Settings**
- A malformed line in the settings file no longer discards the rest, and download paths containing spaces are read correctly.

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
- Top screen gradients: color pairs from [uiGradients](https://github.com/ghosh/uiGradients) by Indrashish Ghosh (MIT licensed)
- Concept inspired by [pkgi-psp](https://github.com/bucanero/pkgi-psp) and [Universal-Updater](https://github.com/Universal-Team/Universal-Updater)