import QtQuick
import "shared/"
import QtQuick.Dialogs
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Item {

id: mainWindow;
width: 1300;
height: 870;

focus: true;
Keys.onEscapePressed: escKeyPressed();

property bool itemsEnabled: true;
property bool fileExists: false;

/* отдельный счётчик списка для файлов без галочки 3D */
property int rows_WO_views_n_sites: 0;

property int defaultSpacing: 10;
property int topOffset: 10;
property var idRowAdditionalFields;
property var rowsArray: [{ strHWND: "", hwnd: QtObject, lvRowIndx: 0, arrf_filePath: "", arrf_3DViewName: "", arrf_siteName: "", arrf_outputFileName: "", arrf_jsonPath: ""}];
property string home_directory: "";
property string selectedDate: new Date().toLocaleString(Qt.locale(),"dd.MM.yyyy");

signal signalBtnIFCSettingsClicked();
signal signalStopClicked();
signal signalRunClicked(rightNow: int, utime: string, udate: string);
signal escKeyPressedSignal();
signal signalSaveViewAndSiteToFile(fileName: string, viewName: string, site_Name: string, sigOutputFileName: string, sigparam_JsonPath: string, isAppend: int);

signal signalIsFileExists(fname: string, rvtVersion: string);

Component.onCompleted:
{
    /* array initialization, removing header line */
    rowsArray.splice(0, 1);
}

/* Обернули сигнал в ф-цию, чтобы вызывать его из backend */
function stopClicked()
{
    signalStopClicked();
    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Процесс закрыт"});
    itemsEnabled = true;
}

function addSubRowWrapper(lvMainRowId: int, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string)
{
    lvMain.addSubRow(lvMainRowId, _3dview_Text, site_Text, fNameText, JSON_text);
}

function addRowWithSubRowsFromCpp(lvMainRowId: int, filePath: string, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string)
{
    listModel.append({ "path": filePath });

    /* invalidating DOM immediately */
    lvMain.forceLayout();

    console.log("Adding checkboxed file " + filePath + " with index " + lvMainRowId);

    var lvDOM = lvMain.contentItem;

    for(var x = 0; x < lvDOM.children.length; ++x) {
        var firstChild = lvDOM.children[x];
        if (firstChild.children.length > 0){
            var secondChild = firstChild.children[0];
            if (secondChild.children.length > 1){
                var thirdChild = secondChild.children[2]; /* Наш пациент! */
                if (thirdChild.objectName === "cbExtract3D_" + lvMainRowId)
                {
                    var newlyCreatedItem = thirdChild;

                    newlyCreatedItem.cbProp_3DViewText = _3dview_Text;
                    newlyCreatedItem.cbProp_siteText = site_Text;
                    newlyCreatedItem.cbProp_filenameText = fNameText;
                    newlyCreatedItem.cbProp_jsonPathText = JSON_text;
                    newlyCreatedItem.checked = 1;

                    newlyCreatedItem.clicked();

                    /* Чистим, т.к. иначе текст запоминается в properties контрола и потом дублируется */
                    newlyCreatedItem.cbProp_3DViewText = "";
                    newlyCreatedItem.cbProp_siteText = "";
                    newlyCreatedItem.cbProp_filenameText = "";
                    newlyCreatedItem.cbProp_jsonPathText = "";
                }
            }
        }
    }
}

function addRowFromCpp(rvt_filePath: string)
{
    /* rows_WO_views_n_sites - это отдельный счётчик списка для файлов без галочки "3D" */
    ++rows_WO_views_n_sites;

    listModel.append({ "path": rvt_filePath });
}

function setInputFieldText(fieldName: string, objectName: string, text: string)
{
    for(var i = 0; i < rowsArray.length; i++)
    {
        if(rowsArray[i].hwnd.objectName === objectName)
        {
            rowsArray[i][fieldName] = text;
        }
    }
}

function removeSubRow(objectName: string)
{
    for(var i = 0; i < rowsArray.length; i++)
    {
        if(rowsArray[i].hwnd.objectName === objectName)
        {
            try{
                rowsArray.splice(i, 1);
                console.log("Removing position");
            }
            catch(error){}
        }
    }
}

function removeSubRows(lvMainRowId: int)    /* Удаляем все подпозиции, если галочку "3D" сняли */
{
    for(var i = 0; i < rowsArray.length; i++)
        if(rowsArray[i].lvRowIndx === lvMainRowId)
            try{
                (rowsArray[i].hwnd).destroy();
            }
            catch(error){}
    rowsArray = rowsArray.filter(function(a){return a.lvRowIndx !== lvMainRowId});
}

/* ф-ция также вызывается из closeEvent в backend.h */
function saveViewsAndSitesToFile()
{
    /* Пишем все вьюхи/площадки в отдельный файл строчками вида 'filename = viewname = sitename' */
    for (var x = 0; x < rowsArray.length; ++x){
            signalSaveViewAndSiteToFile(rowsArray[x].arrf_filePath, rowsArray[x].arrf_3DViewName, rowsArray[x].arrf_siteName, rowsArray[x].arrf_outputFileName, rowsArray[x].arrf_jsonPath, x);
    }
    /* Если нет файлов с указанными вьюхами/площадками/json'ами, то отправляем -1 и файл sav будет очищен */
    if (rowsArray.length == 0)
        signalSaveViewAndSiteToFile("", "", "", "", "", -1);
}

function escKeyPressed()
{
    saveViewsAndSitesToFile();
    escKeyPressedSignal();
}

/* Ф-ция вызывается из closeEvent в backend.h */
function getRows_WO_views_n_sites()
{
    return rows_WO_views_n_sites;
}

Connections
{
    target: datePicker;
    onDatePicked: {
        console.log(selectedDate);
    }
}

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
        text: "Нажмите сюда, чтобы добавить локальный проект";
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
            text: "RSN://ALD-VM-REVIT01/Projects/"
        }
    }

    Image
    {
        id: thePlusImage;
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
                    {
                        listModel.append({ "path": rsnEditBox.text });
                        ++rows_WO_views_n_sites;

                    }
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

    ScrollBar {
        id: vBar;
        active: true;
        anchors {
            right: lvMain.right;
            top: lvMain.top;
            bottom: lvMain.bottom;
            rightMargin: -18;
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
        width: 780;
        clip: true; /* Чтобы динамически создаваемы контролы не вылезали за пределы ListView */

        ScrollBar.vertical: vBar;

        delegate:
        Column
        {
          id: horizontalColumn;
          objectName: "summaryDelegate";
          Row {
            id: horizontalRow;
            Text
            {
                id: rowText;
                text: path;
            }

            /* spacer между именем файла и чекбоксом "3D" */
            Rectangle
            {
                width: lvMain.width - rowText.width - 35;
                height: 20;
            }

            CheckBox3DViews
            {
                id: cbExtract3D
                objectName: "cbExtract3D_" + index;
                ToolTip.text: "Указать 3D-виды и площадки";
                height: 17;
                width: 17;
                checked: false;
                property int trashcanVisible: 0;
                property string cbProp_3DViewText: "";
                property string cbProp_siteText: "";
                property string cbProp_filenameText: "";
                property string cbProp_jsonPathText: "";

                onClicked:{
                    idRowAdditionalFields = horizontalColumn;
                    if (cbExtract3D.checked)
                    {
                        var component = Qt.createComponent("shared\\PlusButtonRow.qml");
                        var subRow = component.createObject(idRowAdditionalFields, { "parentRef": mainWindow, "lvRowId":  index, "trashcanVisible": trashcanVisible,
                                                                "plusBtnProp_3dViewText": cbProp_3DViewText, "plusBtnProp_siteNam": cbProp_siteText, "plusBtnProp_outputFileNam" : cbProp_filenameText, "plusBtnProp_jsonFilePath" : cbProp_jsonPathText } );
                        subRow.objectName = subRow.toString();
                        rowsArray.push({strHWND: subRow.objectName, hwnd: subRow, lvRowIndx: index, arrf_filePath: path,
                                           arrf_3DViewName: cbProp_3DViewText, arrf_siteName: cbProp_siteText, arrf_outputFileName: cbProp_filenameText, arrf_jsonPath: cbProp_jsonPathText });
                    }
                    else
                    {
                        removeSubRows(index);
                        ++rows_WO_views_n_sites;
                    }
                }
            }
            Image
            {
                source: "resources/trash.png";
                width: 18;
                height: 18;
                MouseArea
                {
                    anchors.fill: parent;
                    onClicked: {
                        var removedIndex = index;
                        rows_WO_views_n_sites--;
                        removeSubRows(index);
                        listModel.remove(index);
                        try{
                            rowsArray.forEach(function(_row, _index) {
                                if (_row.lvRowIndx > removedIndex)
                                    _row.lvRowIndx--;
                            });
                        }
                        catch(error){ console.log(error)
                        }
                    }
                }
            }
          } /* Row */
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
        // Uses black magic to hunt for the delegate instance with the given
        // index. Returns undefined if there's no currently instantiated
        // delegate with that index.
        function addSubRow(index: int, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string){
            var i = 0;
            for(var x = 0; x < contentItem.children.length; ++x) {
                var item = contentItem.children[x];
                // We have to check for the specific objectName we gave our
                // delegates above, since we also get some items that are not
                // our delegates here.
                if (item.objectName === "summaryDelegate"){
                    if(i === index){
                        //return item;
                        var cbExtract3D_0 = item.children[0].children[2];

                        cbExtract3D_0.cbProp_3DViewText = _3dview_Text;
                        cbExtract3D_0.cbProp_siteText = site_Text;
                        cbExtract3D_0.cbProp_filenameText = fNameText;
                        cbExtract3D_0.cbProp_jsonPathText = JSON_text;
                        cbExtract3D_0.trashcanVisible = 1;

                        cbExtract3D_0.clicked();                        

                        /* Чистим, т.к. иначе текст запоминается в properties контрола и потом дублируется */
                        cbExtract3D_0.trashcanVisible = 0;
                        cbExtract3D_0.cbProp_3DViewText = "";
                        cbExtract3D_0.cbProp_siteText = "";
                        cbExtract3D_0.cbProp_filenameText = "";
                        cbExtract3D_0.cbProp_jsonPathText = "";
                    }
                    ++i;
                }
            }
            return undefined;
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
            if (lvMain.count !== 0)
                menuLaunch.open()
            else
                lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Добавьте, как минимум, один файл в список для экспорта!"});
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
                    saveViewsAndSitesToFile();
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
                    saveViewsAndSitesToFile();
                    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Назначенное время " + selectedDate + ", " + selectedTime.hour.toString() + ":" + selectedTime.minute.toString()});
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
            stopClicked();
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
            var i = getTime();
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
            x: 140
            enabled: itemsEnabled;
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 0;
            spacing: 6
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
            var options = { day: 'numeric', month: 'numeric', year: 'numeric' };
            labelDate.text = udate;
            selectedDate = udate;
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

    LabelALDE
    {
        id: labelSelectDir;
        anchors.topMargin: 10;
        x: 20;
        anchors.top: cbIFC.bottom;
        text: "Директория для выгрузки файлов:";
    }

    ColumnLayout
    {
        id: horizIFC_Col_text;
        x: 30;
        anchors.topMargin: 5;
        anchors.top: labelSelectDir.bottom;
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
            ToolTip.text: "Нажмите, чтобы выбрать директорию для экспорта";
            ToolTip.visible: hovered;
            onClicked:
            {
                selectDirectoryDialog.open();
            }
        }
    }
/******************************* Industry Foundation Classes **********************************/

/************************************************ Версия IFC **********************************

    Row
    {
        id: hRow;
        objectName: "row_IFCVersion";
        anchors.top: btnBrowseFolderCol.bottom;
        anchors.left: borderRect.left;
        anchors.topMargin: 5;

        /* rib 2.12.25 создано отд.поле для json, в кот. указ. верс. IFC
        LabelALDE
        {
            id: labelIFCVersion;
            anchors.top: horizIFC_Col_text.bottom;
            text: "Задать файл конфигурации .json и версию IFC:";
        }

        /* rib 24.04.2025 Выбор версии IFC перенесен в отд. программу/окно
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

        /* rib 2.12.25 создано отд.поле для json, в кот. указ. верс. IFC
        RoundButton
        {
            id: btnIFCSettings;
            text: "⚙️";
            enabled: itemsEnabled;
            anchors.left: labelIFCVersion.right;
            anchors.leftMargin: 15;
            y: -10;
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
        anchors.top: btnBrowseFolderCol.bottom;
        anchors.topMargin: 30;
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
        title: "Выберите директорию для экспорта";
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
        source: "shared/images/qt-logo.png";
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
