// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
.pragma library

// Wording of ProfileManager's save failure reasons (profilesFileUnreadable | profilesFileUnwritable | keychainRefused)

// The reason: "Not saved: …"
function reason(key) {
    switch (key) {
    case "keychainRefused":
        return "Not saved: the system keychain refused to store the token."
    case "profilesFileUnreadable":
        return "Not saved: the profiles file could not be read."
    case "profilesFileUnwritable":
        return "Not saved: the profiles file could not be written."
    default:
        return "Not saved."
    }
}

// What it means for the profile `name`, new or edited
function consequence(key, name, isNew) {
    switch (key) {
    case "keychainRefused":
        return (isNew ? "“" + name + "” was not added." : "“" + name + "” keeps its previous token, and your other changes are not saved either.") + " Check that the keychain is unlocked, then try again."
    case "profilesFileUnreadable":
        return "It may be damaged or written by a newer Kemai. It was left as it is, so your other profiles are not lost."
    case "profilesFileUnwritable":
        return "Nothing was changed. Check the free space and permissions of your settings folder, then try again."
    default:
        return ""
    }
}
