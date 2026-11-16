# Contributing to Kemai

Bug reports, ideas, documentation and code are welcome. Kemai is maintained on its author's free time: answers may take a few days. Everyone follows the [Code of Conduct](CODE_OF_CONDUCT.md).

All work is based on the `develop` branch (the rewrite). The 0.11.x line (`master`) no longer changes.

## Questions, bugs and ideas

- **Questions**: [GitHub Discussions](https://github.com/AlexandrePTJ/kemai/discussions).
- **Bugs**: search the [issues](https://github.com/AlexandrePTJ/kemai/issues), then open one with your Kemai version (*About Kemai*), your Kimai version, what you did, expected and got, and the relevant lines of the *Log journal*. They never contain your token, but check them for names you would rather keep private.
- **Security issues**: never in a public issue. Open one asking for a private way to send your report.
- **Features**: describe the need, not only the solution, and agree on it before writing code. The maintainer designs new screens: describe what the screen is for and what it lets the user do.

## Code

Build as in the [README](README.md#build-from-source); tests are built by default. You also need `clang-format` and Qt's `qmlformat`.

- Branch from `develop`; one pull request = one feature or one fix. Leave unrelated files alone.
- Follow [docs/CODE_STYLE.md](docs/CODE_STYLE.md) (layers, errors, C++ and QML conventions).
- Format only the files you change: `clang-format -i` (C++), `qmlformat -i` (QML).
- New source files start with:

  ```cpp
  // SPDX-FileCopyrightText: 2026 <your name>
  // SPDX-License-Identifier: MIT
  ```

- Never log, store in a plain file or show an API token.
- A dependency added, removed or relicensed also updates `code/app/qml/Views/ThirdParty.js`.

Before opening the pull request:

```shell
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
cmake --build --preset release --target Kemai_qmllint   # must print no "Warning:" line
```

Test what you change; `code/tests/mocks/fakeKimaiClient.h` stands in for a server. Tests against a real server run only with `KEMAI_TEST_HOST` and `KEMAI_TEST_TOKEN` set; they create and delete a time entry, so never use a production server. For interface changes, try them in the app and say on which systems.

Commits: one short line saying what they do (`Stop the timer from the tray menu`). A user-visible change adds a line under `## [Unreleased]` in [CHANGELOG.md](CHANGELOG.md). Open the pull request against `develop` with the why, the linked issue and screenshots for interface changes; CI (Windows, macOS, Linux, QML lint) must pass. Releases are cut when the maintainer decides.

## Using AI tools

AI assistants are welcome, but **you** are the author: you have read, built and tested every line and can answer the review yourself.

- **Bug fixes**: fine. Keep the change to the fix, with a test that fails without it when possible.
- **Features and behavior changes**: start with a written proposal (need, what the user sees and does, out of scope, impact on existing users). Code comes once the maintainer agrees; pull requests without an agreed proposal are closed.
- Say in the pull request which tool you used and for what.
- Do not let an agent open issues, post comments or answer reviews for you.

[AGENTS.md](AGENTS.md) gives agents the project's rules; Claude Code loads it through `CLAUDE.md`.

## License

By contributing, you agree to release your contribution under the [MIT license](LICENSE.txt).
