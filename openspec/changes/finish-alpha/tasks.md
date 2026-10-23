# Tasks

Every group ends with its own commit: short, explicit one-line message, no body, no `Co-Authored-By`. Builds use the platform preset (`debug-linux` here); "lint clean" means `cmake --build --preset debug-linux --target Kemai_qmllint` prints no `Warning:` line.

Screens follow design.md decision 9: functional brief → Claude Design (Alexandre) → handoff. No screen is implemented before its handoff exists; any conflict between a handoff and the specs is raised with Alexandre, not decided silently. Non-UI groups do not wait for handoffs.

## 1. Pending history labels

- [x] 1.1 Check the uncommitted "Yesterday" time-range work (`formatHelpers`, `timesheetModel.cpp`, `tst_formatHelpers.cpp`) against specs/timesheet-history; verify `ctest` passes (65 tests) and the four `FormatHelpers::formatTimeRange` cases are green
- [x] 1.2 Commit it (e.g. "Show day in history time ranges") and verify `git status` shows no leftover change from this work

## 2. Screen briefs

- [x] 2.1 Write `screens/time-entry-edit-dialog.md`: functional brief of the edit dialog covering specs/timesheet-editing (incl. tags) and specs/timesheet-deletion — purpose, entry points (finished vs running entry), data shown, actions, states (unchanged/edited, invalid range, saving, save/delete failure, delete confirmation), validation, out of scope; verify every requirement of both specs is traceable in the brief and it contains no visual prescription
- [x] 2.2 Write `screens/settings-menu.md`: functional brief of the entry point to "Log out" covering specs/session-logout; verify the same traceability and absence of visual prescription
- [x] 2.3 Commit the briefs (e.g. "Add screen briefs for finish-alpha") and hand them to Alexandre for Claude Design; verify both files are in the commit

## 3. REST client: factored replies, delete, tags

- [x] 3.1 Factor `processApiNetworkReplySingleObject`/`Array` in `kimaiClient_p.h` around one helper taking a parser; verify the build passes and all existing tests (unit + `KimaiClient` integration with `KEMAI_TEST_HOST`/`KEMAI_TEST_TOKEN` set) still pass
- [x] 3.2 Add a body-less `QFuture<void>` reply variant, `sendDeleteRequest`, and `deleteTimeSheet(int id)` to `IKimaiClient`/`KimaiClient`/`FakeKimaiClient`; verify with a new integration test that creates then deletes a timesheet on the test instance
- [x] 3.3 Add `requestTags(const QString &name)` (`GET /api/tags/find`, tolerant to name-list and object-list responses) to the interface, client and fake; verify with an integration test returning at least one known tag of the test instance
- [x] 3.4 Update CLAUDE.md (client layout, reply-processing helper) and commit (e.g. "Add timesheet delete and tag search to client"); verify CLAUDE.md mentions both methods

## 4. Session logic for editing and deleting

- [x] 4.1 Add the `tags` role to `TimesheetModel`; verify with a unit test that a sheet with tags exposes them through the role
- [x] 4.2 Replace `updateTimeSheetTimes` by `SessionController::updateTimeSheet(id, projectId, activityId, description, tags, begin, end)` with the same validation and refresh; keep the current Edit menu working on it; port the two existing tests and add cases for description/tags/project change, verified through `FakeKimaiClient::lastUpdatedTimeSheet`
- [x] 4.3 Add `SessionController::deleteTimeSheet(id)` routing failures through `handleApiError`; verify tests: success refreshes active + recent, deleting the active entry leaves `hasActiveTimesheet` false, 403 notifies `Forbidden`, 401 emits `sessionExpired`
- [x] 4.4 Expose tag suggestions to QML (invokable request + result signal, failures through `handleApiError`); verify a test where the fake's tags reach the signal
- [x] 4.5 Update CLAUDE.md (Timesheet editing pattern replaces "Timesheet time editing") and commit (e.g. "Add timesheet update and delete to session"); verify `ctest` passes

## 5. Logout without UI

- [ ] 5.1 Add `KemaiContext::logout(session)` clearing auto-login and deleting the session; make the `sessionExpired` path in `Main.qml` delete the dropped session too; verify with logs that the session is destroyed on expiry and when `logout` is called from a QML harness
- [ ] 5.2 Update CLAUDE.md (login flow, logout) and commit (e.g. "Add logout to app context"); verify the build passes

## 6. CI integration tests

- [ ] 6.1 Pass `KEMAI_TEST_HOST`/`KEMAI_TEST_TOKEN` secrets to the CI test step; verify the workflow YAML parses and, locally, that the tests skip with empty variables
- [ ] 6.2 Update CLAUDE.md ("CI (future)" becomes current) and commit (e.g. "Run integration tests in CI when secrets are set")

## 7. Edit Time Entry dialog (from handoff)

- [ ] 7.1 Receive the Claude Design handoff for the edit dialog; verify it is available locally and check it against specs/timesheet-editing and specs/timesheet-deletion, raising any conflict with Alexandre before coding
- [ ] 7.2 Implement the dialog's project/activity, description, start/end (through `DateTimeEditDialog` in value-returning mode) and duration per the handoff; verify in a QML harness capture that a finished and a running entry match the handoff (running: end "now", not editable)
- [ ] 7.3 Implement save validation (no save while end ≤ start) and Save/Cancel wired to `updateTimeSheet`; verify in the harness that an inverted range blocks saving, and in the app that a changed description shows in the history and Cancel changes nothing
- [ ] 7.4 Point the history Edit action at the dialog and remove the old start/end menu; verify in the app on a finished and on the running entry
- [ ] 7.5 Update CLAUDE.md (dialog, picker role) and commit (e.g. "Add Edit Time Entry dialog"); verify lint clean

## 8. Tags (from handoff)

- [ ] 8.1 Implement tag editing per the handoff (show, remove, add existing via suggestions, add new) and include tags in Save; verify in a harness capture with tags and open suggestions, then in the app that changes are reflected on the Kimai web interface
- [ ] 8.2 Commit (e.g. "Edit timesheet tags"); verify lint clean

## 9. Delete entry (from handoff)

- [ ] 9.1 Implement "Delete entry" and its confirmation per the handoff, wired to `deleteTimeSheet`; verify in the app: declined confirmation keeps the dialog and its edits, confirmed deletion removes the entry, deleting the running entry stops the timer
- [ ] 9.2 Commit (e.g. "Delete timesheet from edit dialog"); verify lint clean

## 10. Logout entry point (from handoff)

- [ ] 10.1 Receive the Claude Design handoff for the settings entry point; verify it against specs/session-logout, raising any conflict before coding
- [ ] 10.2 Implement the entry point per the handoff and wire it to `logout`; verify in the app: logout shows the login view with profiles intact, and a restart does not auto-login
- [ ] 10.3 Commit (e.g. "Add logout menu"); verify lint clean

## 11. Integration checks

- [ ] 11.1 In the app, cut the network then click ↻: verify the "Could not refresh projects and activities" warning banner appears and the activity list still works
- [ ] 11.2 In the app, type in the activity field, click elsewhere with the pointer over the list, then pick an entry: verify the list stays open and the pick is applied
- [ ] 11.3 After pushing, verify the first CI run passes on the three platforms with Qt 6.11.3 (if `install-qt-action` cannot install it, bump the action or its `aqtversion`) and that the QML lint step runs
- [ ] 11.4 Full pass: edit, tag, delete, logout/login on the test instance; verify `ctest` and lint are green on the final tree

## Workflow follow-up

- Alexandre designs each brief in Claude Design and hands back the handoffs (blocks groups 7–10 only).
- Add the repository secrets for a Kimai instance reachable from GitHub runners (owner decision).
- Archive the change once the alpha is accepted.
