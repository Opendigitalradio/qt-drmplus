/* 
 * This file is part of the QT-DRM+ distribution 
 * (https://github.com/Opendigitalradio/qt-drmplus).
 * Copyright (c) 2017 OpenDigitalRadio.
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


#ifndef DRMWORKER_H
#define DRMWORKER_H

#include <QObject>
#include <QPointF>
#include <QVector>
#include "ringbuffer.h"

class DRMWorker : public QObject
{
    Q_OBJECT

public:
    void *drmHandle;
    bool do_processing;

    DRMWorker(void *mw, RingBuffer<int16_t> *ob);
    ~DRMWorker();

    int addAacSamples(int16_t *in_buff, int num);
    int initAacDecoder(int cFlags, int aacSR);
    void stream_update(void *aac_p, int callbackType, void *srv_p);
    //void spectrum_update(void *sp_p, int callbackType, void *p);
    //void equalizer_update(void *eq_p, int callbackType, void *p);
    void servicelist_update(void *s_p, int callbackType, void *p);
    void siginfo_update(void *s_p, void *p);
    void text_update(void *text_p, void *p);


public slots:
    void process();
    void stopRunning();
    virtual void quit();
    void newVolume(int vol);
    void gotNewInputDevice(void *ptr);
    void ResetSync();
signals:
    void finished();
    void newAudio(int rate);
    void setLed(int led, int val);
    void setLevel(int level);
    void setCodecInfo(int bitrate, int chFlags, int sRate);
    void setText(QString);

private:
    void *mainwin;
    RingBuffer<int16_t> *outBut;
    float audioVol;
    int audioSampleRate;

    //QVector<QVector<QPointF> > spectrum_data;
    //QVector<QVector<QPointF> > equalizer_data; //interleaved enenrgy/phase vectors.

    int streamToPlay;
    void *hAac;
    int chanFlags;
    int aacSamplingRate;

    int16_t *pcmBuff;
    int pcmSamples;

    bool aac_params_shown;
    int audio_prebuff;
    //friend void strX_callback(void *priv_data, void *aac_p, int callbackType, void *srv_p);
};

#endif // DRMWORKER_H
