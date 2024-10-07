import QtQuick
import "shared/"
import QtQuick.Dialogs
import QtQuick.Controls

Item {
    property real defaultSpacing: 10

    signal qmlSignal(msg: string)
    signal selectedFileSignal(fname: string)

    id: mainWindow
    width: 1300
    height: 800

    Rectangle {
        y: 5
        x: 10
        radius: 20
        height: 700
        anchors.fill: parent
        anchors.margins: defaultSpacing

        CustomBorderRect
        {
            id: borderRect
            anchors.top: btnAddLocalProject.bottom
            anchors.margins: 10
            anchors.left: parent.left
            width : 800
            height: 650
            color: "white"

            lBorderwidth: 1
            rBorderwidth: 1
            tBorderwidth: 1
            bBorderwidth: 1
            borderColor: "black"
        }



        RoundButton {
            id: btnAddLocalProject
            anchors {
                left: parent.left
                top: parent.top
                margins: defaultSpacing
            }
            text: "Добавить локальный проект"
            height: 22
            width: 800
            onClicked: fileDialog.open();
        }

        ListView {
            id: lvMain
            objectName: "objLVItem"
            anchors.top: borderRect.top
            anchors.margins: 10
            anchors.left: parent.left
            height: 600
            width: 600

            delegate: Column {
                id: horizCol
                Text
                {
                    x: 5
                    id: rowText
                    text: path }
                Button
                {
                    x: 750
                    y: y - 15
                    width: 15
                    height: 15
                    text: "X"
                    onClicked: listModel.remove(index)
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


        RoundButton {
            id: btnSaveTrueToConfig
//            x: 0;
//            y: 54
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: defaultSpacing
            text: "►"
            width: 45
            height: 45
            onClicked: qmlSignal(mainWindow)
        }


        RoundButton {
            id: btnStop;
            anchors.right: btnSaveTrueToConfig.left;
            anchors.bottom: parent.bottom;
            anchors.margins: defaultSpacing;
            text: "■";
            width: 45;
            height: 45;
            onClicked: qmlSignal(mainWindow);
        }

        Label {
            id: labelExportSettings;
            anchors.left: borderRect.left;
            anchors.top: borderRect.bottom;
            text: "Настройки экспорта";
        }

        Path
        {
            startX: 60;
            startY: 300;
            PathLine
            {
                x: 200;
                y: 300
            }
        }

        ComboBox {
            id: cbVersion
            editable: false;
            anchors.left: labelExportSettings.left;
            anchors.top: labelExportSettings.bottom;
            model: ListModel
            {
                id: revitVersion;
                ListElement { text: "2022" }
                ListElement { text: "2023" }
            }
            // onAccepted: {      model.append({text: editText})        }
        }

        Label
        {
            id: labelIFC
            anchors.left: borderRect.left;
            anchors.top: cbVersion.bottom;
            text: "Industry Foundation Classes";
        }

        FileDialog{
                id: fileDialog;
                title: "Please choose a file";
                nameFilters: ["Revit Files (*.rvt *.png *.gif)"];
                onAccepted: {
                    selectedFileSignal(fileDialog.selectedFile.toString());
                    listModel.append(btnAddLocalProject);
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
        property int timeoutInterval: 2000;
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
