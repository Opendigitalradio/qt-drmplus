/*.
 * This file is part of the QT-DRM+ distribution.
 * (https://github.com/Opendigitalradio/qt-drmplus).
 * Copyright (c) 2017 OpenDigitalRadio.
 *.
 * This program is free software: you can redistribute it and/or modify..
 * it under the terms of the GNU General Public License as published by..
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but.
 * WITHOUT ANY WARRANTY; without even the implied warranty of.
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU.
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License.
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef TECHINFODIALOG_H
#define TECHINFODIALOG_H

#include <QDialog>
#include "ui_StationInfo.h"

namespace Ui {
class TechInfoDialog;
}

class TechInfoDialog : public QDialog
{
    Q_OBJECT

public:
    Ui::TechInfoDialog *ui;
    QWidget *tabs[4];
    Ui::StationInfo *stations[4];
    explicit TechInfoDialog(QWidget *parent = 0);
    ~TechInfoDialog();
public slots:
    void setSigInfo(void *s_p);
    void setStreamsInfo(void *s_p, void *m_p);

//private:
};

#endif // TECHINFODIALOG_H
