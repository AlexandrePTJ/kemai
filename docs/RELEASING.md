# Releasing

Pushing a calendar tag `YY.MONTH.PATCH` (`26.11.1`) runs `.github/workflows/release.yml`:

- **`publish`** derives the channel from the branches containing the tagged commit (`release-channel.sh`: `main`/`master` → stable, `develop` → alpha, a GitHub pre-release), writes the notes from the `CHANGELOG.md` section (`release-notes.sh`) and creates the release, source only.
- **`windows`** attaches the signed Windows package, when `KEMAI_WINDOWS_SIGNING` is `true`.
- **`flathub`** opens the pull requests updating Kemai on Flathub, when `KEMAI_FLATHUB` is `true`.

To cut a release: add its changelog section (an alpha's heading ends with ` [alpha]`), commit, tag the commit on the right branch, push the tag.

## Windows build

- **Metadata**: `code/app/kemai.rc.in` gives `Kemai.exe` its icon and `VERSIONINFO` (product `Kemai`, version strings = the display version, numeric `26,11,1,0`, `0,0,0,0` for dev builds). SignPath checks the product name and version against them.
- **Deployment**: `cmake --install` copies Qt with its QML deploy script and the MSVC runtime with `InstallRequiredSystemLibraries` (no `vc_redist`): the installed folder runs anywhere, and is the alpha ZIP.
- **Installer** (release builds only): `cpack -G WIX --config build/release/CPackConfig.cmake -B build/package`, with the WiX .NET tool and its UI and Util extensions. Keep the output folder short (Windows path limit). Per-machine MSI, `Program Files\Kemai`, Start menu and desktop shortcuts, `Kemai-<version>-windows-x64.msi`, MSI version = the release version.
- **Upgrade code** `88815E44-85A0-469C-9740-B4887D456BAA`, the 0.9–0.11.1 installers': later installers upgrade in place, downgrades are refused. **Never change it.**
- **0.11.x**: its installers registered per-user, which a per-machine MSI cannot upgrade, so setup stops while 0.11.x is installed (`packaging/windows/legacy-check.xml`). It is found by `HKCU\Software\Humanity\Kemai` `installed`, or a `Kemai.exe` at the root of `Program Files\Kemai`. Uninstalling 0.11.x keeps its `settings.json`.

## The `windows` job

On `windows-latest`, it builds the channel's package: alpha → `Kemai-<version>-windows-x64.zip` with `Kemai.exe` signed; stable → the MSI, signed, with its `Kemai.exe` signed too.

SignPath Foundation signs it (Windows shows *SignPath Foundation* as publisher), fetching the unsigned file from a one-day workflow artifact. Then the job checks the signatures and version (`windows-check-signature.ps1`), runs the app (`windows-smoke-test.ps1`), and for the MSI checks it refuses to install over 0.11.1, installs once 0.11.1 is gone, runs and uninstalls (`windows-installer-test.ps1`). Only then does it upload the package and regenerate the notes (`release-notes.sh --windows-zip|--windows-msi`). Any failure leaves the release source-only.

| Name | Kind | Value |
| --- | --- | --- |
| `KEMAI_WINDOWS_SIGNING` | variable | `true` to build and attach the package |
| `SIGNPATH_ORGANIZATION_ID` | variable | SignPath organization id |
| `SIGNPATH_PROJECT_SLUG` | variable | SignPath project |
| `SIGNPATH_SIGNING_POLICY_SLUG` | variable | `release-signing`, or `test-signing` for a dry run |
| `SIGNPATH_ARTIFACT_CONFIGURATION_SLUG` | variable | ZIP configuration (`.github/signpath/artifact-configuration.xml`) |
| `SIGNPATH_MSI_ARTIFACT_CONFIGURATION_SLUG` | variable | MSI configuration (`.github/signpath/artifact-configuration-msi.xml`) |
| `SIGNPATH_API_TOKEN` | secret | token of SignPath's CI submitter user |

- SignPath's [GitHub App](https://github.com/apps/signpath) must be installed on the repository, or signing fails with "Failed to retrieve GitHub App token".
- The XML files in `.github/signpath/` mirror the configurations set on SignPath: change both together.
- With `test-signing`, the job only checks that a signature is present and never attaches anything to a release.
- **Approval**: each signing request waits up to four hours for a manual approval. If it comes later, re-run only the failed `windows` job of the same run (*Re-run failed jobs*), then approve the new request; re-running all jobs fails in `publish`.
- **Build-only runs**: *Actions › Release › Run workflow* with a channel and a throwaway version builds and tests the package without signing or publishing anything — except with `test-signing`, where it is also test-signed (whatever `KEMAI_WINDOWS_SIGNING` says) to check the SignPath setup.
