
// License BSD-3-Clause

import QtQuick
import QtQuick.Controls

Row {
    id: rowItem;
    x: 150;
    property var parentRef;
    property bool subRowVisible;
    property int lvRowId;
    property string componentName;
/*
    Text {
        text: "Введите 3D-вид";
        anchors.verticalCenter: parent.verticalCenter
    }
*/
    TextField {
        id: tfViewField;
        width: 130;
        text: getArr3DViews(index);
        placeholderText: "Введите 3D-вид";

        onTextChanged: {
            setArr3DViews(index, text);
        }


        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            // Remove specific borders by drawing only bottom line
            // Top, left, and right are transparent or width 0
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }
    }


    Rectangle
    {
        color: "transparent";
        height: 1;
        width: 60;
    }
/*
    Text {
        text: "Введите площадку"
        anchors.verticalCenter: parent.verticalCenter
    }
*/
    TextField {
        id: tfSiteField;
        width: 130;
        text: getArrSites(index)
        placeholderText: "Введите площадку";

        onTextChanged: {
            setArrSites(index, text);
        }        

        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            // Remove specific borders by drawing only bottom line
            // Top, left, and right are transparent or width 0
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }
    }

    RoundButton {
        text: "+"
        height: 18;
        width: 18;
        onClicked:
        {
            rowItem.parentRef.addSubRow(parent.lvRowId, true);
        }
    }

    Image
    {
        id: btnTrashCanImage;
        source: "shared/images/trash.png";
        visible: parent.subRowVisible;    /* Первый - 0, поэтому его не видно */
        //x: 765;
        width: 18;
        height: 18;
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                rowItem.parentRef.removeSubRow(parent.parent.componentName);
                parent.parent.destroy();
            }
        }
    }
}
