/*
 * This file is part of the QT-DRM+ distribution
 * (https://github.com/Opendigitalradio/qt-drmplus)
 * Copyright (c) 2017 OpenDigitalRadio
 *
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "GraphDialog.h"
#include "InOutDialog.h"
#include "TechInfoDialog.h"
#include "DRMWorker.h"
#include "ringbuffer.h"
#include "audiosink.h"
#include "virtual-input.h"

#include "drmplus.h"


namespace Ui {
class DRMMainWindow;
}

class DRMMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DRMMainWindow(QWidget *parent = 0);
    ~DRMMainWindow();

    audioSink   *outAudio;
    virtualInput *inputDevice;
    DRMWorker   *worker;
    bool input_is_started;

    InOutDialog *InOut;
    TechInfoDialog *TechInfo;
    QMutex       inputDevMutex;

    GraphDialog *graph_spectrum;
    GraphDialog *graph_equalizer;
    GraphDialog *graph_iq;

    QSettings settings;

    void servicelist_update(void *s_p, void *p);

public slots:
    //UI-related
    //void mousePressEvent(QMouseEvent *event);
    //void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void showSettings();

    void ShowInOutDeviceSelector();
    void ShowSpectrumDiagram();
    void ShowEqualizerDiagram();
    void ShowIQDiagram();
    void ResetSync();
    void ShowTechInfo();
    void ShowAboutApp();
    void GraphClosed(int t);
    void SeekDown();
    void SeekUp();
    void StepDown();
    void StepUp();
    void setLed(int led, int val);
    void setLevel(int level);
    void setCodecInfo(int bitrate, int chFlags, int sRate);
    int selectFrequency( double freq = 0.0);

    int fill_service_data(int sid);

    //processing-related
    void newAudio(int rate);
    void newVolume(int vol);

    void showFreqButtons();
    void hideFreqButtons();
signals:
    void stopWorker();
private:
    Ui::DRMMainWindow *ui;
    QPoint mpos;
    QString CurrentVersion;

    RingBuffer<int16_t>  *audioBuffer;

    QThread     *workerThread;

    int upd_counter;
    int br_collector;
    srv_data_t services[4];
    int service_num;

    bool setTitleFor(int idx, QString str, bool enabled, bool is_supported, bool was_selected);
};

#endif // MAINWINDOW_H

