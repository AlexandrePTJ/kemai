# Contributing to Kemai

Thank you for considering a contribution to Kemai. Bug reports, ideas, documentation and code are all welcome. This guide explains how to take part so that your time, and the maintainer's, is well spent.

Kemai is maintained on its author's free time: reviews and answers may take a few days.

Everyone taking part in the project is expected to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Where things stand

Kemai is being rewritten (Qt 6 / QML) on the `develop` branch, published as alpha releases. The 0.11.x line (`master`) no longer receives changes: bugs and feature requests are handled in the rewrite. Please base all work on `develop`.

## Asking a question

Use [GitHub Discussions](https://github.com/AlexandrePTJ/kemai/discussions) for questions, help with your setup, or to talk about an idea. The issue tracker is for bugs and agreed work.

## Reporting a bug

Search the [existing issues](https://github.com/AlexandrePTJ/kemai/issues) first. If yours is new, open an issue with:

- your Kemai version (*About Kemai* in the account menu shows it, with your system and Qt version) and your Kimai version;
- what you did, what you expected, and what happened instead;
- the relevant log lines: the *Log journal* (account menu, or the login screen) can copy them. They never contain your API token, but check them for server, customer or project names you would rather not make public.

**Security issues**: do not describe a vulnerability in a public issue. Open an issue saying only that you have a security report, and ask for a private way to send it.

## Suggesting a feature

Open a discussion or an issue describing the need, not only the solution: what you are trying to do in Kimai, and why Kemai makes it hard today. For anything beyond a small change, agree on the idea before writing code, so your pull request is not refused for scope reasons.

New screens and dialogs are designed before they are built. Describe what the screen is for, how it is reached, what it shows and what it lets the user do; the maintainer takes care of the visual design.

## Contributing code

### Set up

Follow [Build from source](README.md#build-from-source) in the README. Tests are built by default. You also need `qmlformat` and `qmllint` (shipped with Qt) and `clang-format`.

### Make your change

- Branch from `develop`. One pull request = one feature or one fix.
- Read [docs/CODE_STYLE.md](docs/CODE_STYLE.md): it describes the architecture (what goes in `code/core`, `code/app` and `code/tests`), the error-handling rules, and the C++ and QML conventions.
- Format the files you change, and only those:
  - C++: `clang-format -i <files>` (root `.clang-format`);
  - QML: `qmlformat -i <files>` (root `.qmlformat.ini`).
- Every new source file starts with the SPDX header:

  ```cpp
  // SPDX-FileCopyrightText: 2026 <your name>
  // SPDX-License-Identifier: MIT
  ```

- Never log, store in a plain file, or show an API token.
- Adding, removing or relicensing a dependency (library, Qt module, font, icon set) also updates the credits list in `code/app/qml/Views/ThirdParty.js`, shown in the About dialog.
- Do not change files unrelated to your work (formatting sweeps, `.gitignore`, editor settings).
- Build scripts, CMake files and GitHub workflows are reviewed like code: the signed Windows packages are built from them.

### Check it

Before opening the pull request, on your platform:

```shell
cmake --build --preset release                        # builds the app and the tests
ctest --test-dir build/release --output-on-failure    # runs the tests
cmake --build --preset release --target Kemai_qmllint # must print no "Warning:" line
```

Add or update tests for the behavior you change: `code/tests` has a fake Kimai client (`code/tests/mocks/fakeKimaiClient.h`) so controller and cache logic can be tested without a server. Tests that need a real Kimai server run only when `KEMAI_TEST_HOST` and `KEMAI_TEST_TOKEN` are set; they create and delete a time entry, so never point them at a production server.

For a change in the interface, try it in the running application, and mention in the pull request what you checked and on which systems.

### Commit and open the pull request

- Commit messages: one short line saying what the commit does (`Stop the timer from the tray menu`), no ticket number needed.
- For a user-visible change, add a line under `## [Unreleased]` in [CHANGELOG.md](CHANGELOG.md).
- Open the pull request against `develop`. Explain what it changes and why, link the issue or discussion, and add screenshots for interface changes.
- CI builds and tests on Windows, macOS and Linux, and lints the QML, on every push: enable GitHub Actions on your fork to see the result on your branch before opening the pull request, and make it pass.
- Please do not ask for a release right after a merge: releases are cut from `develop` when the maintainer decides.

## Using AI tools

AI coding assistants are welcome in Kemai, used with common sense. Whatever tool you use, **you** are the author of what you submit: you have read and understood every line, built it, tested it, and can answer review questions about it yourself.

- **Bug fixes**: a pull request written with an AI assistant is fine. Link the issue (or describe how to reproduce the bug), keep the change limited to the fix, and add a test that fails without it when possible.
- **New features and changes of behavior**: these start with a **written proposal**, not with code. Open a discussion or an issue that explains the need, what the user will see and do, what is out of scope, and what it changes for existing users. Code comes once the maintainer has agreed on the proposal. Pull requests for features or behavior changes without an agreed proposal will be closed, however complete they are.
- **Say so**: mention in the pull request which AI tool you used and for what (writing code, tests, investigation…). It does not count against your contribution; it helps the review.
- **Keep the conversation human**: do not let an agent open issues, post comments or answer reviews on your behalf. Check AI-written descriptions before posting them, and keep them short and to the point.
- **Licensing**: only submit code you have the right to release under the MIT license.

[AGENTS.md](AGENTS.md) gives AI coding agents the project's rules and commands. Most agents read it on their own, and Claude Code loads it through `CLAUDE.md`; point any other tool to it.

## Translations

The rewrite is not translatable yet; translation will come in a later alpha. Translations of 0.11.x are managed on [Weblate](https://hosted.weblate.org/engage/kemai/).

## License

Kemai is released under the [MIT license](LICENSE.txt). By contributing, you agree that your contribution is released under the same license.
