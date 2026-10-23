# Spec Delta

## Purpose

Lets the user correct or complete an existing time entry — what was worked on, its description, its times and its tags — directly from the history, including the entry currently running.

## ADDED Requirements

### Requirement: Edit dialog opens from a history entry
Each entry of the history SHALL offer an Edit action that opens a single edit dialog prefilled with the entry's project/activity, description, start, end and tags.

#### Scenario: Open a finished entry
- **WHEN** the user clicks Edit on a finished entry
- **THEN** the edit dialog opens showing that entry's project — activity, description, start time, end time, duration and tags

#### Scenario: Open the running entry
- **WHEN** the user clicks Edit on the running entry
- **THEN** the edit dialog opens with the end shown as "now" and the end not editable

### Requirement: Project and activity can be changed
The edit dialog SHALL let the user pick another project — activity pair among those available in the cache, with the same search behaviour as the timer bar.

#### Scenario: Change the activity
- **WHEN** the user picks another project — activity and saves
- **THEN** the entry is updated on the server with the new project and activity, and the history shows them

### Requirement: Description can be changed
The edit dialog SHALL let the user set, change or clear the entry's description, including on the running entry.

#### Scenario: Describe the running entry
- **WHEN** the user types a description for the running entry and saves
- **THEN** the running entry keeps running and the history shows the new description

#### Scenario: Clear a description
- **WHEN** the user empties the description and saves
- **THEN** the entry is saved without description

### Requirement: Start and end are edited with the date-time picker
The start and end of the entry SHALL each be editable through the existing date-time picker, and the dialog SHALL show the resulting duration before saving.

#### Scenario: Change the end time
- **WHEN** the user opens the end picker, picks 17:00 and confirms
- **THEN** the dialog shows the new end and the recomputed duration, without saving yet

### Requirement: Invalid time ranges are rejected
The system SHALL NOT save an entry whose end is not strictly after its start.

#### Scenario: End before start
- **WHEN** the edited end is equal to or before the edited start
- **THEN** saving is not possible and the entry on the server is unchanged

### Requirement: Tags can be added and removed
The edit dialog SHALL show the entry's tags as removable items and let the user add tags, suggesting tags that already exist on the server.

#### Scenario: Add an existing tag
- **WHEN** the user types the beginning of a tag that exists on the server
- **THEN** that tag is suggested, and once chosen it appears in the entry's tags

#### Scenario: Add a new tag
- **WHEN** the user types a tag name that does not exist yet and validates it
- **THEN** the tag is added to the entry and created on the server when the entry is saved

#### Scenario: Remove a tag
- **WHEN** the user removes a tag and saves
- **THEN** the entry no longer has that tag

### Requirement: Changes are applied only on save
Edits SHALL be applied to the server only when the user saves; cancelling or closing the dialog SHALL discard them.

#### Scenario: Cancel
- **WHEN** the user changes fields then clicks Cancel
- **THEN** the entry on the server and in the history is unchanged

### Requirement: Save failures are reported
When saving fails, the system SHALL keep the entry unchanged and report the failure through the application's notification banner, or return to login if the session has expired.

#### Scenario: Network failure on save
- **WHEN** the save request fails for lack of network
- **THEN** a "Network unavailable" notification is shown and the history still shows the previous values

#### Scenario: Session expired on save
- **WHEN** the server answers that the session is no longer authorized
- **THEN** the user is brought back to the login view
