import QtQuick
import "shared/"
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Item
{
    property bool itemsEnabled: true;
    property bool fileExists: false;
    property real defaultSpacing: 10;
    property real topOffset: 10;
    property int btnNum: 0;

    // Массивы для хранения индивидуальных значений вьюхи и площадки для каждого файла
    property var arr3DViews: []
    property var arrSites: []
    property var selectedDate: new Date().toLocaleString(Qt.locale(),"dd.MM.yyyy");

    signal signalBtnIFCSettingsClicked();
    signal signalStopClicked();
    signal signalRunClicked(rightNow: int, utime: string, udate: string);
    signal escKeyPressedSignal();

    signal signalIsFileExists(fname: string, rvtVersion: string);

    // Функция для установки значения 3D-вьюхи для файла по индексу
    function setArr3DViews(index, view) {
        while (arr3DViews.length <= index) {
            arr3DViews.push("");
        }
        arr3DViews[index] = view;
    }

    // Функция для установки значения площадки для файла по индексу
    function setArrSites(index, site) {
        while (arrSites.length <= index) {
            arrSites.push("");
        }
        arrSites[index] = site;
    }

    // Функция для получения значения view для файла по индексу
    function getArr3DViews(index) {
        if (index >= 0 && index < arr3DViews.length) {
            return arr3DViews[index];
        }
        return "";
    }

    // Функция для получения значения площадки для файла по индексу
    function getArrSites(index) {
        if (index >= 0 && index < arrSites.length) {
            return arrSites[index];
        }
        return "";
    }

    Connections
    {
        target: datePicker;
        onDatePicked: { console.log(selectedDate); }
    }

    id: mainWindow;
    width: 1300;
    height: 870;

    focus: true;
    Keys.onEscapePressed: escKeyPressedSignal();

    MouseArea
    {
        anchors.fill: parent;
        onClicked:
            function(mouse){
                if (!datePicker.contains(Qt.point(mouse.x, mouse.y)))
                {
                        datePicker.visible = false; /*  clicked outside datePicker;  */
                }
            }
    }

    Rectangle
    {
        y: 5;
        x: 10;

        height: 750;
        anchors.fill: parent;
        anchors.margins: defaultSpacing;

        RoundButton
        {
            id: btnAddLocalProject;
            enabled: itemsEnabled;
            anchors
            {
                left: parent.left;
                top: parent.top;
                margins: defaultSpacing;
            }
            text: "Добавить локальный проект";
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
            enabled: itemsEnabled;
            anchors.right: btnAddLocalProject.right;
            anchors.top: theTextBoxRow.top;
            width: 15;
            height: 15;
            MouseArea
            {
                anchors.fill: parent;
                onClicked:
                {
                   signalIsFileExists(rsnEditBox.text, cbVersion.currentText);
                   if (fileExists)
                   {
                        if ( rsnEditBox.text !== "")
                            listModel.append({ "path": rsnEditBox.text });
                        rsnEditBox.text = "";
                   }
                }
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

            lBorderwidth: 1;
            rBorderwidth: 1;
            tBorderwidth: 1;
            bBorderwidth: 1;
            borderColor: "black";
        }

        CustomBorderRect
        {
            id: logFrame;
            y: 10;
            anchors.margins: 10;
            anchors.left: borderRect.right;
            width : 460;
            height: 840;
            color: "white";

            lBorderwidth: 1;
            rBorderwidth: 1;
            tBorderwidth: 1;
            bBorderwidth: 1;
            borderColor: "black";
        }

        LabelALDE
        {
            id: labelLog;
            x: 1000;
            anchors.top: btnAddLocalProject.top;
            Text
            {
                text: "Журнал";
                font.pixelSize: 18;
            }
        }

        ListView
        {
            id: lvMain;
            enabled: itemsEnabled;
            objectName: "o_lvMain";
            anchors.top: borderRect.top;
            anchors.margins: 10;
            anchors.left: parent.left;
            height: 500;
            width: 600;

            delegate:
            Column
            {
                Row{
                id: horizCol;
                Text
                {
                    id: rowText;
                    text: path;
                }                
                Rectangle
                {
//                    width: lvMain.width - rowText.width + 155;
                    height: 20;
                }


                PlusButton
                {
                    text: "+"
                    height: 15
                    width: 15
                    objectName: btnNum;
                    onClicked: {
                        btnNum++;
                        loader.source = "shared/PlusButton.qml"; }
                }

                Loader {
                    id: loader
                }

                CheckBox3DViews
                {
                    id: cbExtract3D
                    objectName: "cbExtract3D_" + index;
                    ToolTip.text: "Указать 3D-виды и площадки";
                    ToolTip.visible: hovered;
                    height: 17;
                    width: 17;
                    checked: false;

                    onClicked:{
                        if (cbExtract3D.checked){
                            rowAdditionalFields.visible = true;
                        }
                        else{
                            rowAdditionalFields.visible = false;
                            // Сбрасываем значения
                            setArr3DViews(index, "");
                            setArrSites(index, "");
                            tfViewField.text = "";
                            tfSiteField.text = "";
                        }
                    }
                }
                Image
                {
                    source: "resources/trash.png";
                    //x: 765;
                    width: 18;
                    height: 18;
                    MouseArea
                    {
                        anchors.fill: parent;
                        onClicked: listModel.remove(index);
                    }
                }
}
                Row {
                    id: rowAdditionalFields
                    y: 80;
                    x: 80;
                    spacing: 20;
                    visible: cbExtract3D.checked;

                    Text {
                        text: "Введите 3D-виды"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        id: tfViewField
                        width: 100;
                        text: getArr3DViews(index)
                        onTextChanged: {
                            setArr3DViews(index, text);
                        }
                    }

                    Text {
                        text: "Введите площадки"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    TextField {
                        id: tfSiteField
                        width: 100;
                        text: getArrSites(index)
                        onTextChanged: {
                            setArrSites(index, text);
                        }
                    }
                }

            }
            model: ListModel
            {
                id: listModel;
/*
                ListElement
                {
                    path: "C:\\Projects\\Autodesk\\wall.rvt";
                }
                ListElement
                {
                    path: "\\\\srv-c666-666\\Projects\\Revit\\arm.rvt";
                }
                ListElement
                {
                    path: "RSN://Projects/101/floor.rvt";
                }
*/
                function removeLastRow(remove)
                {
                    fileExists = remove;
                }
            }
        }

        ListView
        {
            id: lvLog;
            anchors.top: logFrame.top;
            anchors.margins: 10;
            anchors.left: logFrame.left;
            height: 750;
            width: 600;
            objectName: "o_lvLog";
            delegate:
                Text
                {
                    x: 5;
                    id: rowTxt;
                    text: msg
                }

            model: ListModel
            {
                id: lmLogModel;
                ListElement
                {
                    msg: "";
                }

                function addRow(caption)
                {
                    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " " +caption});
                }
            }
            onCountChanged: {
                lvLog.currentIndex = lvLog.count - 1;
                lvLog.positionViewAtEnd();
            }
        }

        RoundButton
        {
            id: btnSaveTrueToConfig;
            objectName: btnRun;
            enabled: mainWindow.itemsEnabled;
            anchors.right: parent.right;
            anchors.bottom: parent.bottom;
            anchors.margins: mainWindow.defaultSpacing;
            text: "🚀";
            width: 45;
            height: 45;
            onClicked:
            {
                onClicked: menuLaunch.open();
            }

            Menu
            {
                id: menuLaunch;
                y: btnSaveTrueToConfig.height;
                MenuItem
                {
                    id: menuRunNow;
                    text: "Запустить сейчас";
                    onTriggered:
                    {
                        var selectedTime = new Date().toLocaleString(Qt.locale(),"hh:mm");

                        lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Запуск прямо сейчас (в " +  selectedTime + ")"});
                        signalRunClicked(1, selectedTime, new Date().toLocaleString(Qt.locale(),"dd.MM.yyyy"));   /* "1" - запустить прямо сейчас */
                        itemsEnabled = false;
                        btnStop.enabled = true;
                    }
                }
                MenuItem
                {
                    id: menuRunScheduled;
                    text: "По указанному времени";
                    onTriggered:
                    {
                        var selectedTime = uTime.getTime();

                        lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Назначенное время " +  selectedTime.hour.toString() + ":" + selectedTime.minute.toString()});
                        signalRunClicked(0, selectedTime.hour.toString() + ":" + selectedTime.minute.toString(), selectedDate);
                        itemsEnabled = false;
                        btnStop.enabled = true;
                    }
                }
            }
        }

        RoundButton
        {
            id: btnStop;
            enabled: mainWindow.itemsEnabled;
            anchors.right: btnSaveTrueToConfig.left;
            anchors.bottom: parent.bottom;
            anchors.margins: mainWindow.defaultSpacing;
            text: "⬛";
            width: 45;
            height: 45;
            onClicked:
            {
                signalStopClicked();
                lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Процесс закрыт"});
                itemsEnabled = true;
            }
        }

        TextEdit
        {
            id: fakeCopyClipboardTextEdit
            visible: false
        }
        RoundButton
        {
            id: btnClipboard;
            anchors.left: lvLog.left;
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 10;

            ToolTip.text: "Копировать журнал в буфер и очистить";
            ToolTip.visible: hovered;
            text: "📋";
            width: 45;
            height: 45;
            onClicked:
            {
                fakeCopyClipboardTextEdit.text = "";
                for (var i = 0; i < lmLogModel.count; i++ )
                {
                    lvLog.currentIndex = i;
                    fakeCopyClipboardTextEdit.text += lmLogModel.get(lvLog.currentIndex).msg + "\n";
                }
                fakeCopyClipboardTextEdit.selectAll();
                fakeCopyClipboardTextEdit.copy();
                lmLogModel.clear();
            }
        }

/************************************* Настройки экспорта ************************************/
        LabelALDE
        {
            id: labelExportSettings;
            x: 35
            anchors.top: borderRect.bottom;
            text: "Настройки экспорта";
        }

        ColumnLayout
        {
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

        ColumnLayout
        {
           anchors.top: labelExportSettings.verticalCenter;
            x: 155;

            Rectangle
            {
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
            objectName: "row_RvtVersion";
            anchors.top: labelExportSettings.bottom;
            anchors.left: borderRect.left;
            anchors.topMargin: 5;

            LabelALDE
            {
                id: labelRevitVersion;
                anchors.top: labelExportSettings.bottom;
                text: "Версия Revit:";
            }

            ComboBox
            {
                id: cbVersion;
                editable: false;
                enabled: itemsEnabled;
                anchors.left: labelRevitVersion.right;
                anchors.leftMargin: 10;
                anchors.top: labelExportSettings.bottom;
                currentIndex: 1;
                model: ListModel
                {
                    id: revitVersion;
                    ListElement { text: "2022" }
                    ListElement { text: "2023" }
                }
            }
        }
/************************************* Версия Revit ******************************************/


/************************************* Время/Дата выгрузки ****************************************/
        Row
        {
            id: horizonRow;

            anchors.top: horizRow.bottom;
            anchors.topMargin: 25;
            x: 90;
            LabelALDE
            {
                id: labelTime;
                text: "Время выгрузки";
            }
        }

        UTimePicker{
            id: uTime;
            enabled: mainWindow.itemsEnabled;
            objectName: "uTime";
            anchors.top: horizRow.top;
            anchors.topMargin: 10;
            x: 65;
            width: 200;             // Font Size <-> depends!
            size: Qt.size(0, 40);
            onChanged:
            {
                var i = getTime()
                // console.log(i.hour)
                // console.log(i.minute)
            }
        }

        Row
        {
            id: horizontalDateRow;

            anchors.top: horizRow.bottom;
            anchors.topMargin: 25;
            anchors.leftMargin: 25;
            x: 290;
            LabelALDE
            {
                id: labelDateText;
                text: "Дата выгрузки: ";
            }
            LabelALDE
            {
                id: labelDate;
                text: Qt.formatDateTime(new Date(), "dd.MM.yy" + "   ");
            }

            RoundButton
            {
                id: btnPickDate;
                enabled: itemsEnabled;
                anchors.bottom: parent.bottom;
                anchors.bottomMargin: 0;
                anchors.topMargin: 20;
                width: 14;
                height: 14;
                onClicked:
                {
                    datePicker.visible = true;
                }
            }
        }

        JWDMDatePicker
        {
            id: datePicker;
            visible: false;
            enabled: mainWindow.itemsEnabled;
            anchors.bottom: lvMain.bottom;
            anchors.topMargin: 10;
            x: 365;
            width: 200;             // Font Size <-> depends!
            height: 300;
            onDatePicked: (udate) =>
            {
                var options = { year: 'numeric', month: 'numeric', day: 'numeric' };
                labelDate.text = udate.toLocaleDateString("ru-RU", options) + "   ";
                selectedDate = udate.toLocaleDateString("ru-RU", options);
            };
        }

/************************************* Время выгрузки ****************************************/

/******************************* Industry Foundation Classes **********************************/
        CheckBoxALDE
        {
            id: cbIFC;
            enabled: mainWindow.itemsEnabled;
            objectName: "cbIFC";
            anchors.top: horizonRow.bottom;
            anchors.topMargin: 5;
            x: 35;
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
                id: line_cb_IFC;
                width: 560;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }

        ColumnLayout
        {
            id: horizIFC_Col_text;
            x: 30;
            anchors.topMargin: 10;
            anchors.top: cbIFC.bottom;
            LabelALDE
            {
                id: labelIFCPath;
                objectName: "text_IFCPath";
                text: "C:\\Documents and Settings";
            }
        }

        ColumnLayout
        {
            id: btnBrowseFolderCol;
            enabled: itemsEnabled;
            anchors.top: cbIFC.bottom;
            anchors.right: borderRect.right;
            RoundButton
            {
                id: btnBrowseFolder;
                text: "...";
                /* width: 15;
                height: 15; */
                onClicked:
                {
                    selectDirectoryDialog.open();
                }
            }
        }
/******************************* Industry Foundation Classes **********************************/

/************************************************ Версия IFC **********************************/

        Row
        {
            id: hRow;
            objectName: "row_IFCVersion";
            anchors.top: btnBrowseFolderCol.bottom;
            anchors.left: borderRect.left;
            anchors.topMargin: 5;

            LabelALDE
            {
                id: labelIFCVersion;
                anchors.top: horizIFC_Col_text.bottom;
                text: "Настройки IFC:";
            }

            /* 24.04.2025 Выбор версии IFC перенесен в отд. программу/окно
            ComboBox
            {
                id: cbIFCVersion;
                enabled: itemsEnabled;
                editable: false;
                anchors.left: labelIFCVersion.right;
                anchors.leftMargin: 10;
                anchors.top: labelIFCVersion.top;
                currentIndex: 10;
                model: ListModel
                {
                    id: revitIFCVersion;
                    ListElement { text: "Default"   }
                    ListElement { text: "IFCBCA"    }
                    ListElement { text: "IFC2x2"    }
                    ListElement { text: "IFC2x3"    }
                    ListElement { text: "IFCCOBIE"  }
                    ListElement { text: "IFC2x3CV2" }
                    ListElement { text: "IFC2x3FM"  }
                    ListElement { text: "IFC2x3BFM" }
                    ListElement { text: "IFC4"      }
                    ListElement { text: "IFC4DTV"   }
                    ListElement { text: "IFC4RV"    }
                }
            }
            */

            RoundButton
            {
                id: btnIFCSettings;
                text: "⚙️";
                enabled: itemsEnabled;
                anchors.left: labelIFCVersion.right;
                anchors.leftMargin: 15;
                y: -5;
                /* width: 30;
                height: 30; */
                ToolTip.text: "Настройки фaйлов IFC";
                ToolTip.visible: hovered;
                onClicked:
                {
                    signalBtnIFCSettingsClicked();
                }
            }
        }

/************************************************ Версия IFC **********************************/


/******************************* Navisworks **********************************/

        CheckBoxALDE
        {
            id: cbNavi;
            objectName: "cbNavi";
            enabled: itemsEnabled;
            anchors.top: hRow.bottom;
            anchors.topMargin: 40;
            x: 35
            checked: true;
            text: "Navisworks";
        }

        ColumnLayout
        {
            anchors.top: cbNavi.verticalCenter;
            x: 10;
            Rectangle   /* Горизонтальная линия */
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
            x: 130;
            Rectangle
            {
                id: line_cb_Navi;
                width: 680;
                Layout.fillWidth: true;
                Layout.preferredHeight: 1;
                color: "black";
            }
        }

/******************************* Navisworks **********************************/

        FileDialog
        {
            id: fileDialog;
            title: "Please choose a file";
            /* nameFilters: ["Revit Files (*.rvt *.ifc)"]; */
            nameFilters: ["Revit Files (*.rvt)"];
            onAccepted:
            {
                var path = fileDialog.selectedFile.toString();
                // remove prefixed "file:///"
                path = path.replace(/^(file:\/{3})/,"");
                // unescape html codes like '%23' for '#'

                signalIsFileExists(decodeURIComponent(path), cbVersion.currentText);
                if (fileExists)
                {
                    listModel.append({"path": decodeURIComponent(path) });
                    fileDialog.selectedFile = "";
                    fileDialog.close();
                }
            }
        }

        FolderDialog
        {
            id: selectDirectoryDialog;
            title: "Выберите папку для экспорта";
            onAccepted:
            {
                var path = selectDirectoryDialog.selectedFolder.toString();
                // remove prefixed "file:///"
                path = path.replace(/^(file:\/{3})/,"");
                // unescape html codes like '%23' for '#'
                labelIFCPath.text = decodeURIComponent(path);
                selectDirectoryDialog.selectedFolder = "";
                selectDirectoryDialog.close();
            }
        }
    }

    property var splashWindow: Window
    {
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

        Image
        {
            id: splashImage
            source: Images.qtLogo
            TapHandler
            {
                onTapped: splash.exit()
            }
        }

        function exit()
        {
            mainWindow.visible = true
            splash.visible = false
            splash.timeout()
        }

        //! [timer]
        Timer
        {
            interval: splash.timeoutInterval; running: splash.visible; repeat: false
            onTriggered: splash.exit()
        }
        //! [timer]
        visible: true
    }
}
