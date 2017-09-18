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


#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "InOutDialog.h"
#include "ui_InOutDialog.h"

#include "audiosink.h"
#include "virtual-input.h"
#include "mainwindow.h"

#include "drmplus.h"

#include	"rawfiles.h"
#include	"wavfiles.h"
#ifdef	HAVE_RTLSDR
#include	"rtlsdr-handler.h"
#endif
#ifdef	HAVE_SDRPLAY
#include	"sdrplay-handler.h"
#endif
#ifdef	HAVE_ELAD_S1
#include	"elad-handler.h"
#endif
#ifdef	__MINGW32__
#ifdef	HAVE_EXTIO
#include	"extio-handler.h"
#endif
#endif
#ifdef	HAVE_RTL_TCP
#include	"rtl_tcp_client.h"
#endif
#ifdef	HAVE_AIRSPY
#include	"airspy-handler.h"
#endif

//extern QSettings settings;

InOutDialog::InOutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InOutDialog)
{
    ui->setupUi(this);

    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());

    setOutputList();

    p->inputDevice = NULL;
    ui->comboBox_in	-> addItem ("Select device");
    ui->comboBox_in	-> addItem ("file input (.iq/.raw)");
    ui->comboBox_in	-> addItem ("file input (.wav)");
/**
  *	Devices can be included or excluded, setting is in the configuration
  *	files. Inclusion is reflected in the selector on the GUI.
  *	Note that HAVE_EXTIO is only useful for Windows
  */
#ifdef	HAVE_SDRPLAY
    ui->comboBox_in	-> addItem ("sdrplay");
#endif
#ifdef	HAVE_RTLSDR
    ui->comboBox_in	-> addItem ("dabstick");
#endif
#ifdef	HAVE_AIRSPY
    ui->comboBox_in	-> addItem ("airspy");
#endif
#ifdef	HAVE_ELAD_S1
    ui->comboBox_in	-> addItem ("elad-s1");
#endif
#ifdef  HAVE_EXTIO
    ui->comboBox_in	-> addItem ("extio");
#endif
#ifdef	HAVE_RTL_TCP
    ui->comboBox_in	-> addItem ("rtl_tcp");
#endif


    QString h = p->settings.value ("device", "no device"). toString ();
    int k		= ui->comboBox_in -> findText (h);
    if (k != -1) {
       ui->comboBox_in	-> setCurrentIndex (k);
       selectInDevice 		(ui->comboBox_in 	-> currentText ());
    }

    connect (ui->comboBox_in, SIGNAL (activated (const QString &)),
                  this, SLOT (selectInDevice (const QString &)));

    switch(p->settings.value("equalizer", "1").toInt())
    {
    case EQ_TYPE_SIMPLE:
        ui->radioButton_si->setChecked(true);
        break;
    case EQ_TYPE_LIN_INTERP:
        ui->radioButton_li->setChecked(true);
        break;
    case EQ_TYPE_LIN_INTERP_MEM:
        ui->radioButton_li_2->setChecked(true);
        break;
    default:
        ui->radioButton_li->setChecked(true);
        break;
    }


    connect (ui->radioButton_li, SIGNAL(clicked(bool)), this ,SLOT(eqChanged(bool)));
    connect (ui->radioButton_li_2, SIGNAL(clicked(bool)), this ,SLOT(eqChanged(bool)));
    connect (ui->radioButton_si, SIGNAL(clicked(bool)), this ,SLOT(eqChanged(bool)));
    connect (ui->radioButton_fe_maxnrg, SIGNAL(clicked(bool)), this ,SLOT(freqEstChanged(bool)));
    connect (ui->radioButton_fe_4syms, SIGNAL(clicked(bool)), this ,SLOT(freqEstChanged(bool)));

}

InOutDialog::~InOutDialog()
{
    delete ui;
}


void InOutDialog::freqEstChanged(bool)
{

    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());
    qDebug() << __FUNCTION__;
    drmplusConfiguration *cfg = drmplusGetConfiguration(p->worker->drmHandle);
    p->inputDevMutex.lock();
    if(ui->radioButton_fe_maxnrg->isChecked()) {
        qDebug() << "freqEst: maxenergy";
        cfg->ifoEstimationType = IFO_EST_MAXENERGY;
    } else if(ui->radioButton_fe_4syms->isChecked()) {
        qDebug() << "freqEst: 4symbold";
        cfg->ifoEstimationType = IFO_EST_4SYMBOLS;
    } else {
        cfg->ifoEstimationType = IFO_EST_MAXENERGY;
    }

    drmplusSetConfiguration(p->worker->drmHandle, cfg);
    p->inputDevMutex.unlock();
    p->settings.setValue("ifoestimation", cfg->ifoEstimationType);

}

void InOutDialog::eqChanged(bool)
{
    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());
    qDebug() << __FUNCTION__;
    drmplusConfiguration *cfg = drmplusGetConfiguration(p->worker->drmHandle);
    p->inputDevMutex.lock();
    if(ui->radioButton_li->isChecked()) {
        qDebug() << "eq: lin";
        cfg->equalizerType = EQ_TYPE_LIN_INTERP;
    } else if(ui->radioButton_li_2->isChecked()) {
        qDebug() << "eq: lin+mem";
        cfg->equalizerType = EQ_TYPE_LIN_INTERP_MEM;
    } else if(ui->radioButton_si->isChecked()) {
        qDebug() << "eq: simple";
        cfg->equalizerType = EQ_TYPE_SIMPLE;
    }
    drmplusSetConfiguration(p->worker->drmHandle, cfg);
    p->inputDevMutex.unlock();
    p->settings.setValue("equalizer", cfg->equalizerType);
}

void InOutDialog::setOutputList()
{
    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());

    p->outAudio->setupChannels(ui->comboBox_out);
    p->outAudio->stop();

    bool err;
    QString h=p->settings.value("audiosink", "default").toString();
    qDebug() << __FUNCTION__ << "output to: " <<  h;
    int k = ui->comboBox_out -> findText (h);
    if (k != -1) {
       ui->comboBox_out -> setCurrentIndex (k);
       err = !p->outAudio->selectDevice (k);
    }
#if 1
    if ((k == -1) || err) {
       qDebug() << __FUNCTION__ << "GOT ERROR! output to default";
       p->outAudio->selectDefaultDevice ();
    }
#endif
    p->outAudio->restart();
    connect (ui->comboBox_out, SIGNAL (activated (int)),
             this,  SLOT (selectOutDevice(int)));
}

void InOutDialog::selectOutDevice(int d) {
    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());
    qDebug() << __FUNCTION__ << d;
    p->outAudio->stop();
    p->outAudio->selectDevice(d);
    p->outAudio->restart();

    QString s = ui->comboBox_out->currentText();
    p->settings.setValue("audiosink", s);
}


/**	In case selection of a device did not work out for whatever
  *	reason, the device selector is reset to "no device"
  *	Qt will trigger on the change of value in the deviceSelector
  *	which will cause selectdevice to be called again (while we
  *	are in the middle, so we first disconnect the selector
  *	from the slot. Obviously, after setting the index of
  *	the device selector, we connect again
  */
void	InOutDialog::resetSelector (void) {
    disconnect (ui->comboBox_in, SIGNAL (activated (const QString &)),
                this, SLOT (selectInDevice (const QString &)));
    int	k	= ui->comboBox_in -> findText (QString ("Select device"));
    if (k != -1) { 		// should always happen
       ui->comboBox_in -> setCurrentIndex (k);
    }
    fprintf (stderr, "deviceSelector is reset %d\n", k);
    connect (ui->comboBox_in, SIGNAL (activated (const QString &)),
             this, SLOT (selectInDevice (const QString &)));
}

//
//	setDevice is called from the GUI. Other GUI's might have a preselected
//	single device to go with, then if suffices to extract some
//	code specific to that device
void	InOutDialog::selectInDevice (QString s)
{
    DRMMainWindow *p = static_cast<DRMMainWindow *>(this->parent());
    qDebug() << __FUNCTION__ << "selecting to: " << s;
    QString	file;


    p->inputDevMutex.lock();
    //p->outAudio->stop();
    if(p->inputDevice) {
        //virtualInput *id = p->inputDevice;
        //p->inputDevice = NULL;
        //id	-> stopReader ();
        //id-> resetBuffer ();
        //delete	id;
         p->inputDevice		-> stopReader ();
         p->inputDevice		-> resetBuffer ();
         delete p->inputDevice;
         p->inputDevice=NULL;
         p->input_is_started=false;
    }

///	OK, everything quiet, now let us see what to do
#ifdef	HAVE_AIRSPY
    if (s == "airspy") {
       try {
          p->inputDevice	= new airspyHandler (&p->settings);
          showButtons ();
          selectChannel	(channelSelector -> currentText ());
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                                   tr ("Airspy or Airspy mini not found\n"));
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else
#endif
#ifdef HAVE_EXTIO
//	extio is - in its current settings - for Windows, it is a
//	wrap around the dll
    if (s == "extio") {
       try {
          p->inputDevice = new extioHandler (&p->settings);
          showButtons ();
          selectChannel (channelSelector -> currentText() );
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                                tr ("extio: no luck\n") );
          p->inputDevice = new virtualInput();
          resetSelector ();
       }
    }
    else
#endif
#ifdef HAVE_RTL_TCP
//	RTL_TCP might be working.
    if (s == "rtl_tcp") {
       try {
          p->inputDevice = new rtl_tcp_client (&p->settings);
          showButtons ();
          selectChannel (channelSelector -> currentText() );
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                               tr ("rtl_tcp: no luck\n") );
          p->inputDevice = new virtualInput();
          resetSelector ();
       }
    }
    else
#endif
#ifdef	HAVE_SDRPLAY
    if (s == "sdrplay") {
       try {
          p->inputDevice	= new sdrplayHandler (&p->settings);
          showButtons ();
          selectChannel	(channelSelector -> currentText ());
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                                   tr ("SDRplay: no library or device\n"));
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else
#endif
#ifdef	HAVE_ELAD_S1
    if (s == "elad-s1") {
       try {
          p->inputDevice	= new eladHandler (&p->settings);
          showButtons ();
          selectChannel	(channelSelector -> currentText ());
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                                   tr ("elad-s1: no library or device\n"));
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else
#endif
#ifdef	HAVE_RTLSDR
    if (s == "dabstick") {
       try {
          p->inputDevice	= new rtlsdrHandler (&p->settings, ui->frame_output);
          emit showFreqButtons ();
          p->selectFrequency();
       }
       catch (int e) {
          QMessageBox::warning (this, tr ("Warning"),
                                   tr ("DAB stick not found! Please use one with RTL2832U or similar chipset!\n"));
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else
#endif
//
//	We always have fileinput!!
    if (s == "file input (.iq/.raw)") {
       file		= QFileDialog::getOpenFileName (this,
                                                    tr ("Open file ..."),
                                                    QDir::homePath (),
                                                    tr ("iq data (*.raw, *.iq, *iq192)"));
       file		= QDir::toNativeSeparators (file);
       try {
          p->inputDevice	= new rawFiles (file, ui->frame_output);
          emit hideFreqButtons ();
       }
       catch (int e) {
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else
    if (s == "file input (.wav)") {
       file		= QFileDialog::getOpenFileName (this,
                                                    tr ("Open file ..."),
                                                    QDir::homePath (),
                                                    tr ("WAV data (*.wav)"));
       file		= QDir::toNativeSeparators (file);
       try {
          p->inputDevice	= new wavFiles (file, ui->frame_output);
          emit hideFreqButtons ();
       }
       catch (int e) {
          p->inputDevice = new virtualInput ();
          resetSelector ();
       }
    }
    else {	// s == "no device"
//	and as default option, we have a "no device"
       p->inputDevice	= new virtualInput ();
    }
    //emit setNewInputDevice((void *)inputDevice);
    //settings.setValue ("device", s);
    p->input_is_started=false;
    p->outAudio->restart();
    p->inputDevMutex.unlock();

}
