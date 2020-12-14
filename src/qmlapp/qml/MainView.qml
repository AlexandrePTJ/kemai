import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Item {
    ColumnLayout {
        anchors.fill: parent

        ComboBox {
            model: customerDataModel
            valueRole: "kimaiId"
            textRole: "name"
        }
    }
}
