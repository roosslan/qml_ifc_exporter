import QtQuick 2.0
import QtQuick.Controls 2.4
import QtQuick.Controls.Universal 2.4

Item{
    id:root
    property alias hour : hourSpin.currentIndex
    property alias minute : minuteSpin.currentIndex
    signal open
    signal close
    signal accepted
    signal rejected
    visible: element.opened
    onOpen: element.open()
    onClose: element.close()
    implicitWidth: 200
    implicitHeight: 200
    Dialog {
        id: element
        modal: true
        width: parent.width
        height: parent.height
        padding: 5
        margins: 5
        background:
        Item{

        }

        onAccepted: {
            root.accepted()
        }
        onRejected: {
            root.rejected()
        }
        contentItem: UCard{
            anchors.fill: parent
            radius: 10
        }

        Column{
            id: column
            spacing: 30
            anchors.centerIn: parent
            Row{
                id: row
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter
                Column{
                    id: column1
                    spacing: 15
                    height: 80
                    width: 50
                    clip:true

                    UTumbler
                    {
                        id:hourSpin
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        model: 24
                    }

                }
                Text{
                    text: ":"
                    font.pointSize: 12
                    anchors.verticalCenter: parent.verticalCenter
                    font.family: "B Nazanin"
                }
                Column{
                    id: column2
                    spacing: 15
                    height: 80
                    width: 50
                    clip:true

                    UTumbler{
                        id:minuteSpin
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        model: 60
                    }

                }

            }
            Row{
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 40
                Button
                {
                    text:"ОK"
                    width: 60;
                    onClicked: {
                        element.accept()
                    }
                }
                Button{
                    text: "Отмена"
                    width: 60;
                    onClicked: {
                        element.reject()
                    }
                }
            }
        }

    }
}
