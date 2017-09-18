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


#ifndef INOUTDIALOG_H
#define INOUTDIALOG_H

#include <QDialog>

namespace Ui {
class InOutDialog;
}

class InOutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InOutDialog(QWidget *parent = 0);
    ~InOutDialog();
signals:
    void setNewInputDevice(void *ptr);
    void showFreqButtons();
    void hideFreqButtons();
public slots:
    void selectInDevice (QString s);
    void selectOutDevice(int d);
    void eqChanged(bool);
    void freqEstChanged(bool);
private:
    void setOutputList();

    void resetSelector (void);

    Ui::InOutDialog *ui;
};

#endif // INOUTDIALOG_H
