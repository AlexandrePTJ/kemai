# Design

## Context

See proposal.md for motivation and specs/ for the required behaviour. Current state that shapes the approach:

- The history Edit button opens a menu ("Edit start time" / "Edit end time") that opens `DateTimeEditDialog` for one datetime; `SessionController::updateTimeSheetTimes(id, begin, end)` sends the change (invalid datetime = unchanged, rejects `end <= begin`).
- `KimaiTimeSheet` already carries `description` and `tags`; `toJson()` already sends both (`tags` joined by commas). `TimesheetModel` exposes `description` but no `tags` role.
- `KimaiClient` has GET/POST/PATCH only. Reply handling is duplicated in `kimaiClient_p.h` (`processApiNetworkReplySingleObject` / `processApiNetworkReplyArray` differ only in how the JSON document is turned into a result).
- Errors follow the CLAUDE.md strategy: client types them (`KimaiApiError`), `SessionController::handleApiError` decides (401 → `sessionExpired`), `NotificationBanner` displays.
- `Main.qml` handles `sessionExpired` by replacing the stack with the login view; the `SessionContext` (parented to `KemaiContext`) is not destroyed.
- Auto-login is stored by `KemaiContext::updateAutoLoginSettings(host, token, remember)`.
- Modal dialogs use the `Popup` idiom (never Qt Quick Controls `Dialog`); `ActivityComboBox` offers fuzzy project — activity search over the cache.

## Goals / Non-Goals

**Goals:**
- One edit dialog per entry, faithful to the mockup, reusing the existing picker, combo box and error channel.
- Every new server call goes through `SessionController` and `handleApiError`, and is unit-tested with `FakeKimaiClient`.
- Client reply processing written once, so `DELETE` (no body) does not add a third copy.

**Non-Goals:**
- Settings beyond "Log out" (no preferences screen).
- Custom title bar / window buttons from the mockup (platform window kept).
- A description field in the timer bar (the mockup has none; description is set via the dialog).
- Creating customers/projects/activities from the client.
- Offline editing or optimistic UI updates.

## Decisions

**1. New `TimeEntryEditDialog`; `DateTimeEditDialog` becomes its picker.**
The Edit button opens `TimeEntryEditDialog` directly (the start/end menu is removed). Its start/end chips open `DateTimeEditDialog`, which keeps its current role of editing one datetime and returns the value to the dialog instead of saving. This keeps the June decision intact (the picker stays a correction tool) while adding the form the mockup shows.
*Alternative*: grow `DateTimeEditDialog` into the full form — rejected, it would mix two responsibilities and break its single-datetime contract.

**2. Project/activity chosen as a valid pair from the cache.**
Functional constraint for the brief: the user picks a project — activity pair among the cache's consolidated pairs (global activities fanned out per project), with search, so an invalid pair cannot be saved. Whether this is one search field (like the timer bar's `ActivityComboBox`) or two dependent selects (as in the mockup) is a visual choice made in Claude Design; the implementation follows the handoff.

**3. One general `SessionController::updateTimeSheet(...)`.**
Signature carrying timesheet id, project id, activity id, description, tags, begin, end; it supersedes `updateTimeSheetTimes`, keeps its validation (`end <= begin` rejected, active entry keeps no end) and refreshes active + recent timesheets on success. The dialog holds edits locally and calls it once on Save (spec: changes applied only on save).
*Alternative*: one invokable per field — several requests per save and partial updates on failure.

**4. Client: `deleteTimeSheet(id)` and `requestTags(name)`, after factoring reply processing.**
In `kimaiClient_p.h`, a single helper handles finished/error/exception/`deleteLater` and takes a parser callable; `SingleObject`, `Array` and a new body-less `QFuture<void>` variant become thin wrappers. `sendDeleteRequest` uses `QNetworkAccessManager::deleteResource`. `requestTags` calls `GET /api/tags/find?name=` and returns tag names. Both are added to `IKimaiClient` and `FakeKimaiClient` (with `failWith` support).

**5. Tags input as its own component, suggestions fetched on demand.**
A `TagsInput` QML component (pills + text field) queries suggestions through `SessionController` as the user types (debounced), rather than loading all tags in `KimaiCache`. Tags are not part of the cache's all-or-nothing contract, and an instance can hold many tags. New tags are simply sent with the entry; Kimai creates them on save.

**6. Deletion always confirmed; refresh is the source of truth.**
The brief requires an explicit confirmation before deleting; its form (inline like `ProfileEntryDelegate`, or otherwise) comes from the handoff. `SessionController::deleteTimeSheet(id)` refreshes active + recent on success, which stops the timer if the active entry was deleted.

**7. Logout through `KemaiContext::logout()`.**
The main view's logout entry point (placement and form from the handoff) calls `appContext.logout()`, which clears auto-login (`updateAutoLoginSettings("", "", false)`) and emits `loggedOut()`; `Main.qml` then replaces the stack with the login view — the same navigation as `sessionExpired`. In both cases the session is released by `KemaiContext::closeSession()` from the `MainView`'s `Component.onDestruction`, not at logout time: deleting it earlier would leave the outgoing view's transition bound to a destroyed object. This C++/navigation part does not depend on the handoff and is done first.

**8. CI integration tests via secrets only.**
The test step gets `KEMAI_TEST_HOST`/`KEMAI_TEST_TOKEN` from repository secrets. The tests already skip when the variables are empty, so forks and unconfigured repositories pass. Which instance to use is a repository setting, not code.

**9. Screens: functional brief → Claude Design → handoff.**
Every screen this change creates (the Edit Time Entry dialog with its tags and delete parts; the logout entry point) starts as a functional brief in `screens/<screen>.md` inside this change: purpose, entry points, data shown, actions, states, validation, out of scope — no visual prescription. Alexandre designs it in Claude Design and hands back a handoff; QML is written from that handoff only. A handoff that contradicts the specs is raised, not silently resolved. Visual choices elsewhere in this design (decisions 1, 2, 6, 7) are defaults the handoff may override, within the specs. Briefs are written early so design runs in parallel with the non-UI work.
*Alternative*: implement from the mockup directly — rejected, visuals are Alexandre's call.

## Risks / Trade-offs

- [Groups 7–10 wait on handoffs] → briefs are written first and all non-UI work (client, session logic, logout plumbing, CI) is ordered before them.

- [Kimai refuses unknown tags when the user lacks the "create tag" permission] → the save fails with a typed error and the banner shows the permission message; acceptable for an alpha.
- [`/api/tags/find` response shape varies across Kimai versions (names vs objects)] → parse both shapes in `requestTags`, covered by an integration test on the test instance.
- [Removing the Edit menu changes a workflow users may know] → the dialog's start/end chips give the same two actions in one more click.
- [Deleting the active entry races with the 1 s duration timer] → refresh after deletion is the single source of truth; the timer stops when the refreshed active list is empty.
- [The CI instance may not be reachable from GitHub runners] → integration tests are skipped without secrets; the repository owner adds secrets only for a reachable instance (e.g. the public Kimai demo).

## Migration Plan

No data migration. `updateTimeSheetTimes` is removed together with its only caller; its tests move to `updateTimeSheet`. Each task group is committed on its own (short one-line messages, no co-author trailer), so any group can be reverted independently.
