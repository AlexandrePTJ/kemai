# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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


[Unreleased]: https://github.com/AlexandrePTJ/kemai/compare/0.10.0...HEAD
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
