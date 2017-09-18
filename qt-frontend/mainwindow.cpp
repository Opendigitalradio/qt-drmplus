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


#include "mainwindow.h"
#include "MultColorLED.h"
#include "ui_DRMMainWindow.h"
#include <QMouseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QMenu>
#include <QSettings>


#include "TechInfoDialog.h"
#include "ui_TechInfoDialog.h"

#include "drmplus.h"
#include "neaacdec.h"

#define VERSION "1.0"
#define REVISION   "$Rev$"

QSettings settings;


DRMMainWindow::DRMMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DRMMainWindow)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint|Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    CurrentVersion = REVISION;
    CurrentVersion.replace(QRegExp("^\\$Rev: (\\d+) \\$$"), "\\1");
    CurrentVersion =  QString(VERSION) + "." + CurrentVersion;

    ui->doubleSpinBox_frequency->setValue(settings.value("freqMHz", "194,064").toDouble());
    setLevel(0);

    connect(ui->button_settings, SIGNAL(clicked(bool)), this, SLOT(showSettings()));

    connect(ui->commandLinkButton_revsearch, SIGNAL(pressed()), this, SLOT(SeekDown()));
    connect(ui->commandLinkButton_fwdsearch, SIGNAL(pressed()), this, SLOT(SeekUp()));
    connect(ui->doubleSpinBox_frequency, SIGNAL(valueChanged(double)), this, SLOT(selectFrequency(double)));

    connect(ui->commandLinkButton_plus, SIGNAL(pressed()), this, SLOT(StepUp()));
    connect(ui->commandLinkButton_minus, SIGNAL(pressed()), this, SLOT(StepDown()));


    upd_counter = 0;
    br_collector = 0;


    graph_spectrum = NULL;
    graph_equalizer = NULL;
    graph_iq = NULL;

    audioBuffer = new RingBuffer<int16_t>(8*32768);
    //outAudio = new audioSink(1, audioBuffer);
    outAudio = new audioSink(1, audioBuffer);
    InOut = new InOutDialog(this);
    connect(InOut, SIGNAL(showFreqButtons()), this, SLOT(showFreqButtons()));
    connect(InOut, SIGNAL(hideFreqButtons()), this, SLOT(hideFreqButtons()));

    TechInfo = new TechInfoDialog(this);




    worker = new DRMWorker(this, audioBuffer);
    connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
    connect(worker, SIGNAL(newAudio(int)), this, SLOT(newAudio(int)));
    connect(worker, SIGNAL(setLevel(int)), this, SLOT(setLevel(int)));
    connect(worker, SIGNAL(setLed(int, int)), this, SLOT(setLed(int, int)));
    connect(worker, SIGNAL(setCodecInfo(int, int, int)), this, SLOT(setCodecInfo(int, int, int)));
    connect(worker, SIGNAL(setText(QString)), ui->TextTextMessage, SLOT(setText(QString)));
    connect(this, SIGNAL(stopWorker()), worker, SLOT(stopRunning()));

    workerThread = new QThread;
    connect(workerThread, SIGNAL(started()), worker, SLOT(process()));
    connect(workerThread, SIGNAL(finished()), worker, SLOT(quit()));
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    worker->moveToThread(workerThread);


    connect(ui->slider_volume, SIGNAL(valueChanged(int)), this, SLOT(newVolume(int)));
#if 1
    ui->ButtonServiceLabel0->setText("[empty 0]");
    ui->ButtonServiceLabel0->setEnabled(false);
    //ui->ButtonServiceLabel0->setChecked(true);
    ui->ButtonServiceLabel1->setText("[empty 1]");
    ui->ButtonServiceLabel1->setEnabled(false);
    ui->ButtonServiceLabel2->setText("[empty 2]");
    ui->ButtonServiceLabel2->setEnabled(false);
    ui->ButtonServiceLabel3->setText("[empty 3]");
    ui->ButtonServiceLabel3->setEnabled(false);
#endif
    workerThread->start();

    ui->slider_volume->setValue(settings.value("volume", 32).toInt());

    this->move(settings.value("pos", QPoint(0,0)).toPoint());

    outAudio->stop();
#if 0
    <------>            audioBuffer -> putDataIntoBuffer (sample_buf, ^M
    <------>                                 2 * (int32_t)KJMP2_SAMPLES_PER_FRAME);
#endif
#if 0
//    ui->CLED_FAC->SetLight(CMultColorLED::RL_GREY);
//    ui->CLED_SDC->SetLight(CMultColorLED::RL_GREY);
//    ui->CLED_MSC->SetLight(CMultColorLED::RL_GREY);
    ui->CLED_FAC->Reset();
    ui->CLED_SDC->Reset();
    ui->CLED_MSC->Reset();
#endif

    ui->ButtonServiceLabel0->installEventFilter(this);
    ui->ButtonServiceLabel1->installEventFilter(this);
    ui->ButtonServiceLabel2->installEventFilter(this);
    ui->ButtonServiceLabel3->installEventFilter(this);
    ui->TextTextMessage->installEventFilter(this);
    installEventFilter(this);
}

void DRMMainWindow::showFreqButtons()
{
    ui->frame_freq_ctrl->setEnabled(true);
}
void DRMMainWindow::hideFreqButtons()
{
    ui->frame_freq_ctrl->setEnabled(false);
}

void DRMMainWindow::StepUp()
{
    //TODO: add autorepeat increasion timer
    double val=ui->doubleSpinBox_frequency->value();
    ui->doubleSpinBox_frequency->setValue(val+0.010);
}

void DRMMainWindow::StepDown()
{
    //TODO: add autorepeat decreasion timer
    double val=ui->doubleSpinBox_frequency->value();
    ui->doubleSpinBox_frequency->setValue(val-0.010);
}


bool DRMMainWindow::setTitleFor(int idx, QString str, bool enabled, bool is_supported, bool was_selected)
{
    bool ret = false;
    switch(idx) {
    case 0:
        ui->ButtonServiceLabel0->setText(enabled ? str : "");
        ui->ButtonServiceLabel0->setCheckable(is_supported);
        ui->ButtonServiceLabel0->setEnabled(enabled && is_supported);
        if(!was_selected && enabled && is_supported) {
            ui->ButtonServiceLabel0->setChecked(true);
            ret=true;
        }
        //ui->ButtonServiceLabel0->setChecked(enabled && is_supported);
        break;
    case 1:
        ui->ButtonServiceLabel1->setText(enabled ? str : "");
        ui->ButtonServiceLabel1->setCheckable(is_supported);
        ui->ButtonServiceLabel1->setEnabled(enabled && is_supported);
        if(!was_selected && enabled && is_supported) {
            ui->ButtonServiceLabel1->setChecked(true);
            ret=true;
        }
        break;
    case 2:
        ui->ButtonServiceLabel2->setText(enabled ? str : "");
        ui->ButtonServiceLabel2->setCheckable(is_supported);
        ui->ButtonServiceLabel2->setEnabled(enabled && is_supported);
        if(!was_selected && enabled && is_supported) {
            ui->ButtonServiceLabel2->setChecked(true);
            ret=true;
        }
        //ui->ButtonServiceLabel2->setChecked(false);
        break;
    case 3:
        ui->ButtonServiceLabel3->setText(enabled ? str : "");
        ui->ButtonServiceLabel3->setCheckable(is_supported);
        ui->ButtonServiceLabel3->setEnabled(enabled && is_supported);
        if(!was_selected && enabled && is_supported) {
            ui->ButtonServiceLabel3->setChecked(true);
            ret=true;
        }
        //ui->ButtonServiceLabel3->setChecked(false);
        break;

    }
    return ret;
}



//Table 51
const char * languages[16] = {
     "No language specified",
     "Arabic",
     "Bengali",
     "Chinese (Mandarin)",
     "Dutch",
     "English",
     "French",
     "German",
     "Hindi",
     "Japanese",
     "Javanese",
     "Korean",
     "Portuguese",
     "Russian",
     "Spanish",
     "Other language"
};


//Table 52
const char * prog_type[32] = {
     "No programme type",
     "News",
     "Current affairs",
     "Information",
     "Sport",
     "Education",
     "Drama",
     "Culture",
     "Science",
     "Varied",
     "Pop Music",
     "Rock Music",
     "Easy Listening Music",
     "Light Classical",
     "Serious Classical",
     "Other Music",
     "Weather/Meteorology",
     "Finance/Business",
     "Children's programmes",
     "Social Affairs",
     "Religion",
     "Phone In",
     "Travel",
     "Leisure",
     "Jazz Music",
     "Country Music",
     "National Music",
     "Oldies Music",
     "Folk Music",
     "Documentary",
     "Not used",
     "Not used - Skip indicator"
};

int DRMMainWindow::fill_service_data(int sid)
{
    qDebug() << __FUNCTION__;
    if(sid >= service_num || services[sid].serviceId == 0)
        goto ret_exit;

    ui->LabelServiceID->setText(QString::asprintf("%d", services[sid].serviceId));
    if(services[sid].language != 15)
        ui->LabelLanguage->setText(languages[services[sid].language]);
    else
        ui->LabelLanguage->setText(QString::asprintf("%c%c%c", services[sid].language2[0], services[sid].language2[1], services[sid].language2[2]).toUpper());

    ui->LabelProgrType->setText(prog_type[services[sid].serviceDesc]);
    ui->LabelCountryCode->setText(QString::asprintf("%c%c", services[sid].country[0], services[sid].country[1]).toUpper());
    ui->TextTextMessage->setText("");
    return 1;
ret_exit:
    ui->LabelLanguage->setText("");
    ui->LabelCountryCode->setText("");
    ui->LabelProgrType->setText("");
    ui->LabelServiceID->setText("");
    ui->TextTextMessage->setText("");
    return 0;
}


void DRMMainWindow::servicelist_update(void *s_p, void *p)
{
    //having 213 carriers
    srv_data_t *srv = (srv_data_t *) s_p;
    mpx_info_t *mpx_info = (mpx_info_t  *) p;

    service_num = mpx_info->service_num_a+mpx_info->service_num_d;
    memcpy(services, srv, sizeof(srv_data_t)*4);

    bool was_selected[4]={0,0,0,0};
    for(int i=0;i<4;i++) {
        //qDebug("SERVICE %d[%d]: %s", i, services[i].serviceId, services[i].label);
        was_selected[i]=setTitleFor(i, QString(srv[i].label), i < service_num ? true : false,
                    srv[i].audioDataFlag==0 && srv[i].audioCoding == 0,
                    was_selected[0]||was_selected[1]||was_selected[2]||was_selected[3]);
        fprintf(stderr, "SERVICE %d[%d]: \"%.*s\" selected:%d\n", i, srv[i].serviceId, 64, srv[i].label, was_selected[i]);
        if(was_selected[i]) {
            fill_service_data(i);
        }
    }

    TechInfo->setStreamsInfo(s_p, p);
}

void DRMMainWindow::setCodecInfo(int bitrate, int chFlags, int sRate)
{
    br_collector+=bitrate;
    if(upd_counter == 20) {

        ui->LabelBitrate->setText(QString("%1 kbps").arg(br_collector/upd_counter));

        if(chFlags == DRMCH_SBR_MONO || chFlags == DRMCH_MONO){
            ui->LabelStereoMono->setText("Mono");
        } else {
            ui->LabelStereoMono->setText("Stereo");
        }

        if(chFlags == DRMCH_SBR_MONO || chFlags == DRMCH_SBR_STEREO){
            ui->LabelCodec->setText("AAC+");
        } else if(chFlags == DRMCH_SBR_PS_STEREO){
            ui->LabelCodec->setText("AAC+v2");
        } else {
            ui->LabelCodec->setText("AAC");
        }
        upd_counter=0;
        br_collector = bitrate;
    } else {
        upd_counter++;
    }
}

void DRMMainWindow::setLevel(int level)
{
    //qDebug() << __FUNCTION__ << ", level:"<<level;
    //0...5
    ui->fivebars->setDisabled(level < 5);
    ui->fourbars->setDisabled(level < 4);
    ui->threebars->setDisabled(level < 3);
    ui->twobars->setDisabled(level < 2);
    ui->twobars->setDisabled(level < 1);
    ui->onebar->setDisabled(level == 0);
}

void DRMMainWindow::setLed(int led, int val)
{
    CMultColorLED::ELightColor val2 = static_cast<CMultColorLED::ELightColor>(val);
    //qDebug() << __FUNCTION__ << "LED:"<<led<<", val:"<<val2;
    switch(led) {
    case 0:
        ui->CLED_FAC->SetLight(val2);
        break;
    case 1:
        ui->CLED_SDC->SetLight(val2);
        break;
    case 2:
        ui->CLED_MSC->SetLight(val2);
        break;
    }
}

void DRMMainWindow::newVolume(int vol)
{
    worker->newVolume(vol);
}

void DRMMainWindow::newAudio(int rate)
{
    outAudio->audioOut(rate);
}

bool DRMMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        mpos = mouseEvent->globalPos() - frameGeometry().topLeft();
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->buttons() == Qt::LeftButton){
            move(mouseEvent->globalPos() - mpos);
            //move(event->globalPos() - mpos);
        }
    }

    return false;
}

void DRMMainWindow::showSettings()
{
    qDebug() << __FUNCTION__;
    QMenu *menu = new QMenu(this);
    QAction *action;

    action = new QAction("In/Out Devices", this);
    action->setIcon(QIcon(":/icons/usb-connector.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowInOutDeviceSelector()));
#if 0
    action = new QAction("Show Freq. Offset", this);
    action->setIcon(QIcon(":/icons/CSSpectrumShiftedPSD.png"));
    action->setData("sp");
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowFrequencyOffsetDiagram()));
#endif
    action = new QAction("Show Input Spectrum", this);
    action->setIcon(QIcon(":/icons/CSSpectrumInpSpectr.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowSpectrumDiagram()));

    action = new QAction("Show Equalizer", this);
    //action->setCheckable(true);
    action->setIcon(QIcon(":/icons/CSSpectrumSNR.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowEqualizerDiagram()));

    action = new QAction("Show I/Q Diagram", this);
    action->setIcon(QIcon(":/icons/CSConstellation.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowIQDiagram()));

    action = new QAction("Technical Info", this);
    action->setIcon(QIcon(":/icons/info.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowTechInfo()));

    action = new QAction("Reset Sync", this);
    action->setIcon(QIcon(":/icons/repeat.png"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ResetSync()));

    action = new QAction("About", this);
    action->setIcon(QIcon(":/icons/question.svg"));
    menu->addAction(action);
    connect(action, SIGNAL(triggered()), this, SLOT(ShowAboutApp()));

    QPoint point = ui->button_settings->pos();
    menu->exec(mapToGlobal(point));
}


DRMMainWindow::~DRMMainWindow()
{
    qDebug() << __FUNCTION__ << ", closing app";
    worker->do_processing = false;
    if(workerThread->isRunning())
        workerThread->exit();

    if(outAudio)
        outAudio->stop();

    if(inputDevice)
        inputDevice->stopReader();

    if(InOut->isVisible())
        InOut->close();
    delete InOut;

    if(TechInfo->isVisible())
        TechInfo->close();
    delete TechInfo;

    if(graph_spectrum) {
        graph_spectrum->close();
        delete graph_spectrum;
    }
    if(graph_equalizer) {
        graph_equalizer->close();
        delete graph_equalizer;
    }
    if(graph_iq) {
        graph_iq->close();
        delete graph_iq;
    }

    settings.setValue("volume", ui->slider_volume->value());
    settings.setValue("pos", this->pos());
    delete ui;
    qDebug() << __FUNCTION__ << ", app closed";
}

void DRMMainWindow::ShowAboutApp()
{
    QMessageBox::about ( this, tr("About Qt-DRM+"),
                         tr("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                            "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
                            "p, li { white-space: pre-wrap; }\n"
                            "</style></head>\n"
                            "<body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
                            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                            "Qt-DRM+ receiving Application.<br />\n"
                            "Version: %1.<br />\n"
                            "Created using Qt5 libraries.<br />\n"
                            "<br />\n"
                            "Credits:<br />\n"
                            "Dream developers for algorithms and code samples<br />\n"
                            "embedded-drm-radio developers for algorithms<br />\n"
                            "Qt-DAB developers for algorithms and OFDM sync samples<br />\n"
                            "Thanks mmbtools forum users for modulated files examples<br />\n"
                            "<br />\n"
                            "&copy; opendigitalradio.org<br />\n"
                            "License: GNU GPLv2<br />\n"
                            "</p></body></html>").arg(CurrentVersion));
}

void DRMMainWindow::GraphClosed(int t)
{
    qDebug() << __FUNCTION__ << t;
    switch(t) {
    case GraphDialog::Spectrum:
        graph_spectrum = NULL;
        break;
    case GraphDialog::Equalizer:
        graph_equalizer = NULL;
        break;
    case GraphDialog::IQDiagram:
        graph_iq = NULL;
        break;
    default:
        qDebug() << "don't know how to close:" << t;
        break;
    }
}


void DRMMainWindow::SeekDown()
{
    qDebug() << __FUNCTION__;
}

void DRMMainWindow::SeekUp()
{
    qDebug() << __FUNCTION__;
}

int DRMMainWindow::selectFrequency(double freqMHz)
{

    if(freqMHz == 0.0)
        freqMHz = ui->doubleSpinBox_frequency->value();

    if(freqMHz < 24.0) {
        freqMHz = 24.0;
        return 0;
    }

    if(freqMHz > 1450.0) {
        freqMHz = 1450.0;
        return 0;
    }


    int32_t	tunedFrequency = 1000000.0 * freqMHz;
    qDebug() << "select Frequency: " << freqMHz << "MHz, " <<tunedFrequency<<" Hz" ;

    outAudio	-> stop ();
    inputDevMutex.lock();
    if(inputDevice) {
        inputDevice		-> stopReader ();
        inputDevice		-> resetBuffer ();
        inputDevice	-> setVFOFrequency (tunedFrequency);
        inputDevice	 -> restartReader ();
    }
    inputDevMutex.unlock();
    settings.setValue ("freqMHz", freqMHz);
    return 1;
}

void DRMMainWindow::ShowTechInfo()
{
    qDebug() << __FUNCTION__;
    TechInfo->show();
}

void DRMMainWindow::ShowInOutDeviceSelector()
{
    qDebug() << __FUNCTION__;
//    int ret = InOut->exec();
//    if(ret == QDialog::Accepted) {
//        qDebug() << "Accepted!!!";
//    }
    InOut->show();
}

void DRMMainWindow::ShowSpectrumDiagram()
{
    if(!graph_spectrum)
        graph_spectrum = new GraphDialog(this, GraphDialog::Spectrum);
    graph_spectrum->show();
}

void DRMMainWindow::ShowEqualizerDiagram()
{
    if(!graph_equalizer)
        graph_equalizer = new GraphDialog(this, GraphDialog::Equalizer);
    graph_equalizer->show();
}

void DRMMainWindow::ShowIQDiagram()
{
    if(!graph_iq)
        graph_iq = new GraphDialog(this, GraphDialog::IQDiagram);
    graph_iq->show();
}

void DRMMainWindow::ResetSync()
{
    qDebug() << __FUNCTION__;
    if(worker)
        worker->ResetSync();
}
