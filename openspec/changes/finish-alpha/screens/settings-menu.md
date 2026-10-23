# Screen brief — Logout entry point

Functional brief for Claude Design. It says **what** the user must be able to do and in **which situations**; placement, form and visuals are open. Source requirements: `specs/session-logout/spec.md` (requirement names quoted in brackets below).

## Purpose

Let the user end the current Kimai session on purpose — to switch account or instance, or to stop the app from reconnecting automatically — instead of only when the session expires.

## Context of use

- Kemai is a desktop app (Linux, Windows, macOS); main window defaults to 800 × 600.
- After login, the main view shows: a timer bar (running timer, project — activity search, Start/Stop), a "timer running on" hint, then the recent time entries with a header (today/week totals, date, refresh button).
- There is no settings screen today and none is planned for the alpha. The mockup (`kemai-main.html`) places a gear button in a custom title bar; the app keeps the platform's own window frame, so that exact placement is not available as such.
- Before login, the login view lets the user pick a saved profile (name, host, token), type host/token, tick "Auto-login", and connect.

## Entry point

- A **Log out** action reachable from the main view at any time during a session, including while a timer is running. [Logout is available from the main view]
- It should be findable but not prominent: it is used rarely and must not be triggered by accident next to frequent actions (Start/Stop, refresh, edit).
- The entry point may be designed as a container for future settings (only "Log out" exists for the alpha), or as a single action — designer's choice.

## Behaviour

- Choosing **Log out** ends the session and shows the login view. [Log out]
- It disables auto-login: the next start of the app shows the login view and does not connect by itself. [Logout disables auto-login]
- Saved profiles and their stored tokens are kept; the login view still lists them. [Logout keeps saved profiles]
- A running timer is **not** stopped by logging out: it keeps running on the Kimai server and will show again at the next login. Whether to warn the user about this when a timer is running (and whether to ask for confirmation in that case) is a design decision.

## States to design

1. **No timer running** — logout is immediate (or confirmed, designer's choice).
2. **Timer running** — see the note above.
3. **Future settings** (optional) — how the entry point would hold more items later, if designed as a container.

## Out of scope

- A settings screen or any preference other than logging out.
- Managing profiles from the main view (done in the login view).
- Custom window title bar / window buttons.

## Open points for Alexandre

- Confirm before logging out: never, always, or only when a timer is running?
