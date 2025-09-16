import QtQuick 2.0
import QtQuick.Controls.Universal 2.4
import QtQuick.Controls 2.4
Tumbler{
    id:hourSpin
    wrap: false
    delegate: Text{

        font.pointSize: 12
        text: modelData
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        opacity: 1.0 - Math.abs(Tumbler.displacement) / (hourSpin.visibleItemCount / 2)
    }
    Rectangle {
        anchors.horizontalCenter: hourSpin.horizontalCenter
        y: hourSpin.height * 0.4
        width: 40
        height: 1
        color: Universal.color(Universal.Cobalt)
    }

    Rectangle {
        anchors.horizontalCenter: hourSpin.horizontalCenter
        y: hourSpin.height * 0.6
        width: 40
        height: 1
        color: Universal.color(Universal.Cobalt)
    }
}