import QtQuick

ListModel {
    id: timeSheetModel

    ListElement {
        activityName: "New UI/UX"
        projectName: "kemai"
        tags: [
            ListElement { name: "development" }
        ]
        startDateTime: "2024-02-01 18:12:00"
        endDateTime:  "2024-02-01 18:42:00"
        duration: "3:00"
    }

    ListElement {
        activityName: "New UI/UX"
        projectName: "kemai"
        tags: [
            ListElement { name: "development" }
        ]
        startDateTime: "2024-02-01 18:12:00"
        endDateTime:  "2024-02-01 18:42:00"
        duration: "6:00"
    }
}
