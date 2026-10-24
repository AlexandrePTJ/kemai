# Kemai, a Kimai Desktop Client

> 🔄 **Complete rewrite in progress**:
The `develop` branch holds a complete rewrite of the application (Qt 6 / QML), published as **alpha** releases, **source only**.
The current stable version is **0.11.1** (`master` branch). Open bugs and feature requests are addressed as part of the rewrite rather than in the 0.11 codebase.

## Build Status

|Develop|Master|Translation|
|:--:|:--:|:--:|
|![Build status](https://github.com/AlexandrePTJ/kemai/actions/workflows/main.yml/badge.svg?branch=develop)|![Build status](https://github.com/AlexandrePTJ/kemai/actions/workflows/main.yml/badge.svg?branch=master)|[![Translation status](https://hosted.weblate.org/widgets/kemai/-/kemai/svg-badge.svg)](https://hosted.weblate.org/engage/kemai/)|

## How to install

### Kemai 0.11.1 (stable)

Pre-built packages are available from [Releases](https://github.com/AlexandrePTJ/kemai/releases/latest). Just select the asset for your OS.

Binaries with `-NoUpdate` suffix will not automatically check for new updates. Those binaries are intended to be used with OS specific package managers.

On macOS, a [Tap](https://github.com/AlexandrePTJ/homebrew-cask) repository is available to install it through `brew`:

```shell script
> brew tap alexandreptj/cask
> brew install kemai
```

### Rewrite (alpha)

Alpha releases of the rewrite are tagged `YEAR.MONTH.PATCH` (for example `2026.11.1`) and listed as pre-releases on the [Releases](https://github.com/AlexandrePTJ/kemai/releases) page. They are **source only**: no binary is provided, see [Build from source](#build-from-source). The release notes give the exact commands for each release.

The rewrite and Kemai 0.11.x can be installed side by side: the rewrite keeps its own settings and never modifies 0.11.x's. Profiles are not imported from 0.11.x.


## Build from source

Requirements, on every platform:
- CMake 3.30 or later and Ninja
- A C++20 compiler (GCC or Clang on Linux, Visual Studio 2022 on Windows, Xcode command line tools on macOS)
- Qt 6.11 or later (desktop, base install). It can be installed with the [Qt online installer](https://www.qt.io/download-qt-installer-oss) or [aqtinstall](https://github.com/miurahr/aqtinstall), for example `aqt install-qt linux desktop 6.11.3 linux_gcc_64` (on Windows, Qt 6.11 needs an aqtinstall newer than 3.3.0, or the online installer)
- On Linux, the Secret Service development files used to store API tokens in the system keychain (`libsecret-1-dev` on Debian/Ubuntu, `libsecret-devel` on Fedora, `libsecret` on Arch)

Other dependencies (spdlog, fmt, magic_enum, qtkeychain, Catch2) are downloaded by CMake during configuration.

```shell script
> git clone https://github.com/AlexandrePTJ/kemai.git
> cd kemai
> cmake --preset release -DCMAKE_PREFIX_PATH=<Qt install dir>/6.11.3/<arch>
> cmake --build --preset release
```

`<arch>` is `gcc_64` on Linux, `msvc2022_64` on Windows and `macos` on macOS. On Windows, run these commands from a *x64 Native Tools Command Prompt for VS 2022*.

The application is built as `build/release/bin/Kemai` (`Kemai.exe` on Windows, `Kemai.app` on macOS). To get a self-contained copy, with the Qt libraries it needs next to it, run `cmake --install build/release --prefix <target dir>`.

To build a release rather than a development version, check out its tag and pass its version and channel, as given in its release notes:

```shell script
> git checkout 2026.11.1
> cmake --preset release -DCMAKE_PREFIX_PATH=<...> -DKEMAI_VERSION=2026.11.1 -DKEMAI_CHANNEL=alpha
```

Without them, Kemai reports itself as a development build (`dev-<commit>`).


## How to use

_Kemai_ connects to your _Kimai_ instance through its API, with an API token (Kimai 2.14 or later): create one in Kimai from your user profile, under *API access*.

Then, on Kemai's login screen, enter the address of your Kimai instance and the token. Saved profiles keep several instances at hand, and the token is stored in the system keychain.

In the settings (gear button), Kemai can stop the running timer when you step away: after a chosen time without keyboard or mouse activity, or when the session is locked. The entry then ends when you left, not when Kemai noticed it. Idle time is read on Windows, macOS, and on Linux under GNOME, KDE Plasma or any X11 session (the latter needs `libxcb-screensaver` at build time); other Wayland desktops do not report it. Session locking is read on Windows, macOS, and on Linux through logind.


## How to help

Ideas, pull requests and translation are welcome.

For the latter (0.11.x for now; the rewrite is not translated yet), [Weblate](https://hosted.weblate.org/engage/kemai/) is used. Here is the current status:

[![Translation status](https://hosted.weblate.org/widgets/kemai/-/kemai/multi-auto.svg)](https://hosted.weblate.org/engage/kemai/)


## Why Kemai

Because "Il n'y a que Maille qui m'aille".
