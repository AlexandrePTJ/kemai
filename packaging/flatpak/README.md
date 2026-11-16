# Kemai Flatpak

`io.github.AlexandrePTJ.Kemai.yml` is the reference manifest: it builds the working tree. The manifest published on
Flathub for a release is written from it by `.github/scripts/flathub-manifest.sh <version> <commit> <channel>`.

```shell
flatpak-builder --user --install --force-clean build-dir packaging/flatpak/io.github.AlexandrePTJ.Kemai.yml
flatpak run io.github.AlexandrePTJ.Kemai
```

Without a local Flatpak SDK, Flathub's CI image builds it:
`docker run --privileged -v $PWD:/src ghcr.io/flathub-infra/flatpak-github-actions:kde-6.11 flatpak-builder --disable-rofiles-fuse ...`.

## Dependencies

Qt comes from the `org.kde.Platform` 6.11 runtime, as do libsecret and the X11 screensaver extension. fmt, magic_enum,
spdlog and qtkeychain are modules of the manifest, pinned to the versions of the `CPMAddPackage` lines of the root
`CMakeLists.txt`: change both together. Kemai is built with `-DKEMAI_BUILD_LOCAL_DEPENDENCIES=OFF` (no download during
the build), `-DKEMAI_DEPLOY_QT=OFF` (no copy of Qt) and `-DKEMAI_ENABLE_UPDATE_CHECK=OFF` (Flathub delivers updates).

## Permissions

| Permission | Why |
|---|---|
| `--share=network` | Talking to the user's Kimai server |
| `--socket=wayland`, `--socket=fallback-x11`, `--share=ipc` | The window, on Wayland or X11 |
| `--device=dri` | Qt Quick renders with the GPU |
| `--talk-name=org.freedesktop.secrets` | API tokens in the Secret Service keychain (GNOME, most desktops) |
| `--talk-name=org.kde.kwalletd6` | API tokens in KWallet on KDE Plasma |
| `--talk-name=org.kde.StatusNotifierWatcher` | The tray icon (StatusNotifierItem) |
| `--talk-name=org.gnome.Mutter.IdleMonitor` | Idle time on GNOME, to stop the timer when the user is away |
| `--talk-name=org.freedesktop.ScreenSaver` | Idle time on KDE Plasma, same feature |

Not requested: the user's files (settings, profiles and logs live in `~/.var/app/io.github.AlexandrePTJ.Kemai/`; links
and the log folder open through the portals), and logind on the system bus, which Flathub refuses: stopping the timer
when the session is locked shows as unsupported in the settings of the Flatpak.
