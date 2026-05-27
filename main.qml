/*
  - rir 18.2.2026
  - last changed 24.4.2026
 */

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
Keys.onEscapePressed: esc_key_pressed();

property bool items_enabled: true;

/* отдельный счётчик списка для файлов без галочки 3D */
property int rows_wo_views_n_sites: 0;

property int default_spacing: 10;
property int top_offset: 10;
property var id_row_additional_fields;
property var rows_array: [{ str_hwnd: "", hwnd: QtObject, lv_row_index: 0, arrf_file_path: "", arrf_3dview_name: "",
        arrf_site_name: "", arrf_output_file_name: "", arrf_json_path: "", arrf_should_be_exported: true }];

property var empty_array_structure_template: {
    str_hwnd: "";
    hwnd: QtObject;
    lv_row_index: 0;
    arrf_file_path: "";
    arrf_3dview_name: "";
    arrf_site_name: "";
    arrf_output_file_name: "";
    arrf_json_path: "";
    arrf_should_be_exported: true;
}

property string home_directory: "";
property string selectedDate: new Date().toLocaleString(Qt.locale(),"dd.MM.yyyy");

signal signal_stop_clicked();
signal signal_copy_to_clipboard_clicked();
signal signal_run_clicked(rightNow: int, utime: string, udate: string);
signal signal_esc_key_pressed();
signal signal_save_views_and_sites_to_file(sig_file_name: string, sig_view_name: string, sig_site_name: string,
                                           sig_output_file_name: string, sig_json_path: string, sig_should_be_exported: bool, sig_is_append: int);
signal signal_clear_log_files();
SensitiveData { id: sensitive_data; }

Component.onCompleted: {
    /* array initialization, removing header line */
    rows_array.splice(0, 1);
}

/* Обернул сигнал в ф-цию, чтобы вызывать его из backend */
function stop_clicked() {
    signal_stop_clicked();
    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Процесс закрыт"});
    items_enabled = true;
}

function show_message_box() {
    message_dialog.open();
}

function clear_main_list() {
    /* такой костыль по очистке массива, как и весь js */
    rows_array = [ Object.assign( {}, empty_array_structure_template ) ];
    listModel.clear();
}

function add_item_cb_files_sav(sav_file_name: string) {
    model_sav_file.append( { "text": sav_file_name} );
}

function add_subrow_wrapper(lvMainRowId: int, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string) {
    lvMain.add_subrow(lvMainRowId, _3dview_Text, site_Text, fNameText, JSON_text);
}

function add_row_from_cpp(rvt_file_path: string, should_be_exported: bool) {
    /* rows_WO_views_n_sites - это отдельный счётчик списка для файлов без галочки "3D" */
    ++rows_wo_views_n_sites;

    listModel.append({ "path": rvt_file_path, "shouldExport": should_be_exported });
}

function add_row_w_subrows_from_cpp(lvMainRowId: int, filePath: string, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string, should_be_exported: bool) {
    listModel.append({ "path": filePath, "shouldExport": should_be_exported });

    /* invalidating DOM immediately */
    lvMain.forceLayout();

    console.log("Adding 3D-checkboxed file " + filePath + " with index " + lvMainRowId);

    var lvDOM = lvMain.contentItem;

    for(var x = 0; x < lvDOM.children.length; ++x) {
        var firstChild = lvDOM.children[x];
        if (firstChild.children.length > 0){
            var secondChild = firstChild.children[0]; /* secondChild это horizontalRow */
            secondChild.shouldExport = should_be_exported; /* Из его propery shouldExport берется состояние для rows_array.arrf_should_be_exported */
            if (secondChild.children.length > 1){
                var cb_should_exported = secondChild.children[0];

                var thirdChild = secondChild.children[3]; /* Наш пациент! */
                if (thirdChild.objectName === "cbExtract3D_" + lvMainRowId)
                {
                    var newlyCreatedItem = thirdChild;

                    newlyCreatedItem.cbProp_3DViewText = _3dview_Text;
                    newlyCreatedItem.cbProp_siteText = site_Text;
                    newlyCreatedItem.cbProp_filenameText = fNameText;
                    newlyCreatedItem.cbProp_jsonPathText = JSON_text;
                    newlyCreatedItem.checked = 1;

                    newlyCreatedItem.clicked();

                    /* Галочка "Выгружать/Не выгружать" */
                    cb_should_exported.checked = should_be_exported;

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

function set_input_field_text(fieldName: string, objectName: string, text: string) {
    for(var i = 0; i < rows_array.length; i++)
    {
        if(rows_array[i].hwnd.objectName === objectName)
        {
            rows_array[i][fieldName] = text;
        }
    }
}

function remove_subrow(objectName: string) {
    for(var i = 0; i < rows_array.length; i++)
    {
        if(rows_array[i].hwnd.objectName === objectName)
        {
            try {
                rows_array.splice(i, 1);
                console.log("Removing position");
            }
            catch(error){}
        }
    }
}

function remove_subrows(lvMainRowId: int) {   /* Удаляем все подпозиции, если галочку "3D" сняли */

    for(var i = 0; i < rows_array.length; i++)
        if(rows_array[i].lv_row_index === lvMainRowId)
            try {
                (rows_array[i].hwnd).destroy();
            }
            catch(error){}
    rows_array = rows_array.filter(function(a){return a.lv_row_index !== lvMainRowId});
}

/* ф-ция также вызывается из closeEvent в backend.h */
function save_views_and_sites_to_file() {
    /* Сортируем массив перед записью в файл по полю "Имя RVT-файла", т.к. иначе, идущие не подряд одинаковые RVT-файлы, будут при запуске экспортера создавать отдельные позиции */
    rows_array.sort( (a, b) => (a.arrf_file_path > b.arrf_file_path) ? 1 : ((b.arrf_file_path > a.arrf_file_path) ? -1 : 0));

    /* Пишем все вьюхи/площадки в отдельный файл строчками вида 'filename = viewname = sitename' */
    for (var x = 0; x < rows_array.length; ++x){
            signal_save_views_and_sites_to_file(rows_array[x].arrf_file_path, rows_array[x].arrf_3dview_name, rows_array[x].arrf_site_name,
                                                rows_array[x].arrf_output_file_name, rows_array[x].arrf_json_path, rows_array[x].arrf_should_be_exported, x);
    }
    /* Если нет файлов с указанными вьюхами/площадками/json'ами, то отправляем -1 и файл sav будет очищен */
    if (rows_array.length == 0)
        signal_save_views_and_sites_to_file("", "", "", "", "", false, -1);
}

function esc_key_pressed() {
    save_views_and_sites_to_file();
    signal_esc_key_pressed();
}

/* Ф-ция вызывается из closeEvent в backend.h */
function get_rows_wo_views_n_sites() {
    return rows_wo_views_n_sites;
}

Connections {
    target: datePicker;
    onDatePicked: {
        console.log(selectedDate);
    }
}

MouseArea {
    anchors.fill: parent;
    onClicked:
        function(mouse){
            if (!datePicker.contains(Qt.point(mouse.x, mouse.y)))
            {
                    datePicker.visible = false; /*  clicked outside datePicker;  */
            }
        }
}

Rectangle {
    y: 5;
    x: 10;

    height: 750;
    anchors.fill: parent;
    anchors.margins: default_spacing;

    RoundButton {
        id: btnAddLocalProject;
        enabled: items_enabled;
        anchors {
            left: parent.left;
            top: parent.top;
            margins: default_spacing;
        }
        text: "Нажмите сюда, чтобы добавить локальный проект";
        height: 22;
        width: 800;
        onClicked: fileDialog.open();
    }

    Row {
        id: theTextBoxRow;
        anchors.top: btnAddLocalProject.bottom;
        anchors.left: btnAddLocalProject.left;
        anchors.topMargin: 10;
        spacing: 25;
        TextField {
            id: rsnEditBox;
            width: btnAddLocalProject.width-50;
            text: sensitive_data.revit_server_hostname;
        }
    }

    Image {
        id: thePlusImage;
        source: "resources/plus.png";
        enabled: items_enabled;
        anchors.right: btnAddLocalProject.right;
        anchors.top: theTextBoxRow.top;
        width: 15;
        height: 15;
        MouseArea {
            anchors.fill: parent;
            onClicked: {
                if ( rsnEditBox.text !== "") {
                    listModel.append({ "path": rsnEditBox.text, shouldExport: true });
                    ++rows_wo_views_n_sites;
                }
                rsnEditBox.text = "";
            }
        }
    }

    CustomBorderRect {
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

    CustomBorderRect {
        id: logFrame;
        y: 10;
        anchors.margins: 10;
        anchors.left: borderRect.right;
        width : 470;
        height: 840;
        color: "white";

        lBorderwidth: 1;
        rBorderwidth: 0;
        tBorderwidth: 1;
        bBorderwidth: 1;
        borderColor: "black";
    }

    RLabel {
        id: labelLog;
        x: 1000;
        anchors.top: btnAddLocalProject.top;
        Text {
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

    ListView {
        id: lvMain;
        enabled: items_enabled;
        objectName: "o_lvMain";
        anchors.top: borderRect.top;
        anchors.margins: 10;
        anchors.left: parent.left;
        height: 500;
        width: 780;
        clip: true; /* Чтобы динамически создаваемые контролы не вылезали за пределы ListView */

        ScrollBar.vertical: vBar;

        delegate:
        Column {
          id: horizontalColumn;
          objectName: "summaryDelegate";

          Row {
            id: horizontalRow;
            objectName: "horizontalRow";
            property bool shouldExport: true;   /* Значение меняется ф-циями add_row_from_cpp и add_row_w_subrows_from_cpp */

            CheckBox {
                id: cbShouldBeExported;
                objectName: "cbShouldBeExported_" + index;
                ToolTip.text: "Выгружать файл - да/нет";
                ToolTip.visible: hovered;
                height: 17;
                width: 17;
                checked: shouldExport;
                onClicked: {
                    if (cbShouldBeExported.checked){
                        var checkedFilePath = listModel.get(index).path;
                        for(var i = 0; i < rows_array.length; i++)
                        {
                            if(rows_array[i].arrf_file_path === checkedFilePath) {
                                rows_array[i].arrf_should_be_exported = true;
                            }
                        }

                        listModel.get(index).shouldExport = true;
                    }
                    else {
                        var uncheckedFilePath = listModel.get(index).path;
                        for(var i = 0; i < rows_array.length; i++)
                        {
                            if(rows_array[i].arrf_file_path === uncheckedFilePath)
                            {
                                rows_array[i].arrf_should_be_exported = false;
                            }
                        }

                        listModel.get(index).shouldExport = false;
                    }
                }
            }

            Text {
                id: rowText;
                text: path;
            }

            /* spacer между именем файла и чекбоксом "3D" */
            Rectangle {
                width: lvMain.width - rowText.width - 52;
                height: 20;
            }

            CheckBox3DViews {
                id: cbExtract3D;
                objectName: "cbExtract3D_" + index;
                ToolTip.text: "Указать 3D-виды и площадки";
                ToolTip.visible: hovered;
                height: 17;
                width: 17;
                checked: false;
                property int trashcanVisible: 0;
                property string cbProp_3DViewText: "";
                property string cbProp_siteText: "";
                property string cbProp_filenameText: "";
                property string cbProp_jsonPathText: "";                

                onClicked: {
                    id_row_additional_fields = horizontalColumn;
                    if (cbExtract3D.checked)
                    {
                        var component = Qt.createComponent("shared\\PlusButtonRow.qml");
                        var subRow = component.createObject(id_row_additional_fields, { "parentRef": mainWindow, "lvRowId":  index, "trashcanVisible": trashcanVisible,
                                                                "plusBtnProp_3dViewText": cbProp_3DViewText, "plusBtnProp_siteNam": cbProp_siteText,
                                                                "plusBtnProp_outputFileNam" : cbProp_filenameText, "plusBtnProp_jsonFilePath" : cbProp_jsonPathText } );
                        subRow.objectName = subRow.toString();
                        rows_array.push({str_hwnd: subRow.objectName, hwnd: subRow, lv_row_index: index, arrf_file_path: path,
                                           arrf_3dview_name: cbProp_3DViewText, arrf_site_name: cbProp_siteText, arrf_output_file_name: cbProp_filenameText,
                                            arrf_json_path: cbProp_jsonPathText, arrf_should_be_exported: parent.shouldExport });
                    }
                    else
                    {
                        remove_subrows(index);
                        ++rows_wo_views_n_sites;
                    }
                }
            }
            Image {
                source: "resources/trash.png";
                width: 18;
                height: 18;
                MouseArea {
                    anchors.fill: parent;
                    onClicked: {
                        var removedIndex = index;
                        rows_wo_views_n_sites--;
                        remove_subrows(index);
                        listModel.remove(index);
                        try {
                            rows_array.forEach(function(_row, _index) {
                                if (_row.lv_row_index > removedIndex)
                                    _row.lv_row_index--;
                            });
                        }
                        catch(error){ console.log(error)
                        }
                    }
                }
            }
          } /* Row */
        }
        model: ListModel {
            id: listModel;
/*
            ListElement {
                path: "C:\\Projects\\Autodesk\\wall.rvt"; shouldExport: true;
            }
            ListElement {
                path: "\\\\srv-c666-666\\Projects\\Revit\\arm.rvt"; shouldExport: true;
            }
            ListElement {
                path: "RSN://Projects/101/floor.rvt"; shouldExport: true;
            }

            /* bool file_exists, bool remove_or_not
            function set_var_file_exists_from_cpp(remove_or_not) { file_exists = remove_or_not; }
*/
        }

        /* Uses black magic to hunt for the delegate instance with the given
         * index. Returns undefined if there's no currently instantiated
         * delegate with that index.
         */
        function add_subrow(index: int, _3dview_Text: string, site_Text: string, fNameText: string, JSON_text: string) {
            var i = 0;
            for(var x = 0; x < contentItem.children.length; ++x) {
                var item = contentItem.children[x];
                // We have to check for the specific objectName we gave our
                // delegates above, since we also get some items that are not
                // our delegates here.
                if (item.objectName === "summaryDelegate"){
                    if(i === index){
                        var cbExtract3D_0 = item.children[0].children[3];

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

    ListView {
        id: lvLog;
        anchors.top: logFrame.top;
        anchors.margins: 10;
        anchors.left: logFrame.left;
        height: 760;
        width: 490;
        objectName: "o_lvLog";
        clip: true; /* Чтобы динамически создаваемые контролы не вылезали за пределы ListView */


        ScrollBar.horizontal:  ScrollBar {
            id: hscroll_bar;
            active: ScrollBar.AlwaysOn;
            policy: ScrollBar.AlwaysOn;
            width: 5;
            anchors {
                right: lvLog.right;
                bottom: lvLog.bottom;
            }
        }
        flickableDirection: Flickable.HorizontalAndVerticalFlick;
        contentWidth: 3000;


        delegate:
            Text {
                x: 5;
                id: rowTxt;
                text: msg
            }

        model: ListModel {
            id: lmLogModel;
            ListElement {
                msg: "";
            }

            function add_to_log_listview(caption) {
                lmLogModel.append({"msg": new Date().toLocaleTimeString() + " " +caption});
            }
        }
        onCountChanged: {
            lvLog.currentIndex = lvLog.count - 1;
            lvLog.positionViewAtEnd();
        }
    }

    RoundButton {
        id: btnSaveTrueToConfig;
        objectName: btnRun;
        enabled: mainWindow.items_enabled;
        anchors.right: parent.right;
        anchors.bottom: parent.bottom;
        anchors.margins: mainWindow.default_spacing;
        text: "🚀";
        width: 45;
        height: 45;
        onClicked: {
            if (lvMain.count !== 0)
                menuLaunch.open()
            else
                lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Добавьте, как минимум, один файл в список для экспорта!"});
        }

        Menu {
            id: menuLaunch;
            y: btnSaveTrueToConfig.height;
            MenuItem {
                id: menuRunNow;
                text: "Запустить сейчас";
                onTriggered: {
                    var selectedTime = new Date().toLocaleString(Qt.locale(),"hh:mm");
                    save_views_and_sites_to_file();
                    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Запуск прямо сейчас (в " +  selectedTime + ")"});
                    signal_run_clicked(1, selectedTime, new Date().toLocaleString(Qt.locale(),"dd.MM.yyyy"));   /* "1" - запустить прямо сейчас */
                    items_enabled = false;
                    btnStop.enabled = true;
                }
            }
            MenuItem {
                id: menuRunScheduled;
                text: "По указанному времени";
                onTriggered: {
                    var selectedTime = uTime.getTime();
                    save_views_and_sites_to_file();
                    lmLogModel.append({"msg": new Date().toLocaleTimeString() + " | Назначенное время " + selectedDate + ", " + selectedTime.hour.toString() + ":" + selectedTime.minute.toString()});
                    signal_run_clicked(0, selectedTime.hour.toString() + ":" + selectedTime.minute.toString(), selectedDate);
                    items_enabled = false;
                    btnStop.enabled = true;
                }
            }
        }
    }

    RoundButton {
        id: btnStop;
        enabled: mainWindow.items_enabled;
        anchors.right: btnSaveTrueToConfig.left;
        anchors.bottom: parent.bottom;
        anchors.margins: mainWindow.default_spacing;
        text: "⬛";
        width: 45;
        height: 45;
        onClicked: {
            stop_clicked();
        }
    }

    RoundButton {
        id: btnClipboard;
        anchors.left: lvLog.left;
        anchors.bottom: parent.bottom;
        anchors.bottomMargin: 10;

        ToolTip.text: "Копировать журнал в буфер и очистить";
        ToolTip.visible: hovered;
        text: "📋";
        width: 45;
        height: 45;
        onClicked: {
            menu_log_content.open();
        }

        Menu {
            id: menu_log_content;
            y: btnClipboard.height;
            MenuItem {
                id: menu_copy_and_clear_log;
                text: "Copy log and clear";
                onTriggered: {
                    signal_copy_to_clipboard_clicked();
                    lmLogModel.clear();
                }
            }
            MenuItem
            {
                id: menu_clear_logs;
                text: "Очистить файлы журналов";
                onTriggered:
                {
                    signal_clear_log_files();
                }
            }
        }
    }

/************************************* Настройки экспорта ************************************/

    RLabel {
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
    Row {
        id: horizRow;
        objectName: "row_RvtVersion";
        anchors.top: labelExportSettings.bottom;
        anchors.left: borderRect.left;
        anchors.topMargin: 5;

        RLabel {
            id: labelRevitVersion;
            anchors.top: labelExportSettings.bottom;
            text: "Версия Revit:";
        }

        ComboBox {
            id: cbVersion;
            editable: false;
            enabled: items_enabled;
            anchors.left: labelRevitVersion.right;
            anchors.leftMargin: 10;
            anchors.top: labelExportSettings.bottom;
            currentIndex: 1;
            model: ListModel {
                id: revitVersion;
                ListElement { text: "2022" }
                ListElement { text: "2023" }
                ListElement { text: "2026" }
            }
        }

/****************************************** sav-файл ******************************************/

        RLabel {
            id: label_sav_file;
            anchors.top: labelExportSettings.bottom;
            anchors.leftMargin: 110;
            anchors.left: cbVersion.right;
            text: "Файл выгрузки:";
        }

        ComboBox {
            id: cb_sav_file;
            objectName: "cb_sav_file";
            editable: false;
            enabled: items_enabled;
            anchors.left: label_sav_file.right;
            anchors.leftMargin: 10;
            anchors.top: labelExportSettings.bottom;
            currentIndex: 0;
            width: 240;
            model: ListModel {
                id: model_sav_file;
                objectName: "model_sav_file";
                ListElement { text: "views_sites.sav" }
            }
            /* sav_file_choose_dialog и так вызывает on_sav_combo_changed
            onCountChanged: {
                    backend.on_sav_combo_changed(currentIndex, currentValue, "");
                }
            */
            onActivated: {
                backend.on_sav_combo_changed(currentIndex, currentValue, "");
            }
        }

        Image {
            id: the_plus_image;
            source: "resources/plus.png";
            enabled: items_enabled;
            anchors.left: cb_sav_file.right;
            anchors.top: labelExportSettings.bottom;
            anchors.leftMargin: 10;
            width: 15;
            height: 15;
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                   sav_file_choose_dialog.open();
                }
            }
        }

        Image {
            id: img_recyclebin;
            source: "resources/recycle_bin.png";
            enabled: false;
            anchors.left: the_plus_image.right;
            anchors.top: labelExportSettings.bottom;
            anchors.leftMargin: 10;
            width: 15;
            height: 15;
            MouseArea {
                anchors.fill: parent;
                onClicked: {
                   //cb_sav_file.
                }
            }
        }

        FileDialog {
            id: sav_file_choose_dialog;
            title: "Please choose a sav-file";
            nameFilters: [".sav-files (*.sav)"];
            onAccepted: {
                var path = sav_file_choose_dialog.selectedFile.toString();
                // remove prefixed "file:///"
                path = path.replace(/^(file:\/{3})/,"");
                // unescape html codes like '%23' for '#'
                var file_name = path.toString().split('/').pop();

                if (cb_sav_file.find(file_name) !== -1) {
                    lmLogModel.add_to_log_listview("| Файл с таким именем уже был ранее добавлен в список!");
                }
                else {
                    model_sav_file.append( { "text": decodeURIComponent(file_name) } );
                    sav_file_choose_dialog.selectedFile = "";
                    sav_file_choose_dialog.close();
                    cb_sav_file.currentIndex = cb_sav_file.count - 1;
                    backend.on_sav_combo_changed(cb_sav_file.currentIndex, cb_sav_file.currentValue, path);
                }
            }
        }
    }
/************************************* Время/Дата выгрузки ****************************************/

    Row {
        id: horizonRow;

        anchors.top: horizRow.bottom;
        anchors.topMargin: 25;
        x: 90;
        RLabel {
            id: labelTime;
            text: "Время выгрузки";
        }
    }

    UTimePicker {
        id: uTime;
        enabled: mainWindow.items_enabled;
        objectName: "uTime";
        anchors.top: horizRow.top;
        anchors.topMargin: 10;
        x: 65;
        width: 200;             // Font Size <-> depends!
        size: Qt.size(0, 40);
        onChanged: {
            var i = getTime();
        }
    }

    Row {
        id: horizontalDateRow;

        anchors.top: horizRow.bottom;
        anchors.topMargin: 25;
        anchors.leftMargin: 25;
        x: 290;
        RLabel {
            id: labelDateText;
            text: "Дата выгрузки: ";
        }
        RLabel {
            id: labelDate;
            text: Qt.formatDateTime(new Date(), "dd.MM.yy" + "   ");
        }

        RoundButton {
            id: btnPickDate;
            x: 140
            enabled: items_enabled;
            anchors.bottom: parent.bottom;
            anchors.bottomMargin: 0;
            spacing: 6
            anchors.topMargin: 20;
            width: 14;
            height: 14;
            onClicked: {
                datePicker.visible = true;
            }
        }
    }

    JWDMDatePicker {
        id: datePicker;
        visible: false;
        enabled: mainWindow.items_enabled;
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

    RCheckBox {
        id: cbIFC;
        enabled: mainWindow.items_enabled;
        objectName: "cbIFC";
        anchors.top: horizonRow.bottom;
        anchors.topMargin: 5;
        x: 35;
        checked: true;
        text: "Industry Foundation Classes (IFC)";
    }

    ColumnLayout {
        anchors.top: cbIFC.verticalCenter;
        x: 10;
        Rectangle {
            id: lineIFC;
            width: 20;
            Layout.fillWidth: true;
            Layout.preferredHeight: 1;
            color: "black";
        }
    }

    ColumnLayout {
        id: line_cb_IFC_col;
        anchors.top: cbIFC.verticalCenter;
        x: 250;
        Rectangle {
            id: line_cb_IFC;
            width: 560;
            Layout.fillWidth: true;
            Layout.preferredHeight: 1;
            color: "black";
        }
    }

    RLabel {
        id: labelSelectDir;
        anchors.topMargin: 10;
        x: 20;
        anchors.top: cbIFC.bottom;
        text: "Директория для выгрузки файлов:";
    }

    ColumnLayout {
        id: horizIFC_Col_text;
        x: 30;
        anchors.topMargin: 5;
        anchors.top: labelSelectDir.bottom;
        RLabel {
            id: labelIFCPath;
            objectName: "text_IFCPath";
            text: "C:\\Documents and Settings";
        }
    }

    ColumnLayout {
        id: btnBrowseFolderCol;
        enabled: items_enabled;
        anchors.top: cbIFC.bottom;
        anchors.right: borderRect.right;
        RoundButton {
            id: btnBrowseFolder;
            text: "...";
            ToolTip.text: "Нажмите, чтобы выбрать директорию для экспорта";
            ToolTip.visible: hovered;
            onClicked: {
                selectDirectoryDialog.open();
            }
        }
    }
/******************************* Industry Foundation Classes **********************************/

/******************************* Navisworks **********************************/

    RCheckBox {
        id: cbNavi;
        objectName: "cbNavi";
        enabled: items_enabled;
        anchors.top: btnBrowseFolderCol.bottom;
        anchors.topMargin: 30;
        x: 35
        checked: false;
        text: "Navisworks";
        onCheckedChanged: {
            if(!cbNavi.checked && !cbIFC.checked) {
                btnSaveTrueToConfig.enabled = false;
            }
        }
    }

    ColumnLayout {
        anchors.top: cbNavi.verticalCenter;
        x: 10;
        Rectangle {  /* Горизонтальная линия */
            id: lineNavi;
            width: 20;
            Layout.fillWidth: true;
            Layout.preferredHeight: 1;
            color: "black";
        }
    }

    ColumnLayout {
        id: line_cb_Navi_col;
        anchors.top: cbNavi.verticalCenter;
        x: 130;
        Rectangle {
            id: line_cb_Navi;
            width: 680;
            Layout.fillWidth: true;
            Layout.preferredHeight: 1;
            color: "black";
        }
    }

/******************************* Navisworks **********************************/

    RLabel {
        id: labelInfoPrio;
        anchors.topMargin: 30;
        x: 10;
        anchors.top: line_cb_Navi_col.bottom;
        text: "* При изменении файла выгрузки, файлы без отметки '3D' будут удалены из views_sites.sav";
    }

    /* Круг с вопросительным знаком */
    Rectangle {
        width: 20;
        height: 20;
        anchors.left: labelInfoPrio.right;
        anchors.top: labelInfoPrio.top;
        anchors.leftMargin: 10;
        border.color: "gray";
        border.width: 1;
        radius: width * 0.5;
        RoundButton {
            id: textHelpInfo;
            enabled: false;
            anchors.centerIn: parent
            text: "?";
            ToolTip.text: "При изменении файла выгрузки или добавлении нового, файлы без отметки '3D' будут удалены из предыдущего списка";
            ToolTip.visible: hovered;
            width: 20;
            height: 20;
        }
    }

    FileDialog {
        id: fileDialog;
        title: "Please choose a file";
        /* nameFilters: ["Revit Files (*.rvt *.ifc)"]; */
        nameFilters: ["Revit Files (*.rvt)"];
        onAccepted: {
            var path = fileDialog.selectedFile.toString();
            // remove prefixed "file:///"
            path = path.replace(/^(file:\/{3})/,"");
            // unescape html codes like '%23' for '#'

            listModel.append({"path": decodeURIComponent(path), shouldExport: true });
            fileDialog.selectedFile = "";
            fileDialog.close();
        }
    }

    FolderDialog {
        id: selectDirectoryDialog;
        title: "Выберите директорию для экспорта";
        onAccepted: {
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

    MessageDialog {
        id: message_dialog;
        title: "Information";
        text: "The operation completed successfully.";
        buttons: MessageDialog.Ok;
        /* onAccepted: console.log("clicked OK"); */
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
        source: "shared/images/qt-logo.png";
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
