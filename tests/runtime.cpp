#include "backend.h"
#include "named_pipe_client.h"
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QThread>
#include <QLocalServer>
#include <cstdio>
#include <functional>
#include <stdexcept>

static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
static bool wait_for(const std::function<bool()>& condition) {
    QElapsedTimer timer;
    timer.start();
    while (!condition() && timer.elapsed() < 3000) {
        QCoreApplication::processEvents();
        QThread::msleep(1);
    }
    return condition();
}
static QByteArray contents(const QString& path) {
    QFile file(path);
    require(file.open(QIODevice::ReadOnly), "read test INF");
    return file.readAll();
}
int main(int argc, char** argv) {
    std::fprintf(stderr, "TEST: starting Qt\n");
    qputenv("QT_QPA_PLATFORM", "offscreen");
    QGuiApplication app(argc, argv);
    std::fprintf(stderr, "TEST: Qt ready\n");
    QTemporaryDir temporary;
    qputenv("APPDATA", temporary.path().toUtf8());
    QDir().mkpath(temporary.path() + app_directory);
    try {
        QTcpServer probe;
        require(probe.listen(QHostAddress::LocalHost, 0), "reserve port");
        const auto port = probe.serverPort();
        probe.close();
        const QString inf = temporary.path() + app_directory + "/ifcexprt.inf";
        QFile initial(inf);
        require(initial.open(QIODevice::WriteOnly), "create INF");
        initial.write("[Manufacturer]\ntcp_port=" + QByteArray::number(port) + "\n");
        initial.close();
        std::fprintf(stderr, "TEST: loading QML\n");
        ExportQuickView view;
        view.setSource(QUrl("qrc:/main.qml"));
        auto* root = view.rootObject();
        require(root != nullptr, "load production QML");
        std::fprintf(stderr, "TEST: QML ready\n");
        BackEnd backend(&app, root, &view);
        view.rootContext()->setContextProperty("backend", &backend);
        QObject::connect(&backend, &BackEnd::launch_failed, root, [root]() {
            QMetaObject::invokeMethod(root, "launch_failed");
        });
        QObject::connect(root, SIGNAL(signal_stop_clicked()), &backend, SLOT(slot_stop_clicked()));
        auto* log = root->findChild<QAbstractListModel*>("model_log");
        auto* files = root->findChild<QAbstractListModel*>("model_files");
        auto* version = root->findChild<QQuickItem*>("combo_revit_version");
        require(log && files && version, "named QML objects");
        require(QMetaObject::invokeMethod(root, "add_row_from_cpp", Q_ARG(QString, "C:/test.rvt"), Q_ARG(bool, true)), "add file");
        require(files->rowCount() == 1, "file model");

        // Ошибка QML не должна менять INF и оставлять кнопки заблокированными.
        int failures = 0;
        QObject::connect(&backend, &BackEnd::launch_failed, [&]() { ++failures; });
        const auto before = contents(inf);
        version->setObjectName("missing");
        root->setProperty("items_enabled", false);
        backend.slot_run_clicked(1, "12:34", "01.01.2030");
        require(failures == 1 && contents(inf) == before && root->property("items_enabled").toBool(), "QML failure preserves INF and restores controls");
        version->setObjectName("combo_revit_version");

        // Плановый запуск сохраняет дату и время; завершение приходит по TCP.
        backend.slot_run_clicked(0, "12:34", "01.01.2030");
        require(backend.read_inf_string("ControlFlags", "Date") == "01.01.2030", "scheduled date");
        require(backend.read_inf_string("ControlFlags", "Time") == "12:34", "scheduled time");
        QTcpSocket first, second;
        first.connectToHost(QHostAddress::LocalHost, port);
        second.connectToHost(QHostAddress::LocalHost, port);
        require(wait_for([&]() { return first.state() == QAbstractSocket::ConnectedState && second.state() == QAbstractSocket::ConnectedState; }), "TCP connections");
        wait_for([&]() { return backend.findChildren<QTcpSocket*>().size() >= 2; });
        root->setProperty("items_enabled", false);
        const auto baseline = log->rowCount();
        first.write("first\nEnd of ex"); first.flush();
        require(wait_for([&]() { return log->rowCount() == baseline + 1; }), "complete line before partial record");
        second.write("independent\r\n"); second.flush();
        require(wait_for([&]() { return log->rowCount() == baseline + 2; }), "per-connection buffers and CRLF");
        require(!root->property("items_enabled").toBool(), "partial completion must not finish export");
        first.write("port\n"); first.flush();
        require(wait_for([&]() { return root->property("items_enabled").toBool(); }), "fragmented completion restores controls");
        require(backend.read_inf_string("ControlFlags", "Enabled") == "false", "completion disables export");
        const QByteArray utf8 = QString::fromUtf8("Привет").toUtf8();
        const auto rows = log->rowCount();
        first.write(utf8.left(1)); first.flush();
        QCoreApplication::processEvents();
        first.write(utf8.mid(1) + "\njoined\n"); first.flush();
        require(wait_for([&]() { return log->rowCount() == rows + 2; }), "split UTF-8 and joined lines");
        require(log->data(log->index(rows, 0), 0).toString().contains(QString::fromUtf8("Привет")), "UTF-8 integrity");

        // Ошибка сохранения отменяет запуск, сохраняя прежний файл.
        backend.inf_file = temporary.path() + "/missing/subdir/ifcexprt.inf";
        backend.slot_run_clicked(1, "12:34", "01.01.2030");
        require(failures == 2 && contents(inf) != QByteArray(), "save failure cancels export");
        backend.inf_file = inf;

        // Чтение ошибки не должно повторно использовать конфигурацию в памяти.
        require(QFile::rename(inf, inf + ".saved"), "move INF");
        require(QDir().mkdir(inf), "replace INF with directory");
        backend.slot_run_clicked(1, "12:34", "01.01.2030");
        require(failures == 3, "read failure cancels export");
        QDir().rmdir(inf);
        QFile::rename(inf + ".saved", inf);

        // Проверяем немедленный запуск без настоящей службы и повторное использование клиента.
        QLocalServer pipe;
        const QString pipe_name = "exporter-test-" + QString::number(QCoreApplication::applicationPid());
        require(pipe.listen(pipe_name), "test pipe server");
        pipe_client client(pipe_name);
        bool pipe_error = false;
        QObject::connect(&client, &pipe_client::send_failed, [&](const QString&) { pipe_error = true; });
        for (int i = 0; i < 2; ++i) {
            client.send_message_to_server("START_IMMEDIATELY");
            require(wait_for([&]() { return pipe.hasPendingConnections(); }), "reusable pipe connect");
            auto* peer = pipe.nextPendingConnection();
            require(wait_for([&]() { return peer->bytesAvailable() > 0; }), "pipe command delivered");
            require(peer->readAll().startsWith("START_IMMEDIATELY"), "pipe payload");
            require(wait_for([&]() { return peer->state() == QLocalSocket::UnconnectedState; }), "graceful pipe close");
            peer->deleteLater();
        }
        require(!pipe_error, "normal pipe close is not a send failure");

        // Немедленный запуск очищает старое расписание даже при недоступной службе.
        QLocalServer helper_pipe;
        require(helper_pipe.listen("bghelperpipe"), "cannot reserve helper pipe; a real service may be running");
        backend.slot_run_clicked(1, "12:34", "01.01.2030");
        require(backend.read_inf_string("ControlFlags", "Date").isEmpty(), "immediate clears date");
        require(backend.read_inf_string("ControlFlags", "Time").isEmpty(), "immediate clears time");
        require(wait_for([&]() { return helper_pipe.hasPendingConnections(); }), "immediate command connects");
        auto* helper_peer = helper_pipe.nextPendingConnection();
        require(wait_for([&]() { return helper_peer->bytesAvailable() > 0; }), "immediate command delivered");
        require(helper_peer->readAll().startsWith("START_IMMEDIATELY"), "immediate command payload");
        require(wait_for([&]() { return helper_peer->state() == QLocalSocket::UnconnectedState; }), "immediate connection closes");
        helper_peer->deleteLater();
        helper_pipe.close();
        root->setProperty("items_enabled", false);
        backend.slot_run_clicked(1, "12:34", "01.01.2030");
        require(wait_for([&]() { return failures == 4; }), "missing helper reports error");
        require(root->property("items_enabled").toBool(), "pipe error restores controls");
        require(backend.read_inf_string("ControlFlags", "Enabled") == "false", "pipe error disables export");
        require(!backend.str2bool("invalid"), "invalid boolean defaults to false");
        std::puts("PASS: QML validation, atomic INF, TCP framing/UTF-8, completion, pipe reuse, immediate schedule");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL: %s\n", error.what());
        return 1;
    }
}
