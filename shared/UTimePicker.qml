import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

Item
{
    id: scope;
    clip: true;

    QtObject
    {
        id:variables
        property var time: ( {hour: 0, minute: 0} )
        onTimeChanged:
        {
            refreshDialogTime()
        }
    }

    signal changed
    property alias caption : captionTxt.text
    property size size : Qt.size(30,70)
    property string splitter : ":"
    property alias spacing : row.spacing;

    Component.onCompleted:
    {
        var q = new Date()
        var curtime = q.toLocaleTimeString().substring(0,5);
        if(splitter != ":")
        {
            curtime.replace(':',splitter)
        }
        var vars = curtime.split(':')
        setTime(vars[0],vars[1])
        refreshDialogTime()
    }

    function refreshDialogTime()
    {
        dialog.hour = variables.time.hour
        dialog.minute = variables.time.minute
    }

    function getTime()
    {
        return variables.time;
    }

    function setTimeString(time)
    {
        textArea.text= time
    }

    function setTime(hour,minute)
    {
        var _hour = hour;
        if(_hour < 10)
        {
            _hour = "0" + hour.toString();
        }
        else{
            _hour = hour.toString();
        }
        var _minute = minute
        if(_minute < 10)
        {
            _minute = "0" + minute.toString();
        }
        else{
            _minute = minute.toString();
        }

        var time = _hour + ":" + _minute;
        textArea.text = time;
    }

    implicitHeight: 50;
    implicitWidth: 200;
    Row
    {
        id: row
        width: parent.width
        height: parent.height
        spacing: 25
        layoutDirection: Qt.RightToLeft

        Text
        {
            font.bold: true
            id: captionTxt
            font.pointSize: 12
            horizontalAlignment: Text.AlignRight
            anchors.verticalCenter: parent.verticalCenter
            width: scope.size.width * scope.width /100 - scope.spacing/2
            verticalAlignment: Text.AlignVCenter
            font.family: "B Nazanin"

        }
        Item
        {
            id: element;
            anchors.verticalCenter: parent.verticalCenter;
            height: parent.height;
            width: scope.size.height * scope.width/100 - scope.spacing/2;

            Rectangle
            {
                id: backrec;
                height: parent.height;
                anchors.verticalCenter: parent.verticalCenter;
                width: parent.width

                Image
                {
                    id: iconBtn;
                    source: "images/clock.png";
                    anchors.verticalCenter: parent.verticalCenter;
                    height: 12;
                    width: 12;
                    anchors.right: parent.right;
                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:
                        {
                            textArea.focus = true
                            dialog.open()
                        }
                    }
                }

                TextField
                {
                    id: textArea;
                    placeholderText : "HH:mm"
                    font.family: "B Nazanin"

/****************THE FONT SIZE ****************************************************************************/
                    font.pointSize: 8;

                    selectByMouse: true;
                    anchors.verticalCenter: parent.verticalCenter;

                    //anchors.left: parent.left;
                    //anchors.right: iconBtn.left;

                    height: parent.height;
                    bottomPadding: 5;
                    topPadding: 5;
                    verticalAlignment: Text.AlignVCenter;
                    onFocusChanged:
                    {
                        if(focus)
                        {
                            captionTxt.color = Universal.color( Universal.Cobalt);
                        }
                        else
                        {
                            captionTxt.color = "black";
                        }
                    }

                    background: URect
                    {
                        color: "transparent";
                        border.width: 0;
                    }
                    onTextChanged: {
                        var _temp = text.split(splitter);
                        if(_temp.length>0)
                        {
                            variables.time.hour =_temp[0] == ""?0:  _temp[0];
                            variables.time.minute = _temp[1] == ""?0:_temp[1];
                        }
                        changed()
                    }
                    inputMask:  "99:99"
                    validator: RegularExpressionValidator { regularExpression: /^([0-1\s]?[0-9\s]|2[0-3\s]):([0-5\s][0-9\s])$ / }
                }
            }
        }
    }

    UTimeDialog
    {
        id: dialog;
        x: 56;
        y: 78;

        onAccepted:
        {
            setTime(hour,minute)
        }
    }
}
