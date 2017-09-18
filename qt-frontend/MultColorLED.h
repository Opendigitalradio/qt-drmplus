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


#ifndef MULTCOLORLED_H
#define MULTCOLORLED_H

#include <qlabel.h>
#include <qpixmap.h>
#include <qtimer.h>


/* Definitions ****************************************************************/
#define DEFAULT_UPDATE_TIME             600

/* The red and yellow light should be on at least this interval */
#define MIN_TIME_FOR_RED_LIGHT          100


/* Classes ********************************************************************/
class CMultColorLED : public QFrame
{
    Q_OBJECT

public:
    enum ELightColor {RL_GREY, RL_RED, RL_GREEN, RL_YELLOW};

    CMultColorLED(QWidget* parent);
    virtual ~CMultColorLED() {}

    void SetUpdateTime(int);
    void SetLight(ELightColor);
    void Reset();

protected:

    ELightColor     eColorFlag;

    QTimer          TimerRedLight;
    QTimer          TimerGreenLight;
    QTimer          TimerYellowLight;

    bool            bFlagRedLi;
    bool            bFlagGreenLi;
    bool            bFlagYellowLi;

    int             iUpdateTime;

    QColor          green;
    QColor          yellow;
    QColor          red;
    QColor          grey;

    void            UpdateColor();
    void            SetColor(const QColor& color);

protected slots:
    void OnTimerRedLight();
    void OnTimerGreenLight();
    void OnTimerYellowLight();
};

#endif // MULTCOLORLED_H
