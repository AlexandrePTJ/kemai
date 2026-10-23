# Spec Delta

## Purpose

Lets the user remove a time entry that should not exist (started by mistake, duplicate) without going through the Kimai web interface.

## ADDED Requirements

### Requirement: Entry can be deleted from the edit dialog
The edit dialog SHALL offer a "Delete entry" action that removes the entry from the server, after an explicit confirmation in the dialog.

#### Scenario: Confirmed deletion
- **WHEN** the user clicks "Delete entry" then confirms
- **THEN** the entry is deleted on the server, the dialog closes and the entry disappears from the history

#### Scenario: Deletion not confirmed
- **WHEN** the user clicks "Delete entry" then cancels the confirmation
- **THEN** nothing is deleted and the dialog stays open with its current edits

### Requirement: Deleting the running entry stops tracking
Deleting the running entry SHALL leave no timer running in the client.

#### Scenario: Delete the running entry
- **WHEN** the user deletes the running entry
- **THEN** the timer bar shows no running timer and Start is available again

### Requirement: Deletion failures are reported
When deletion fails, the entry SHALL stay in the history and the failure SHALL be reported through the notification banner, or the user returned to login if the session has expired.

#### Scenario: Server refuses the deletion
- **WHEN** the server refuses the deletion (for example, missing permission)
- **THEN** a permission notification is shown and the entry is still listed
