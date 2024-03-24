import QtQuick

ListModel {
    id: timeSheetModel

    ListElement {
        activityName: "New UI/UX"
        projectName: "kemai"
        tags: [
            ListElement { name: "development" }
        ]
        startDateTime: "2024/02/01T18:12:00"
        endDateTime:  "2024/02/01T18:42:00"
    }
}
