#include "TBViewerMainWindow.hpp"
#include "SignalHandler.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Load IBM Plex Sans fonts
    QFontDatabase::addApplicationFont(":/IBMPlexSans-Bold.otf");
    QFontDatabase::addApplicationFont(":/IBMPlexSans-Regular.otf");

    // set up application and organization info (for QSettings)
    QCoreApplication::setApplicationName("Poker Remote");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("hdnastudio");
    QCoreApplication::setOrganizationDomain("hdnastudio.com");

    // set up signal handler for graceful shutdown
    SignalHandler signalHandler;
    QObject::connect(&signalHandler, &SignalHandler::shutdownRequested, &a, &QApplication::closeAllWindows);

    // set up command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);

    // open a single viewer window (no file arguments for viewer)
    auto window(new TBViewerMainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();

    return a.exec();
}
