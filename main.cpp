/*
 *
 *  This program is Copyright (c) 2015 by Al Williams al.williams@awce.com
 *  All rights reserved.
 *
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

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
