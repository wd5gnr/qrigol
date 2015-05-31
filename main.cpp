#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationDomain("awce.com");
    QCoreApplication::setOrganizationName("awce");
    QCoreApplication::setApplicationName("QRigol");
    MainWindow w;
    w.show();

    return a.exec();
}
