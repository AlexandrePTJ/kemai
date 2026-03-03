import QtQuick
import QtQuick.Controls

TextField {
    
    color: Theme.colorTextPrimary
    
    font.pixelSize: Theme.fontSizeNormal
    placeholderTextColor: Theme.colorTextSecondary
    
    leftPadding: 10
    rightPadding: 10

    background: Rectangle {
        color: Theme.colorSurface
        radius: Theme.radiusMedium

        implicitHeight: 40
    }
}
