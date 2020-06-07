import QtQml 2.1
import QtQml.Models 2.1
import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.2
import QtGraphicalEffects 1.0

// Custom qml modules are in /theme (and included by resources.qrc)
import Style 1.0

Button {
    id: root

    Layout.alignment: Qt.AlignRight
    display: AbstractButton.IconOnly
    Layout.preferredWidth:  Style.trayWindowHeaderHeight
    Layout.preferredHeight: Style.trayWindowHeaderHeight
    flat: true

    icon.width: Style.headerButtonIconSize
    icon.height: Style.headerButtonIconSize
    icon.color: "transparent"

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: Style.hoverEffectsEnabled
        onClicked: root.clicked()
    }

    background:
        Rectangle {
        color: mouseArea.containsMouse ? "white" : "transparent"
        opacity: 0.2
    }
}
