#include <QtGui/QApplication>
#include "vfkmainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VfkMainWindow w;
    w.show();

    return a.exec();
}
