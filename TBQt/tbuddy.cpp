#include "TBMainWindow.hpp"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // set up application and organization info (for QSettings)
    QCoreApplication::setApplicationName("tbuddy");
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

    TBMainWindow* window(nullptr);

    // open a window for every file passed on the command line
    for(const QString& filename : parser.positionalArguments())
    {
        window = new TBMainWindow;
        window->load_document(filename);
        window->show();
    }

    // if no files passed on the command line, open a window without a file
    if(window == nullptr)
    {
        window = new TBMainWindow;
        window->show();
    }

    return a.exec();
}
