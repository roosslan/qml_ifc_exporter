/* License BSD-3-Clause */

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

Row {
    id: rowItem;
    x: 18;
    property var parentRef;
    property bool trashcanVisible;
    property int lvRowId;
    property string plusBtnProp_3dViewText;
    property string plusBtnProp_siteNam;
    property string plusBtnProp_outputFileNam;
    property string plusBtnProp_jsonFilePath;

    TextField {
        id: tfViewField;
        width: 165;
        text: plusBtnProp_3dViewText;
        placeholderText: "Введите название 3D-вида";

        onTextChanged:{
            rowItem.parentRef.set_input_field_text("arrf_3dview_name", parent.objectName, text);
        }

        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            /* Remove specific borders by drawing only bottom line
             * Top, left, and right are transparent or width 0      */
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }
    }

    /* Разделитель между полями ввода */
    Rectangle {
        color: "transparent";
        height: 1;
        width: 10;
    }

    TextField {
        id: tfSiteField;
        width: 175;
        text: plusBtnProp_siteNam;
        placeholderText: "Введите название площадки";

        onTextChanged: {
            rowItem.parentRef.set_input_field_text("arrf_site_name", parent.objectName, text);
        }        

        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            /* Remove specific borders by drawing only bottom line
             * Top, left, and right are transparent or width 0      */
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }

        ContextMenu.menu: Menu {
                MenuItem {
                    text: qsTr("Очистить")
                    onTriggered: {
                        tfSiteField.text = "";
                        tfSiteField.textChanged();
                    }
                }
        }
    }

    /* Разделитель между полями ввода */
    Rectangle {
        color: "transparent";
        height: 1;
        width: 10;
    }

    TextField {
        id: tfOutputFileName;
        width: 145;
        text: plusBtnProp_outputFileNam;
        placeholderText: "Имя выходного файла";

        onTextChanged: {
            rowItem.parentRef.set_input_field_text("arrf_output_file_name", parent.objectName, text);
        }

        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            /* Remove specific borders by drawing only bottom line
             * Top, left, and right are transparent or width 0      */
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }
    }

    /* Разделитель между полями ввода */
    Rectangle {
        color: "transparent";
        height: 1;
        width: 10;
    }

    TextField {
        id: tfJsonFilePath;
        width: 155;
        text: plusBtnProp_jsonFilePath;
        placeholderText: "Путь к предустанов.json";

        onTextChanged: {
            rowItem.parentRef.set_input_field_text("arrf_json_path", parent.objectName, text);
        }

        background: Rectangle {
            color: "transparent";
            border.width: 0;
            border.color: "gray"

            /* Remove specific borders by drawing only bottom line
             * Top, left, and right are transparent or width 0      */
            Rectangle {
                anchors.bottom: parent.bottom;
                height: 1;
                width: parent.width;
                color: "gray";
            }
        }
    }

    RoundButton {
        text: "…"
        height: 18;
        width: 18;
         onClicked: fileSelectDialog.open();
    }
    Rectangle {
        color: "transparent";
        height: 1;
        width: 10;
    }
    RoundButton {
        text: "+"
        height: 18;
        width: 18;
        onClicked:
        {
            if(tfViewField.text === "" && tfSiteField.text === "" && tfOutputFileName.text === "" && tfJsonFilePath.text === ""){}
            else
                rowItem.parentRef.add_subrow_wrapper(parent.lvRowId, "", "", "", "");
        }
    }

    Image {
        id: btnTrashCanImage;
        source: "resources/trash.png";
        visible: parent.trashcanVisible;
        //x: 765;
        width: 18;
        height: 18;
        MouseArea
        {
            anchors.fill: parent;
            onClicked: {
                rowItem.parentRef.remove_subrow(parent.parent.objectName);
                parent.parent.destroy();
            }
        }
    }

    FileDialog {
        id: fileSelectDialog;
        title: "Please choose a file";
        nameFilters: ["JSON files (*.json)"];
        onAccepted: {
            var path = fileSelectDialog.selectedFile.toString();
            // remove prefixed "file:///"
            path = path.replace(/^(file:\/{3})/,"");
            // unescape html codes like '%23' for '#'
            tfJsonFilePath.text = decodeURIComponent(path);
            fileSelectDialog.selectedFile = "";
            fileSelectDialog.close();

        }
    }
}
