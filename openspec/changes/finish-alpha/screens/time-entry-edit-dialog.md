# Screen brief — Edit Time Entry

Functional brief for Claude Design. It says **what** the screen must let the user do and **which situations** it must handle; layout, components, wording style and visuals are open. Source requirements: `specs/timesheet-editing/spec.md` and `specs/timesheet-deletion/spec.md` (requirement names quoted in brackets below).

## Purpose

Correct or complete one existing time entry — what was worked on, its description, its times, its tags — or delete it, without leaving Kemai for the Kimai web interface.

## Context of use

- Kemai is a desktop app (Linux, Windows, macOS). The main window defaults to 800 × 600 and holds: a timer bar (running timer, project — activity search, Start/Stop), a "timer running on" hint, then the list of recent time entries.
- Each entry of that list shows: color of its activity, project, activity, description, duration, time range ("14:25 – 17:00", "Yesterday 16:00 – 18:10", "14:25 – now"), and actions: Edit, and Start (re-start the same project/activity) on finished entries.
- The app already has a single-date-time picker (calendar + minute list + :00/:15/:30/:45/Now shortcuts + 24h/12h) used to edit one date-time. This screen reuses it for start and end; it is not redesigned here.
- Prior art: the "Edit Time Entry" dialog in `kemai-main.html` (mockup). It is an inspiration, not a constraint.

## Entry points

- **Edit** on a finished entry → opens the screen for that entry. [Edit dialog opens from a history entry]
- **Edit** on the running entry → opens the screen for it; the entry keeps running while the screen is open. [Open the running entry]
- The screen is modal: one entry at a time.

## Data shown and editable

| Field | Shown | Editable | Notes |
|---|---|---|---|
| Project — activity | yes | yes | Chosen among the pairs known to the app (each activity belongs to a project; "global" activities are offered under every project). Must support searching: instances can have hundreds of pairs. An invalid pair must not be selectable. [Project and activity can be changed] |
| Activity color | yes | no | Helps recognise the entry. |
| Description | yes | yes | Free text, may be empty, may be set/changed/cleared on the running entry too. Usually one line; can be longer. [Description can be changed] |
| Start | yes (date + time) | yes, through the picker | [Start and end are edited with the date-time picker] |
| End | yes (date + time), or "now" for the running entry | yes through the picker; **not** on the running entry | [Open the running entry] |
| Duration | yes | no — computed from start/end | Updates as soon as start or end change, before saving. Running entry: duration up to now (may tick). |
| Tags | yes, as a set of individual tags | add / remove | Suggestions from tags existing on the server while typing; a name that does not exist yet can be added and is created on save. [Tags can be added and removed] |

Start and end may be on different days (night work, corrections of a past day): the date must be visible, not only the time.

## Actions

- **Save** — sends all changes at once, then the screen closes and the list shows the new values. [Changes are applied only on save]
- **Cancel / close** — discards every change; nothing is sent. [Cancel]
- **Pick start / pick end** — opens the date-time picker on the current value; confirming returns the new value to this screen without saving anything yet.
- **Delete entry** — destructive; requires an explicit confirmation before anything is sent. Declining the confirmation returns to the screen with all current edits intact. On confirmed success the screen closes and the entry disappears from the list; if it was the running entry, no timer runs anymore. [Entry can be deleted from the edit dialog] [Deleting the running entry stops tracking]
- Re-starting an entry is **not** an action of this screen (it stays on the list).

## States to design

1. **Unchanged** — just opened, values from the entry. Saving with no change may be allowed or not (designer's choice); it must not do any harm.
2. **Edited** — at least one field differs from the original.
3. **Invalid time range** — end equal to or before start: saving is impossible, and the user must understand why. [Invalid time ranges are rejected]
4. **Running entry** — end is "now" and not editable; everything else editable; delete allowed.
5. **Saving / deleting in progress** — the request takes from a few ms to several seconds; prevent double submission.
6. **Save or delete failed** — the entry stays unchanged on the server and in the list; the user's edits must not be lost (they can retry or cancel). Failure reasons the app distinguishes: no network, permission refused, not found (deleted meanwhile), server error, unexpected response. The app reports errors in a notification banner at the top of the main window; the design must make sure the user **sees** the failure while this modal screen is open (banner visible above it, or a message inside the screen — designer's choice). [Save failures are reported] [Deletion failures are reported]
7. **Session expired** during save/delete — the screen closes and the app returns to the login view (handled by the app; nothing to design beyond not blocking it).
8. **Delete confirmation pending** — distinct from the normal state; cancel and confirm both reachable.
9. **Tag suggestions** — none yet / loading / list of matches / no match (then "add as new tag").

## Validation rules

- End strictly after start (running entry: no end).
- Project — activity must be a valid pair (guaranteed by choosing from the list).
- Tag names: non-empty after trimming; **a tag cannot contain a comma** (the app sends tags as a comma-separated list); duplicates are ignored.
- Description: no rule.

## Keyboard and accessibility expectations

- Escape cancels (or closes the picker / the confirmation if one is open).
- Enter in the tag field adds the typed tag; elsewhere it should not delete anything by accident.
- Every action reachable without a mouse is a plus, not a requirement for the alpha.

## Out of scope

- Creating customers, projects or activities.
- Editing several entries at once; splitting or merging entries.
- Changing the user an entry belongs to, billable flags, rates, custom fields.
- Redesigning the date-time picker itself.
- Undo after delete.

## Open points for Alexandre

- **Punch mode**: when a Kimai instance is configured in "punch" tracking mode, the app does not send start/end (Kimai manages them). Should start/end be shown read-only in that mode (with an explanation), or hidden? The current specs assume times are always editable; this needs a decision and a spec update.
- **Project — activity**: one searchable field (like the timer bar) or two dependent fields (as in the mockup)? Functionally both work as long as only valid pairs can be chosen.
- Where errors appear while the screen is open (state 6).
