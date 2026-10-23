# Spec Delta

## Purpose

Defines how the recent time entries are listed and labelled, so the user can tell at a glance when each entry happened.

## ADDED Requirements

### Requirement: Time range shows the day when it is not today
Each history entry SHALL show its time range in local time, prefixed by "Yesterday" or by the date when the entry did not start today.

#### Scenario: Entry from today
- **WHEN** an entry ran today from 14:25 to 17:00
- **THEN** its range reads "14:25 – 17:00"

#### Scenario: Entry from yesterday
- **WHEN** an entry ran yesterday from 16:00 to 18:10
- **THEN** its range reads "Yesterday 16:00 – 18:10"

#### Scenario: Older entry
- **WHEN** an entry ran on Monday September 7 from 08:30 to 11:50
- **THEN** its range reads "Mon, Sep 7 08:30 – 11:50" (date in the same format as the history header)

### Requirement: Running entry reads "now"
The time range of the running entry SHALL end with "now".

#### Scenario: Running entry
- **WHEN** an entry started today at 14:25 and is still running
- **THEN** its range reads "14:25 – now"

### Requirement: Entries crossing midnight name their end day
When an entry ends on another day than it started, its range SHALL name the end day explicitly.

#### Scenario: Night entry
- **WHEN** an entry ran from yesterday 23:00 to today 01:30
- **THEN** its range reads "Yesterday 23:00 – Today 01:30"

### Requirement: Labels follow the date change
At midnight, the day labels of all listed entries SHALL update without requiring a reload.

#### Scenario: Midnight passes
- **WHEN** the application stays open across midnight
- **THEN** within a minute, entries from the previous day read "Yesterday …"
