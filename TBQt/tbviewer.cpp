#include "SignalHandler.hpp"
#include "TBViewerMainWindow.hpp"
#include "TBQtAppConfig.hpp"

#include <QApplication>
#include <QCommandLineParser>
#include <QFontDatabase>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    // Load IBM Plex Sans fonts
    QFontDatabase::addApplicationFont(":/IBMPlexSans-Bold.otf");
    QFontDatabase::addApplicationFont(":/IBMPlexSans-Regular.otf");

    // set up application and organization info (for QSettings)
    QCoreApplication::setApplicationName(TBQtAppConfig::TBViewer::APPLICATION_NAME);
    QCoreApplication::setApplicationVersion(TBQtAppConfig::APPLICATION_VERSION);
    QCoreApplication::setOrganizationName(TBQtAppConfig::ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(TBQtAppConfig::ORGANIZATION_DOMAIN);

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
    auto* window(new TBViewerMainWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();

    return QApplication::exec();
}
