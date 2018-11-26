#include "TBMainWindow.hpp"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TBMainWindow w;
    w.show();

    return a.exec();
}
