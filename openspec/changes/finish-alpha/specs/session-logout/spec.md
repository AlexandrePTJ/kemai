# Spec Delta

## Purpose

Lets the user end the current Kimai session on purpose, for example to switch account or instance, instead of only through session expiry.

## ADDED Requirements

### Requirement: Logout is available from the main view
The main view SHALL provide a "Log out" action reachable at any time during a session.

#### Scenario: Log out
- **WHEN** the user chooses "Log out"
- **THEN** the session ends and the login view is shown

### Requirement: Logout disables auto-login
Logging out SHALL disable auto-login, so the next start of the application shows the login view instead of reconnecting.

#### Scenario: Restart after logout
- **WHEN** the user logged out, closes the application and starts it again
- **THEN** the login view is shown and no connection is attempted automatically

### Requirement: Logout keeps saved profiles
Logging out SHALL NOT remove saved profiles or their stored tokens.

#### Scenario: Profiles after logout
- **WHEN** the user logs out
- **THEN** the login view still lists the saved profiles and can connect with them
