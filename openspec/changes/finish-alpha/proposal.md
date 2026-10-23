# Proposal

## Why

The Qt6/QML rewrite of Kemai has its foundations in place (testable core, error-handling strategy, cache refresh, clean qmllint), but it still cannot do several things a daily Kimai user needs and that the `kemai-main.html` mockup shows: fully editing an entry, tagging it, deleting it, or logging out. Closing these gaps, plus the leftover quality items identified during the refactoring, is what separates the current state from a usable alpha.

## What Changes

- Replace the "Edit start time / Edit end time" menu of a history row with a full **Edit Time Entry** dialog covering what the mockup shows: project/activity, description, start → end with computed duration, tags. Start and end are picked with the existing `DateTimeEditDialog` picker. Works on the active entry too (end shown as "now", not editable).
- Allow setting the description of the running entry through that dialog (the timer bar stays as in the mockup: project — activity only).
- Edit tags of an entry as pills, with suggestions from the server's existing tags.
- **Delete** an entry from the edit dialog, with an inline confirmation.
- Add a **Log out** action reachable from the main view: drops the session, clears auto-login, returns to the login view.
- Show the day in the history time range for entries not from today ("Yesterday 16:00 – 18:10", "Mon, Sep 7 …") — already implemented, to be committed.
- Run the Kimai integration tests in CI when credentials are provided as secrets.
- Screens are specified first in a functional brief, designed by Alexandre in Claude Design, then implemented from the resulting handoff.
- Internal: factor the duplicated reply-processing code of the REST client (needed anyway to add a body-less `DELETE`). No behavior change.
- Verify in the real app the items only covered by tests or harnesses so far (notification banner, activity list hover, first CI run on Qt 6.11.3).

## Capabilities

### New Capabilities
- `timesheet-editing`: editing an existing time entry — project/activity, description, start/end, tags — including the active entry, with validation.
- `timesheet-deletion`: deleting a time entry from the client, with confirmation.
- `timesheet-history`: how recent entries are listed and labelled (time range with day context).
- `session-logout`: explicitly ending a session from the main view and its effect on auto-login.
- `ci-quality-gates`: the checks CI runs on every push (build, unit tests, qmllint, optional Kimai integration tests).

### Modified Capabilities
<!-- None: openspec/specs/ is empty, no capability exists yet. -->

## Impact

- **Core**: `IKimaiClient`/`KimaiClient` gain `deleteTimeSheet` and `requestTags`; `kimaiClient_p.h` reply processing factored; `SessionController` gains a general `updateTimeSheet(...)` (superseding `updateTimeSheetTimes`), `deleteTimeSheet`, tag access; `TimesheetModel` exposes a `tags` role; `FakeKimaiClient` follows the interface.
- **Screen briefs**: `screens/time-entry-edit-dialog.md` and `screens/settings-menu.md` in this change, input for Claude Design.
- **App/QML**: new `TimeEntryEditDialog` (and a tags input component), built from the design handoff; `TimeEntryDelegate` Edit button opens it; `MainView`/`Main.qml` gain the logout entry point; `KemaiContext` gains `logout()`.
- **Tests**: new cases in `tst_sessionController`, client integration tests for delete/tags (guarded by env vars).
- **CI**: `.github/workflows/main.yml` passes `KEMAI_TEST_HOST`/`KEMAI_TEST_TOKEN` secrets to the test step.
- **Kimai API**: uses `PATCH /api/timesheets/{id}`, `DELETE /api/timesheets/{id}`, `GET /api/tags/find`. No new dependency.
