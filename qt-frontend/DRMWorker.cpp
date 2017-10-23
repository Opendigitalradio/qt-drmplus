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


#include <QDebug>
#include <QThread>
#include <QTime>
#include <QFileDialog>
#include <QSettings>

#include <unistd.h>
#include <stdio.h>
#include	<sys/time.h>
#include	<time.h>


#include "DRMWorker.h"

#include "drmplus.h"
#include "neaacdec.h"

#include "MultColorLED.h"

#include "mainwindow.h"
#include "TechInfoDialog.h"


//#define DUMP_PCM

#ifdef DUMP_PCM
static FILE *pcm_file = NULL;
#endif

//extern QSettings settings;

extern "C" void stream_callback(void *priv_data, void *aac_p, int callbackType, void *srv_p)
{
    if(priv_data && callbackType >= STREAM0_CALLBACK && callbackType <= STREAM3_CALLBACK)
        static_cast<DRMWorker *>(priv_data)->stream_update(aac_p, callbackType, srv_p);
}


extern "C" void servicelist_callback(void *priv_data, void *eq_p, int callbackType, void *p)
{
    if(priv_data && callbackType == STRLIST_CALLBACK)
        static_cast<DRMMainWindow *>(priv_data)->servicelist_update(eq_p, p);
}

extern "C" void text_callback(void *priv_data, void *text_p, int callbackType, void *p)
{
    if(priv_data && callbackType >= TEXT0_CALLBACK && callbackType <= TEXT3_CALLBACK)
        static_cast<DRMWorker *>(priv_data)->text_update(text_p, p);
}


extern "C" void siginfo_callback(void *priv_data, void *sp_p, int callbackType, void *p)
{
    if(priv_data && callbackType == SIGINFO_CALLBACK)
        static_cast<DRMWorker *>(priv_data)->siginfo_update(sp_p, p);
}


void DRMWorker::text_update(void *text_p, void *p)
{
    char *text = (char *) text_p;
    if(text) {
        //qDebug("New text: \"%s\"", text);
        emit setText(QString(text));
    }
}

void DRMWorker::siginfo_update(void *s_p, void *p)
{
    //having 213 carriers
    siginfo_data_t *sig = (siginfo_data_t *) s_p;
    DRMMainWindow *mw = static_cast<DRMMainWindow *>(mainwin);


    emit setLed(0, sig->fac_crc_ok ? CMultColorLED::RL_GREEN : CMultColorLED::RL_YELLOW);
    emit setLed(1, sig->sdc_crc_ok ? CMultColorLED::RL_GREEN : CMultColorLED::RL_YELLOW);

    double level_dB = 0.0;
    if(sig->snr_noise)
        level_dB = 10*log10(sig->snr_signal/sig->snr_noise);


    if(level_dB > 30.0)
        emit setLevel(5);
    else if(level_dB > 25.0)
        emit setLevel(4);
    else if(level_dB > 20.0)
        emit setLevel(3);
    else if(level_dB > 15.0)
        emit setLevel(3);
    else if(level_dB > 10.0)
        emit setLevel(2);
    else if(level_dB > 5.0)
        emit setLevel(1);
    else
        emit setLevel(0);
#if 0
    qDebug("siginfo: state:%d  freq:%.1f signal: %.1f dB fac|sdc: [%d|%d]",
           sig->sync_state,
           192000.0*sig->dc_freq_coarse/432.0 + sig->dc_freq_fine, level_dB, sig->fac_crc_ok, sig->sdc_crc_ok);
#endif
    if(mw->TechInfo->isVisible())
        mw->TechInfo->setSigInfo(sig);

}

void DRMWorker::stream_update(void *aac_p, int callbackType, void *srv_p)
{
    uint8_t *aacFrame = (uint8_t *) aac_p;
    srv_data_t *srv = (srv_data_t *) srv_p;

    //qDebug() << __FUNCTION__;
    if(streamToPlay != srv->streamId) {
        qDebug("this callback is for other stream %d != %d!", streamToPlay, srv->streamId);
        return;
    }

    if(!srv || srv->aacFrameLen <=0) return;

    //fprintf(stderr, "Got STREAM[%d] %d bytes\n", callbackType-STREAM0_CALLBACK, srv->aacFrameLen);
#if 0
    fprintf(stderr, "%02x|", aacFrame[0]);
    int i;
    for(i=1;i<srv->aacFrameLen;i++)
        fprintf(stderr, "%02x", aacFrame[i]);
    fprintf(stderr, "\n");
#endif

    int newChanFlags=0;
    switch(srv->audioMode) {
    case 0:
        newChanFlags = srv->sbrFlag ? DRMCH_SBR_MONO : DRMCH_MONO;
        break;
    case 1:
        newChanFlags = DRMCH_SBR_PS_STEREO;
        break;
    case 2:
        newChanFlags = srv->sbrFlag ? DRMCH_SBR_STEREO : DRMCH_STEREO;
        break;
    }

    bool is_new=false;
    if(newChanFlags != chanFlags || srv->audioSamplingRate != aacSamplingRate) {
        if(hAac) NeAACDecClose(hAac);
        hAac=NULL;
        qDebug() << "New AAC channels flags: "<<newChanFlags;
        char initErr = NeAACDecInitDRM(&hAac, (srv->audioSamplingRate == 3) ? 24000 : 48000, newChanFlags);
        if (initErr != 0) {
            hAac=NULL;
            fprintf(stderr, "NeAACDecInitDRM() returned error: %d\n", initErr);
        } else {
           chanFlags = newChanFlags;
           aacSamplingRate = srv->audioSamplingRate;
           aac_params_shown=false;
           audio_prebuff=0;

           DRMMainWindow *p = static_cast<DRMMainWindow *>(mainwin);
           p->outAudio->restart();


#ifdef DUMP_PCM
           if(pcm_file)
               fclose(pcm_file);
           pcm_file = fopen("audiodump.pcm","wb");
#endif
        }
        is_new=true;
    }

    //int16_t buff[3072*2];
    NeAACDecFrameInfo hInfo;
    hInfo.error=1;
    int16_t *audioSamplesTemp=NULL;

    if(aacFrame) {
        audioSamplesTemp=(int16_t*) NeAACDecDecode(hAac, &hInfo, aacFrame, srv->aacFrameLen);
        if(is_new || hInfo.error)
            fprintf(stderr,"FAAD error: %d samples:%lu, sbr:%d, ps:%d, ch:%d sr:%lu bytes_left:%lu\n",
            hInfo.error, hInfo.samples, hInfo.sbr, hInfo.ps, hInfo.channels, hInfo.samplerate, srv->aacFrameLen - hInfo.bytesconsumed);
    }

    addAacSamples(audioSamplesTemp, hInfo.samples);

    if(hInfo.error) {
        emit setLed(2, aacFrame ? CMultColorLED::RL_YELLOW :  CMultColorLED::RL_RED);
    } else {
        emit setLed(2, CMultColorLED::RL_GREEN);
        if(!aac_params_shown && hInfo.samples) {
             int num_aac_frames = (srv->audioSamplingRate == 3) ? 5 : 10;
             int br = srv->aacFrameLen * 8 * num_aac_frames * 5 / 1000;

             emit setCodecInfo(br, chanFlags, aacSamplingRate);
             //aac_params_shown=true;
        }
    }

}



DRMWorker::DRMWorker(void *mw, RingBuffer<int16_t> *ob) :
    mainwin(mw),
    outBut(ob)
{
    DRMMainWindow *p = static_cast<DRMMainWindow *>(mainwin);
    qDebug() << __FUNCTION__;
    do_processing=true;
    audioVol = 0.5;
    audioSampleRate = 48000;
    streamToPlay = 0;
    pcmBuff = NULL;
    pcmSamples = 0;
    hAac = NULL;
    aac_params_shown=true;
    audio_prebuff=0;

    chanFlags=-1;
    aacSamplingRate=-1;

    drmHandle = drmplusOpen();
    if(!drmHandle)
        return;

    drmplusConfiguration *cfg = drmplusGetConfiguration(drmHandle);
    cfg->callOnEveryFACandSDC = 1;
    cfg->equalizerType = p->settings.value("equalizer", "1").toInt();
    cfg->ifoEstimationType = p->settings.value("ifoestimation", "0").toInt();
    int ret = 0;
    if((ret = drmplusSetConfiguration(drmHandle, cfg)) == DRMPLUS_ERR) {
        qDebug() << "setting cfg failed:" << ret;
        return;
    }

    //drmplusSetCallback(drmHandle, SIGSP_CALLBACK, spectrum_callback, this);
    //drmplusSetCallback(drmHandle, SIGEQ_CALLBACK, equalizer_callback, this);

    drmplusSetCallback(drmHandle, SIGINFO_CALLBACK, siginfo_callback, this);
    drmplusSetCallback(drmHandle, STRLIST_CALLBACK, servicelist_callback, mainwin);

#if 0
    drmplusSetCallback(drmHandle, SIGSP_CALLBACK, spectrum_update, NULL);
    drmplusSetCallback(drmHandle, SIGIQ_CALLBACK, iq_update, NULL);
    drmplusSetCallback(drmHandle, SIGEQ_CALLBACK, equalizer_update, NULL);
    drmplusSetCallback(drmHandle, FAC_CALLBACK, fac_update, NULL);
    drmplusSetCallback(drmHandle, SDC_CALLBACK, sdc_update, NULL);
    drmplusSetCallback(drmHandle, MSC_CALLBACK, msc_update, NULL);
#endif

    drmplusSetCallback(drmHandle, STREAM0_CALLBACK+streamToPlay, stream_callback, this);
    drmplusSetCallback(drmHandle, TEXT0_CALLBACK+streamToPlay, text_callback, this);
//    drmplusSetCallback(drmHandle, STREAM1_CALLBACK, stream_callback, this);
//    drmplusSetCallback(drmHandle, STREAM2_CALLBACK, stream_callback, this);
//    drmplusSetCallback(drmHandle, STREAM3_CALLBACK, stream_callback, this);
}

DRMWorker::~DRMWorker()
{
    qDebug() << __FUNCTION__;
}

void DRMWorker::quit()
{
    qDebug() << __FUNCTION__;
    do_processing=false;
}

void DRMWorker::newVolume(int newVal)
{
    //64 controls
    audioVol = (newVal*newVal)/(63.0*63.0);
    //qDebug() << __FUNCTION__ << "new:" << newVal << "a_vol:" << audioVol;
}

int DRMWorker::addAacSamples(int16_t *in_buff, int samples)
{
    //qDebug() << __FUNCTION__ << "buff:" << in_buff << "samples: " << samples;
    //no free space for this frame
    if(outBut->WriteSpace() < samples) {
        fprintf(stderr,"X");
        return 0;
    }

    float aVol=audioVol;
    //we have AAC error, produce echo...
    if(!in_buff || !samples) {
       if(!pcmBuff || !pcmSamples)
           return 0; //too early
       qDebug() << __FUNCTION__ << " Adding echo!";
       aVol=0.7;
    } else {
        if(pcmSamples != samples) {
            if(pcmBuff)
                pcmBuff = (int16_t *)realloc(pcmBuff, sizeof(int16_t)*samples);
            else
                pcmBuff = (int16_t *)malloc(sizeof(int16_t)*samples);
            pcmSamples=samples;
        }        
        memcpy(pcmBuff, in_buff, pcmSamples*sizeof(int16_t));
    }

    //change sound volume
    for(int i=0;i<pcmSamples;i++)
        pcmBuff[i] = (((float)pcmBuff[i]) * aVol);


    outBut->putDataIntoBuffer(pcmBuff, pcmSamples);
#ifdef DUMP_PCM
    fwrite(pcmBuff, sizeof(int16_t), pcmSamples, pcm_file);
#endif


//    if(audio_prebuff < 10) {
//        audio_prebuff++;
//        return 1;
//    }

    if (outBut->GetRingBufferReadAvailable() > audioSampleRate/8)
        emit newAudio(audioSampleRate);
//    else
//        fprintf(stderr,"Q");
    return 1;
}



static inline
int64_t		getMyTime	(void) {
struct timeval	tv;

    gettimeofday (&tv, NULL);
    return ((int64_t)tv. tv_sec * 1000000 + (int64_t)tv. tv_usec);
}


void DRMWorker::gotNewInputDevice(void *ptr)
{

}

void DRMWorker::ResetSync()
{
    qDebug() << __FUNCTION__;
    drmplusResetSync(drmHandle, 0);
}

void DRMWorker::process()
{
    DRMMainWindow *p = static_cast<DRMMainWindow *>(mainwin);

    qDebug() << __FUNCTION__;
    int processed=0;

#if 0
    //FILE *iq_f = fopen("samples/01_drm_e_192k_v2.iq192", "rb");
    FILE *iq_f = fopen("samples/02_drm_testeE.iq192", "rb");
    //FILE *iq_f = fopen("samples/03_spark_drm_e_iq_0_mode.iq192", "rb");
    int16_t IQ_data[2*480];

    int64_t nextStop = getMyTime();
    int64_t period = 1000000 * 480/192000; //in microseconds
    // do something..

    while(iq_f && do_processing) {

//        if(outBut->GetRingBufferWriteAvailable() < 1920*2) {
//            fprintf(stderr, "X");
//            QThread::usleep(2500);
//            continue;
//        }


        nextStop += period;
        int readed = fread(IQ_data, 2, 2*480, iq_f);
        if(readed < 2*480) {
            fseek(iq_f, 0, SEEK_SET);
            continue;
            //break;
        }

        if(drmplusAddSamples(drmHandle, IQ_data, 480)!=DRMPLUS_OK) {
            qDebug() << "decoding failed";
            break;
        }
        processed++;
        if (nextStop - getMyTime () > 0)
           usleep (nextStop - getMyTime ());
    }
#else
    double IQ_data[2*480];
    DSPCOMPLEX IQ_data2[480];

    int64_t nextStop = getMyTime();
    int64_t period = 1000000 * 480/192000; //in microseconds

    p->input_is_started = false;

    // do something..
    while(do_processing) {
        nextStop += period;

        p->inputDevMutex.lock();
        if(p->inputDevice) {
            if(!p->input_is_started) {
                p->inputDevice->resetBuffer();
                p->inputDevice->restartReader();
                p->input_is_started=true;
            }

            if(p->inputDevice->Samples() < 480) {
                //qDebug("have %d/%d samples!", p->inputDevice->Samples(), 480);
                p->inputDevMutex.unlock();
                usleep (100);
                continue;
            }

            //qDebug("have %d samples", p->inputDevice->Samples());
            int num = p->inputDevice->getSamples(IQ_data2, 480);
            if(num != 480) {
                p->inputDevMutex.unlock();
                if(num) qDebug("received small amount of data: %d/%d!", num, 480);
                usleep (100);
                continue;
            }

            for(int i=0;i<480;i++) {
                IQ_data[2*i] = IQ_data2[i].real();
                IQ_data[2*i+1] = IQ_data2[i].imag();
            }

            //TODO: move baseband + resample!!!

        } else {
            //memset(IQ_data, 0, 2*480*sizeof(double));
            //qDebug("do nothing");
            p->input_is_started = false;
        }
        p->inputDevMutex.unlock();

        if(p->input_is_started) {
            if(drmplusAddSamplesDouble(drmHandle, IQ_data, 480)!=DRMPLUS_OK) {
                qDebug() << "decoding failed";
                break;
            }
            processed++;
        } else {
            if (nextStop - getMyTime () > 0)
                usleep (nextStop - getMyTime ());
        }

    }
#endif

    qDebug() << "thread ended, processed:" << processed;
}


void DRMWorker::stopRunning()
{
    qDebug() << __FUNCTION__;
    do_processing=false;
    return ;
}
