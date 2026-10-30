# Releasing and Windows packaging

This document describes how a Kemai release is published by `.github/workflows/release.yml`, and how the
Windows package is built, signed and checked. Users' side of it is in the README (*Installation*,
*Code signing policy*).

## Publishing a release

Pushing a calendar tag `YY.MONTH.PATCH` (`26.11.1`) runs the release workflow:

- `publish` checks the tag and derives the channel from the remote branches containing the tagged commit
  (`.github/scripts/release-channel.sh`): `main` or `master` gives a stable release, `develop` only gives an
  alpha (a GitHub pre-release). It writes the notes from `CHANGELOG.md` (`release-notes.sh`) and creates the
  release, with no file: every release is first source-only.
- `windows` attaches the channel's signed Windows package, once Windows signing is enabled (below).
- `flathub` opens the pull requests updating Kemai on Flathub, once `KEMAI_FLATHUB` is `true`.

## Windows build

- **Product metadata.** `code/app/kemai.rc.in`, configured by CMake on Windows, gives `Kemai.exe` its icon and
  its `VERSIONINFO`: product name and file description `Kemai`, version strings = the display version
  (`26.11.1`, or `dev-<commit>` for a development build), numeric version `26,11,1,0` (`0,0,0,0` for a
  development build). The code signing service checks the product name and version against these fields.
- **Deployment.** `cmake --install` copies Qt (libraries, plugins, QML imports) with Qt's QML deploy script,
  and the MSVC runtime (`msvcp140*.dll`, `vcruntime140*.dll`, `concrt140.dll`) next to `Kemai.exe` with
  `InstallRequiredSystemLibraries`. Qt's `vc_redist` installer is left out: the installed folder runs on a
  Windows without the Visual C++ runtime, with no installation.
- **Installer.** A release build (`KEMAI_VERSION` set) on Windows has a CPack WIX package; a development build
  has none. Build it with the WiX .NET tool and its UI and Util extensions installed:

  ```shell
  cpack -G WIX --config build/release/CPackConfig.cmake -B build/package
  ```

  Use a short output folder: a long one makes windeployqt exceed the Windows path length limit.

  The MSI is per-machine, installs in `Program Files\Kemai` with Start menu and desktop shortcuts, and is
  named `Kemai-<version>-windows-x64.msi`. It keeps the upgrade code of the Kemai 0.9–0.11.1 installers,
  `88815E44-85A0-469C-9740-B4887D456BAA`: later stable installers upgrade it in place, and downgrades are refused.
  **Never change this upgrade code.**

  The 0.x installers left `ALLUSERS` unset, so Windows registered them per-user, and Windows Installer cannot
  upgrade a per-user product from a per-machine one. Setup therefore stops while Kemai 0.11.x is installed, asking
  to uninstall it first (`packaging/windows/legacy-check.xml`, added with `CPACK_WIX_PATCH_FILE`). It finds 0.x by
  the `installed` value of `HKCU\Software\Humanity\Kemai` (written by the 0.x installers for the user who installed
  them; "Humanity" was CPack's default vendor), or by a `Kemai.exe` at the root of `Program Files\Kemai` (0.x's
  layout; the rewrite's is in `bin`). Uninstalling 0.11.x keeps its `settings.json`.

  Its version is the release version (`26.11.1`): the two-digit year keeps the first field under the 255 that
  Windows Installer allows, so the installed apps list, the file name, the executable and the application all show
  the same number.

## The `windows` job

It runs after `publish` when the repository variable `KEMAI_WINDOWS_SIGNING` is `true`, on a GitHub-hosted
`windows-latest` runner, and builds the package of the release's channel:

- **alpha**: a portable ZIP, `Kemai-<version>-windows-x64.zip`, of the installed folder with `Kemai.exe` signed;
- **stable**: the installer, signed, with `Kemai.exe` inside it signed too.

Signing goes through SignPath Foundation's free code signing for open source (certificate issued to
SignPath Foundation, which Windows shows as the publisher). The unsigned file is a workflow artifact kept one day,
only for SignPath to fetch. Before anything is attached, the job checks the signatures and product version
(`windows-check-signature.ps1`), starts the signed `Kemai.exe` (`windows-smoke-test.ps1`) and, for an installer,
checks it refuses to install while Kemai 0.11.1 is installed, then installs it once 0.11.1 is removed, runs it and
uninstalls it (`windows-installer-test.ps1`). Then it attaches the
package with `gh release upload` and rewrites the notes with the Windows section (`release-notes.sh
--windows-zip` or `--windows-msi`). If any step fails, the release stays source-only: nothing unsigned is
ever attached.

### Settings

| Name | Kind | Value |
| --- | --- | --- |
| `KEMAI_WINDOWS_SIGNING` | variable | `true` to build and attach the Windows package |
| `SIGNPATH_ORGANIZATION_ID` | variable | SignPath organization id |
| `SIGNPATH_PROJECT_SLUG` | variable | SignPath project |
| `SIGNPATH_SIGNING_POLICY_SLUG` | variable | `release-signing`, or `test-signing` for a dry run |
| `SIGNPATH_ARTIFACT_CONFIGURATION_SLUG` | variable | ZIP configuration (`.github/signpath/artifact-configuration.xml`) |
| `SIGNPATH_MSI_ARTIFACT_CONFIGURATION_SLUG` | variable | MSI configuration (`.github/signpath/artifact-configuration-msi.xml`) |
| `SIGNPATH_API_TOKEN` | secret | token of SignPath's CI submitter user |

The XML files in `.github/signpath/` are the reference copies of the artifact configurations set on SignPath:
change both together. Setting `KEMAI_WINDOWS_SIGNING` to `false` makes releases source-only again.

With `test-signing`, Windows does not trust the test certificate: the job then only checks that a signature is
present.

### Approval

Every signing request waits for a manual approval on SignPath, for up to four hours. If the approval comes later,
the job fails and the release stays source-only: re-run only the failed `windows` job (*Re-run failed jobs*) of the
same workflow run, then approve the new request. Re-running all jobs fails in `publish`, as the release already
exists.

### Build-only runs

Run the workflow by hand (*Actions › Release › Run workflow*) on a branch, choosing a channel and a throwaway
version: the `windows` job builds, installs and tests the package (the installer test included, for stable), then
stops. Nothing is signed, uploaded or published, not even as a workflow artifact. Use it to check a change to the
Windows build before a release.
