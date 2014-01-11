#include <QApplication>

#include <QCoreApplication>
#include <QTranslator>
#include <QDir>
#include <cstring>
#include <QDateTime>

#include "mainwindow.h"
#include "settings.h"
#include "banpair.h"
#include "server.h"
#include "audio.h"

#if defined(WIN32) && defined(VS2010)
#include "breakpad/client/windows/handler/exception_handler.h"

using namespace google_breakpad;

static bool callback(const wchar_t *dump_path, const wchar_t *id,
                     void *context, EXCEPTION_POINTERS *exinfo,
                     MDRawAssertionInfo *assertion,
                     bool succeeded) {
    if (succeeded)
        qWarning("Dump file created in %s, dump guid is %ws\n", dump_path, id);
    else
        qWarning("Dump failed\n");
    return succeeded;
}

int main(int argc, char *argv[]) {
    ExceptionHandler eh(L"./dmp", NULL, callback, NULL,
                        ExceptionHandler::HANDLER_ALL);
#else
int main(int argc, char *argv[]) {
#endif
    if (argc > 1 && strcmp(argv[1], "-server") == 0) {
        new QCoreApplication(argc, argv);
    } else {
        new QApplication(argc, argv);
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
    }

#ifdef Q_OS_MAC
#ifdef QT_NO_DEBUG
    QDir::setCurrent(qApp->applicationDirPath());
#endif
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    qApp->installTranslator(&qt_translator);
    qApp->installTranslator(&translator);

    Sanguosha = new Engine;
    Config.init();
    qApp->setFont(Config.AppFont);
    BanPair::loadBanPairs();

    if (qApp->arguments().contains("-server")) {
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

    QFile file("qss/sanguosha.qss");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }

#ifdef AUDIO_SUPPORT
    Audio::init();
#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    foreach (QString arg, qApp->arguments()) {
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostAddress", arg);

            main_window->startConnection();
            break;
        }
    }

    return qApp->exec();
}

