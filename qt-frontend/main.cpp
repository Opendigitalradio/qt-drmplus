/*.
 * This file is part of the QT-DRM+ distribution
 * (https://github.com/Opendigitalradio/qt-drmplus)
 * Copyright (c) 2017 OpenDigitalRadio
 *.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>
 */


#include <QApplication>
#include <QCoreApplication>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("OpenDigitalRadio");
    QCoreApplication::setOrganizationDomain("opendigitalradio.org");
    QCoreApplication::setApplicationName("DRMplus");
    QCoreApplication::setApplicationVersion("0.1");

    a.setOrganizationName("OpenDigitalRadio");
    a.setOrganizationDomain("opendigitalradio.org");
    a.setApplicationName("DRMplus");
    a.setApplicationVersion("0.1");

    DRMMainWindow w;
    w.show();

    return a.exec();
}
