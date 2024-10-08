import QtQuick
import "shared/"
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Item {
    property real defaultSpacing: 10
    property real topOffset: 10
    signal qmlSignal(msg: string)
    signal selectedFileSignal(fname: string)

    id: mainWindow;
    width: 1300;
    height: 870;

    Rectangle {

        y: 5
        x: 10

        height: 750;
        anchors.fill: parent
        anchors.margins: defaultSpacing

        RoundButton {
            id: btnAddLocalProject
            anchors {
                left: parent.left
                top: parent.top
                margins: defaultSpacing
            }
            text: "Добавить локальный проект"
            height: 22;
            width: 800;
            onClicked: fileDialog.open();
        }

        Row
        {
            id: theTextBoxRow;
            anchors.top: btnAddLocalProject.bottom;
            anchors.left: btnAddLocalProject.left;
            anchors.topMargin: 10;
            spacing: 25;
            TextField
            {
                id: rsnEditBox;

                width: btnAddLocalProject.width-50;
                text: "RSN://ALD-SRV-B01/Projects/";
            }
        }
        Image
        {
            source: "resources/plus.png";
            anchors.right: btnAddLocalProject.right;
            anchors.top: theTextBoxRow.top;
            width: 15;
            height: 15;
            MouseArea
            {
                anchors.fill: parent
                //onClicked: listModel.remove(index)
            }
        }

        CustomBorderRect
        {
            id: borderRect;
            anchors.top: theTextBoxRow.bottom;
            anchors.margins: 10;
            anchors.left: parent.left;
            width : 800;
            height: 550;
            color: "white";

            lBorderwidth: 1
            rBorderwidth: 1
            tBorderwidth: 1
            bBorderwidth: 1
            borderColor: "black"
        }

        CustomBorderRect
        {
            id: logFrame
            y: 10;
            anchors.margins: 10
            anchors.left: borderRect.right
            width : 450
            height: 840
            color: "white"

            lBorderwidth: 1
            rBorderwidth: 1
            tBorderwidth: 1
            bBorderwidth: 1
            borderColor: "black"
        }

        Label {
            id: labelLog;
            x: 1000;
            anchors.top: btnAddLocalProject.top;
            Text
            {
                text: "Журнал";
                font.pixelSize: 18;
            }
        }

        ListView {
            id: lvMain
            objectName: "objLVItem"
            anchors.top: borderRect.top
            anchors.margins: 10
            anchors.left: parent.left
            height: 500
            width: 600

            delegate:
            Column
            {
                id: horizCol                
                Text
                {
                    id: rowText
                    x: 10;
                    text: path
                }
                Image
                {
                    source: "resources/trash.png";
                    x: 765;
                    //y: y - 15
                    width: 15;
                    height: 15;
                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked: listModel.remove(index)
                    }
                }
            }

                model: ListModel { id: listModel
                    ListElement
                    {
                        path: "C:\\Project\\Autodesk"
                    }
                    ListElement
                    {
                        path: "C:\\Projects\\Revit"
                    }
                    ListElement
                    {
                        path: "RSN:"
                    }
                }
        }

        ListView {
            id: lvLog
            anchors.top: logFrame.top
            anchors.margins: 10
            anchors.left: logFrame.left
            height: 750
            width: 600

            delegate:
                Text
                {
                    x: 5
                    id: rowTxt
                    text: msg
                }


                model: ListModel
                {
                    id: lmModel
                    ListElement
                    {
                        msg: "Подготовка к выгрузке"
                    }
                    ListElement
                    {
                        msg: "Ожидаем..."
                    }
                }
        }

        RoundButton
        {
            id: btnSaveTrueToConfig
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: defaultSpacing
            text: "►"
            width: 45
            height: 45
            onClicked: qmlSignal(mainWindow)
        }


        RoundButton
        {
            id: btnStop;
            anchors.right: btnSaveTrueToConfig.left;
            anchors.bottom: parent.bottom;
            anchors.margins: defaultSpacing;
            text: "■";
            width: 45;
            height: 45;
            onClicked: qmlSignal(mainWindow);
        }
/************************************* Настройки экспорта ************************************/

        Label {
            id: labelExportSettings;
            x: 35
            anchors.top: borderRect.bottom;
            text: "Настройки экспорта";
        }

        ColumnLayout {
            anchors.top: labelExportSettings.verticalCenter;
            x: 10;
            Rectangle {
                id: lineSettings;
                width: 20;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }

        ColumnLayout {
           anchors.top: labelExportSettings.verticalCenter;
            x: 155;
            Rectangle {
                width: 655;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }
/************************************* Настройки экспорта ************************************/


/************************************* Версия Revit ******************************************/
        Row
        {
            id: horizRow;
            anchors.top: labelExportSettings.bottom;
            anchors.left: borderRect.left;
            anchors.topMargin: 5;
            Label
            {
                id: labelRevitVersion;
                anchors.top: labelExportSettings.bottom;
                text: "Версия Revit:";
            }

            ComboBox {
                id: cbVersion;
                editable: false;
                anchors.left: labelRevitVersion.right;
                anchors.leftMargin: 10;
                anchors.top: labelExportSettings.bottom;
                model: ListModel
                {
                    id: revitVersion;
                    ListElement { text: "2022" }
                    ListElement { text: "2023" }
                }
                // onAccepted: {      model.append({text: editText})        }
            }
        }
/************************************* Версия Revit ******************************************/

/************************************* Время выгрузки ****************************************/
        Row
        {
            id: horizonRow;
//            x: 5;
            anchors.top: horizRow.bottom;
            anchors.topMargin: 25;
            anchors.left: horizRow.left;
            Label
            {
                id: labelTime;
                text: "Время выгрузки";
            }
        }

        UTimePicker{
            anchors.top: horizRow.top;
//            anchors.topMargin: 25;
            x: 400;
            width: 200
            //spacing: 10
            size: Qt.size(20,40)
//            caption: "time"
            onChanged: {
                var i =  getTime()
//                console.log(i.hour)
//                console.log(i.minute)
            }

        }
/************************************* Время выгрузки ****************************************/

/******************************* Industry Foundation Classes **********************************/


        CheckBox
        {
            id: cbIFC;
            anchors.top: horizonRow.bottom;
            anchors.topMargin: 5;
            x: 35
            checked: true;
            text: "Industry Foundation Classes (IFC)";
        }

        ColumnLayout
        {
            anchors.top: cbIFC.verticalCenter;
            x: 10;
            Rectangle
            {
                id: lineIFC;
                width: 20;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }

        ColumnLayout
        {
            id: line_cb_IFC_col;
            anchors.top: cbIFC.verticalCenter;
            x: 250;
            Rectangle
            {
                id: line_cb_IFC
                width: 555;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }

        ColumnLayout
        {
            id: horizIFC_Col_text;
            x: 30
            anchors.topMargin: 10;
            anchors.top: cbIFC.bottom;
            Label
            {
                id: labelIFCPath;
                text: "C:\\MyDocs";
            }
        }

        ColumnLayout
        {
            id: btnBrowseFolderCol;
            anchors.top: cbIFC.bottom;
            anchors.right: borderRect.right;
            RoundButton
            {
                id: btnBrowseFolder;
                text: "...";
                width: 15
                height: 15
                onClicked: qmlSignal(mainWindow)
            }
        }
/******************************* Industry Foundation Classes **********************************/

/************************************************ Версия IFC **********************************/

        Row
        {
            id: hRow;
            anchors.top: btnBrowseFolderCol.bottom;
            anchors.left: borderRect.left;
            anchors.topMargin: 5;
            Label
            {
                id: labelIFCVersion;
                anchors.top: horizIFC_Col_text.bottom;
                text: "Версия IFC:";
            }

            ComboBox {
                id: cbIFCVersion;
                editable: false;
                anchors.left: labelIFCVersion.right;
                anchors.leftMargin: 10;
                anchors.top: labelIFCVersion.top;
                model: ListModel
                {
                    id: revitIFCVersion;
                    ListElement { text: "Default" }
                    ListElement { text: "IFCBCA" }
                    ListElement { text: "IFC2x2" }
                    ListElement { text: "IFC2x3" }
                    ListElement { text: "IFCCOBIE" }
                    ListElement { text: "IFC2x3CV2" }
                    ListElement { text: "IFC2x3FM" }
                    ListElement { text: "IFC2x3BFM" }
                    ListElement { text: "IFC4" }
                    ListElement { text: "IFC4DTV" }
                    ListElement { text: "IFC4RV" }

                }
                // onAccepted: {      model.append({text: editText})        }
            }
        }

/************************************************ Версия IFC **********************************/


/******************************* Navisworks **********************************/

                CheckBox
                {
                    id: cbNavi;
                    anchors.top: hRow.bottom;
                    anchors.topMargin: 20;
                    x: 35
                    checked: true;
                    text: "Navisworks";
                }

                ColumnLayout
                {
                    anchors.top: cbNavi.verticalCenter;
                    x: 10;
                    Rectangle
                    {
                        id: lineNavi;
                        width: 20;
                        Layout.fillWidth: true;
                        Layout.preferredHeight: 1;
                        color: "black";
                    }
                }

                ColumnLayout
                {
                    id: line_cb_Navi_col;
                    anchors.top: cbNavi.verticalCenter;
                    x: 180;
                    Rectangle
                    {
                        id: line_cb_Navi
                        width: 600;
                        Layout.fillWidth: true;
                        Layout.preferredHeight: 1;
                        color: "black";
                    }
                }

                ColumnLayout
                {
                    id: horizNavi_Col_text;
                    x: 30
                    anchors.topMargin: 10;
                    anchors.top: cbNavi.bottom;
                    Label
                    {
                        id: labelNaviPath;
                        text: "C:\\MyDocs";
                    }
                }

                ColumnLayout
                {
                    id: btnBrowseFolderNaviCol;
                    anchors.top: cbNavi.bottom;
                    anchors.right: borderRect.right;
                    RoundButton
                    {
                        id: btnBrowseFolderNavi;
                        text: "...";
                        width: 15
                        height: 15
                        onClicked: qmlSignal(mainWindow)
                    }
                }
/******************************* Navisworks **********************************/

        FileDialog{
                id: fileDialog;
                title: "Please choose a file";
                nameFilters: ["Revit Files (*.rvt *.ifc)"];
                onAccepted: {
                    selectedFileSignal(fileDialog.selectedFile.toString());

                    listModel.append({"path": fileDialog.selectedFile.toString() })
                    fileDialog.selectedFile = "";
                    fileDialog.close()
                }
            }
    }


    property var splashWindow: Window {
        id: splash;
        color: "transparent";
        title: "Splash Window";
        modality: Qt.ApplicationModal;
        flags: Qt.SplashScreen;
        property int timeoutInterval: 300;
        signal timeout;
    //! [splash-properties]
    //! [screen-properties]
        x: (Screen.width - splashImage.width) / 2;
        y: (Screen.height - splashImage.height) / 2;
    //! [screen-properties]
        width: splashImage.width;
        height: splashImage.height;

        Image {
            id: splashImage
            source: Images.qtLogo
            TapHandler {
                onTapped: splash.exit()
            }
        }

        function exit() {            
            mainWindow.visible = true
            splash.visible = false
            splash.timeout()
        }

        //! [timer]
        Timer {
            interval: splash.timeoutInterval; running: splash.visible; repeat: false
            onTriggered: splash.exit()
        }
        //! [timer]
        visible: true
    }

}
