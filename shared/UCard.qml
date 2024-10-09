import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Controls.Universal 2.4

Item{
    property alias radius : morakhasiRect.radius
    property alias color : morakhasiRect.color
    implicitWidth: 150
    implicitHeight: 150

    Rectangle{
        anchors.rightMargin: 1
        anchors.leftMargin: 1
        anchors.bottomMargin: 1
        anchors.topMargin: 1
        id:morakhasiRect
        anchors.fill: parent
        color: "#f5f5f5"
    }
}
