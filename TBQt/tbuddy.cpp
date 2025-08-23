#include "TBBuddyMainWindow.hpp"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // set up application and organization info (for QSettings)
    QCoreApplication::setApplicationName("Poker Buddy");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("hdnastudio");
    QCoreApplication::setOrganizationDomain("hdnastudio.com");

    // set up command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file(s) to open.");
    parser.process(a);

    // decide which windows to open
    if(parser.positionalArguments().size() > 0)
    {
        // open a window for every file passed on the command line
        for(const QString& filename : parser.positionalArguments())
        {
            auto window(new TBBuddyMainWindow);
            window->setAttribute(Qt::WA_DeleteOnClose);
            window->load_document(filename);
            window->show();
        }
    }
    else
    {
        // if no files passed on the command line, open a window without a file
        auto window(new TBBuddyMainWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->show();
    }

    return a.exec();
}
