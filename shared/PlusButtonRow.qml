
// License BSD-3-Clause

import QtQuick
import QtQuick.Controls

Row {
    id: rowItem;
    x: 180;
    property var parentRef;
    property bool trashcanVisible;
    property int lvRowId;
    property string _3dViewText;
    property string siteNam;

    TextField {
        id: tfViewField;
        width: 180;
        text: _3dViewText;
        placeholderText: "Введите название 3D-вида";

        onTextChanged: {
            rowItem.parentRef.set3DViewName(parent.objectName, text);
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

    TextField {
        id: tfSiteField;
        width: 220;
        text: siteNam;
        placeholderText: "Введите наименование площадки";

        onTextChanged: {
            rowItem.parentRef.setSiteName(parent.objectName, text);
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
            if(tfViewField.text == "" && tfSiteField.text == ""){}
            else
                rowItem.parentRef.addSubRowWrapper(parent.lvRowId, "", "");
        }
    }

    Image
    {
        id: btnTrashCanImage;
        source: "resources/trash.png";
        visible: parent.trashcanVisible;
        //x: 765;
        width: 18;
        height: 18;
        MouseArea
        {
            anchors.fill: parent;
            onClicked:
            {
                rowItem.parentRef.removeSubRow(parent.parent.objectName);
                parent.parent.destroy();
            }
        }
    }
}
