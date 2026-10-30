# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project uses [calendar versions](https://calver.org/) (`YY.MONTH.PATCH`) since the rewrite; versions up to 0.11.1 followed [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
Alpha releases are marked by ` [alpha]` after their date (`## [26.10.1] - 2026-10-26 [alpha]`); the release metadata shown by Linux software centers lists them as development releases.

## [Unreleased]

### Added
- Signed Windows packages, once the project is accepted by SignPath Foundation: a portable ZIP for alpha releases, an installer for stable releases, which asks to uninstall Kemai 0.11.x first
- Linux: desktop entry, icon and AppStream metadata installed with Kemai; a Flatpak manifest, for Flathub
- Profiles of Kemai 0.11 found on the same computer are offered for import at start, with their API tokens moved to the system keychain
- Required custom fields of time entries (Kimai's custom fields plugin) are asked for before a timer starts when they would be left empty, and shown and checked when editing an entry
- The history can be filtered by text (project, activity, description, tags) within the period it shows

### Changed
- Versions have a two-digit year (`26.10.1`), the same number everywhere, Windows installer included; the first alpha, published as 2026.10.1, does not see later releases in its update check
- Debug messages are no longer printed on the console (hence the system journal) unless Kemai is started with `--verbose`; log files and the log journal still have them
- The history shows a period — this week by default, this month or the last 30 days, remembered — named in its header with its dates, instead of the latest 50 entries; a timer running since before the period stays listed, entries planned later in the period are marked
- With Kimai's Task Management plugin, the History | Tasks switch moved to the right of the timer status line

### Fixed
- Log files no longer lose their last lines when Kemai crashes or is killed
- The day and week totals of the history count every entry of the week, not only those among the latest 50 listed

## [26.10.1] - 2026-10-26 [alpha]

First alpha of the complete rewrite (Qt 6 / QML). Source-only: no binary is provided, build it as shown below. It can be installed next to Kemai 0.11.x: it keeps its own settings and never modifies 0.11.x's (profiles are not imported).

### Added
- New QML interface: timer bar, history of recent time entries with day labels, "Yesterday" and running entry first
- Connection profiles with tokens stored in the system keychain, auto-login, logout from the account menu
- Edit dialog for time entries: project and activity search, description, start and end, tags (new tags are created on Kimai), delete
- Creating customers, projects and activities from the project and activity search
- Task Management plugin: list, start and close your open tasks (shown only when the plugin is installed)
- Running timer kept in sync with the web interface and other clients
- Stopping the timer when idle or when the session is locked (optional, in the settings)
- Settings dialog; window size and position remembered
- System tray icon showing whether a timer runs (project, activity and duration in its tooltip), with a menu to show the window, stop the timer or quit; optional close and minimize to the tray (on Linux, needs a desktop that hosts tray icons)
- Trusting a self-signed certificate for one server, after checking its fingerprint; trusted certificates listed in the settings
- Log files and a log journal (filter, copy for a bug report, open the log folder), without any API token in them
- Update check at start and on demand: a notice links to the release page of a newer version of your channel (nothing is downloaded); About dialog; open Kimai in the browser
- Projects and activities cache refreshed on demand and when it gets old; clear messages when the network or the server fails
- Calendar versions (`YY.MONTH.PATCH`), alpha releases from the `develop` branch

### Not there yet (compared with 0.11.1)
- Translations

Login requires an API token (Kimai 2.14 or later): login with username and API password is no longer supported.

## [0.11.1] - 2024-06-30

### Fixed
- Fix missing log directory at first startup (Thanks to @ensemblebd) 
- Settings management


## [0.11.0] - 2024-05-22

### Fixed
- Fix combo selection after model reset (Thanks to @virtulis)

## Added
- Template button to reload previous timesheet without starting it (Thanks to @poelzi)
- Allow usage of API Token for kimai>=2.14.0


## [0.10.0] - 2023-07-17

### Fixed
- Bad MacOSX layout resizing [#96](https://github.com/AlexandrePTJ/kemai/issues/96)

### Added
- About dialog [#95](https://github.com/AlexandrePTJ/kemai/issues/95)
- Recent time sheets history [#86](https://github.com/AlexandrePTJ/kemai/issues/86)
- Log journal widget [#58](https://github.com/AlexandrePTJ/kemai/issues/58)


## [0.9.4] - 2023-05-10

### Changed
- Force gcc-10 for linux build to have better compatibility over linux distros


## [0.9.3] - 2023-05-08

### Fixed
- Sort regression [#84](https://github.com/AlexandrePTJ/kemai/issues/84).
- SSL problems on some linux distro [#65](https://github.com/AlexandrePTJ/kemai/issues/65).
- Profile name not updated in menu [#82](https://github.com/AlexandrePTJ/kemai/issues/82).
- List projects with same name [#83](https://github.com/AlexandrePTJ/kemai/issues/83).

### Changed
- Replace conan for dependencies management with CMake built-in FetchContent


## [0.9.2] - 2023-04-04

### Fixed
- Profiles not saved [#81](https://github.com/AlexandrePTJ/kemai/issues/81).


## [0.9.1] - 2023-03-28

### Added
- Add 'refresh cache' menu item 

### Fixed
- Fix filter on Customer / Projects / Activities


## [0.9.0] - 2023-03-27

Special thanks to @shrippen for its support through [sponsorship](https://github.com/sponsors/AlexandrePTJ).

### Added
- Periodic check of current timesheet [#80](https://github.com/AlexandrePTJ/kemai/issues/80).

### Fixed
- Regression resetting dropdown fields when stopping timesheet [#77](https://github.com/AlexandrePTJ/kemai/issues/77).

### Changed
- Allow selection of project without first selecting customer [#74](https://github.com/AlexandrePTJ/kemai/issues/74).
- Improve Loading of Customer / Projects / Activities [#44](https://github.com/AlexandrePTJ/kemai/issues/44).
- Improve filter on Customer / Projects / Activities [#75](https://github.com/AlexandrePTJ/kemai/issues/75).


## [0.8.0] - 2023-03-14

### Added
- TimeSheet description and tags are saved on stop [#43](https://github.com/AlexandrePTJ/kemai/issues/43).
- Option to enable/disable Kemai's update check [#68](https://github.com/AlexandrePTJ/kemai/issues/68).
- Can stop current timesheet when idle is detected [#63](https://github.com/AlexandrePTJ/kemai/issues/63).
- Czech translation (Thanks to @office256) 
- Dutch translation (Thanks to Thomas)
- Turkish translation (Thanks to Mert Saraç)

### Fixed
- Enhance MacOSX generated dmg [#54](https://github.com/AlexandrePTJ/kemai/issues/54).

### Changed
- Migrate to GitHub Actions
- Changelog format
- Kimai API client refactoring


## [0.7.1] - 2022-06-21

### Fixed
- Settings not saved [#51](https://github.com/AlexandrePTJ/kemai/issues/51).
- Missing translations [#52](https://github.com/AlexandrePTJ/kemai/issues/52).


## [0.7.0] - 2022-06-19

### Added
- Use connection profiles to allow several kimai endpoints [#46](https://github.com/AlexandrePTJ/kemai/issues/46).
- Minimize to tray [#47](https://github.com/AlexandrePTJ/kemai/issues/47).
- Add greek translations (Thanks to @dkstiler).
- 
### Fixed
- Fix tab order (Thanks to @sdreilinger) [#50](https://github.com/AlexandrePTJ/kemai/issues/50).


## [0.6.0] - 2021-11-11

### Added
- Add translations (de, fr, hr, nb_NO) [#23](https://github.com/AlexandrePTJ/kemai/issues/23).
- Setup connection to Kimai without the need to restart [#21](https://github.com/AlexandrePTJ/kemai/issues/21).
- Support 'timetracking' mode [#37](https://github.com/AlexandrePTJ/kemai/issues/37).
- Add preliminary support to Kimai TaskManagement plugin [#25](https://github.com/AlexandrePTJ/kemai/issues/25). 

### Changed
- Migrate to Qt 6 [#27](https://github.com/AlexandrePTJ/kemai/issues/27).


## [0.5.0] - 2020-10-23

### Added
- Provide AppImage for Linux [#17](https://github.com/AlexandrePTJ/kemai/issues/17).
- Save / Restore window geometry [#16](https://github.com/AlexandrePTJ/kemai/issues/16).
- Add description and tags to activity widget [#15](https://github.com/AlexandrePTJ/kemai/issues/15).


## [0.4.0] - 2020-08-20

### Changed
- Set token edit field as password [#14](https://github.com/AlexandrePTJ/kemai/issues/14).
- Better handling of SSL errors [#13](https://github.com/AlexandrePTJ/kemai/issues/13).
- Change icon when timer is running (Thanks to @maxguru) [#12](https://github.com/AlexandrePTJ/kemai/pull/12).

### Fixed
- Fix sizes policies (Thanks to @maxguru) [#12](https://github.com/AlexandrePTJ/kemai/pull/12).


## [0.3.1] - 2020-06-11

### Changed
- Allow usage of CMake >= 3.11 [#10](https://github.com/AlexandrePTJ/kemai/issues/10)


## [0.3.0] - 2020-06-09

### Added
- Add budget and timeBudget to activity, project and customer dialogs
- Add rotating file logger

### Fixed
- Fix 'Not acceptable' [#9](https://github.com/AlexandrePTJ/kemai/issues/9)


## [0.2.1] - 2020-05-20

### Fixed
- Add OpenSSL binaries to Windows installer [#8](https://github.com/AlexandrePTJ/kemai/issues/8).
- Fix Windows installer upgrade process [#7](https://github.com/AlexandrePTJ/kemai/issues/7).


## [0.2.0] - 2020-05-14

### Added
- Add updater [#3](https://github.com/AlexandrePTJ/kemai/issues/3).

### Fixed
- Handle user timezone correctly [#6](https://github.com/AlexandrePTJ/kemai/issues/6).
- Fix host with subpath [#5](https://github.com/AlexandrePTJ/kemai/issues/5).


## [0.1.1] - 2020-05-09

- Fix minimal OSX version [#2](https://github.com/AlexandrePTJ/kemai/issues/2).


## [0.1.0] - 2020-05-04

Initial version.


[Unreleased]: https://github.com/AlexandrePTJ/kemai/compare/0.11.1...HEAD
[0.11.1]: https://github.com/AlexandrePTJ/kemai/compare/0.11.0...0.11.1
[0.11.0]: https://github.com/AlexandrePTJ/kemai/compare/0.10.0...0.11.0
[0.10.0]: https://github.com/AlexandrePTJ/kemai/compare/0.9.4...0.10.0
[0.9.4]: https://github.com/AlexandrePTJ/kemai/compare/0.9.3...0.9.4
[0.9.3]: https://github.com/AlexandrePTJ/kemai/compare/0.9.2...0.9.3
[0.9.2]: https://github.com/AlexandrePTJ/kemai/compare/0.9.1...0.9.2
[0.9.1]: https://github.com/AlexandrePTJ/kemai/compare/0.9.0...0.9.1
[0.9.0]: https://github.com/AlexandrePTJ/kemai/compare/0.8.0...0.9.0
[0.8.0]: https://github.com/AlexandrePTJ/kemai/compare/0.7.1...0.8.0
[0.7.1]: https://github.com/AlexandrePTJ/kemai/compare/0.7.0...0.7.1
[0.7.0]: https://github.com/AlexandrePTJ/kemai/compare/0.6.0...0.7.0
[0.6.0]: https://github.com/AlexandrePTJ/kemai/compare/0.5.0...0.6.0
[0.5.0]: https://github.com/AlexandrePTJ/kemai/compare/0.4.0...0.5.0
[0.4.0]: https://github.com/AlexandrePTJ/kemai/compare/0.3.1...0.4.0
[0.3.1]: https://github.com/AlexandrePTJ/kemai/compare/0.3.0...0.3.1
[0.3.0]: https://github.com/AlexandrePTJ/kemai/compare/0.2.1...0.3.0
[0.2.1]: https://github.com/AlexandrePTJ/kemai/compare/0.2.0...0.2.1
[0.2.0]: https://github.com/AlexandrePTJ/kemai/compare/0.1.1...0.2.0
[0.1.1]: https://github.com/AlexandrePTJ/kemai/compare/0.1.0...0.1.1
[0.1.0]: https://github.com/AlexandrePTJ/kemai/releases/tag/0.1.0
