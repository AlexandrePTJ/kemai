# Spec Delta

## Purpose

Defines the automated checks every push must pass, so regressions in build, behaviour or QML hygiene are caught before they reach a release.

## ADDED Requirements

### Requirement: Build and unit tests on every platform
CI SHALL build the project and run the unit tests on Linux, Windows and macOS for every push, with the Qt version the project requires.

#### Scenario: Unit test failure
- **WHEN** a pushed commit breaks a unit test on any platform
- **THEN** the CI run fails for that platform

### Requirement: QML must be lint-clean
CI SHALL fail when qmllint reports any warning on the application's QML.

#### Scenario: QML warning introduced
- **WHEN** a pushed commit introduces a qmllint warning
- **THEN** the CI run fails and reports the number of warnings

### Requirement: Kimai integration tests run when credentials are configured
CI SHALL run the integration tests against a Kimai instance when the repository provides its host and API token as secrets, and SHALL skip them without failing when it does not.

#### Scenario: Secrets configured
- **WHEN** the host and token secrets are set
- **THEN** the integration tests run against that instance and a failure fails the CI run

#### Scenario: Secrets absent
- **WHEN** the secrets are not set (for example, on a fork)
- **THEN** the integration tests are reported as skipped and the CI run does not fail because of them
