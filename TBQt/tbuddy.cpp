#include "SignalHandler.hpp"
#include "TBBuddyMainWindow.hpp"
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
    QCoreApplication::setApplicationName(TBQtAppConfig::TBuddy::APPLICATION_NAME);
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
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(a);

    // decide which windows to open
    if(!parser.positionalArguments().empty())
    {
        // open a window for every file passed on the command line
        for(const QString& filename : parser.positionalArguments())
        {
            auto* window(new TBBuddyMainWindow);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->load_document(filename);
            window->show();
        }
    }
    else
    {
        // if no files passed on the command line, open a window without a file
        auto* window(new TBBuddyMainWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    }

    return QApplication::exec();
}
