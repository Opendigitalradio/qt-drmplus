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

#include "TechInfoDialog.h"
#include "ui_TechInfoDialog.h"
#include "ui_StationInfo.h"

#include "drmplus.h"

TechInfoDialog::TechInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TechInfoDialog)
{
    ui->setupUi(this);

    for(int i=0;i<4;i++) {
        tabs[i] = new QWidget();
        ui->tabWidget->addTab(tabs[i], QString::asprintf("Station%d", i));
        stations[i] = new Ui::StationInfo;
        stations[i]->setupUi(tabs[i]);
        if(i==0)
            ui->tabWidget->setCurrentWidget(tabs[i]);
        else
            ui->tabWidget->setTabEnabled(i, false);
    }
}


const char protLevelsQAM4[4][4] = {"1/4", "1/3", "2/5", "1/2"};
const char protLevelsQAM16[4][10] = {"1/6 + 1/2", "1/4 + 4/7", "1/3 + 2/3", "1/2 + 3/4"};
const char audioCodingStr[4][40]={"AAC","reserved","reserved","xHE-AAC"};
const char audioModeAacStr[4][50]={"mono","P. stereo", "stereo", "reserved"};
const char audioSamplingRateStrAAC[8][40]={"reserved","12 kHz","reserved","24 kHz","reserved","48 kHz","reserved","reserved"};
const char audioSamplingRateStrXHEAC[8][40]={"9.6 kHz","12 kHz","16 kHz","19.2 kHz","24 kHz","32 kHz","38.4 kHz","48 kHz"};
extern const char * languages[16];
extern const char * prog_type[32];

void TechInfoDialog::setStreamsInfo(void *s_p, void *m_p)
{
    srv_data_t *srv = (srv_data_t *) s_p;
    mpx_info_t *mpx_info = (mpx_info_t  *) m_p;
    qDebug() << __FUNCTION__;

    switch(mpx_info->msc_mode) {
    case 0:
        ui->label_modulation->setText("QAM-16");
        break;
    case 3:
        ui->label_modulation->setText("QAM-4");
        break;
    default:
        ui->label_modulation->setText(QString::asprintf("Unknown:%d", mpx_info->msc_mode));
        break;
    }

    ui->label_FACRATE->setText("1/4");

    ui->label_SDCRATE->setText(mpx_info->sdc_mode ? "1/4" : "1/2");

    if(mpx_info->msc_mode==3) {
        ui->label_MSC_A_RATE->setText(protLevelsQAM4[mpx_info->prot_A]);
        ui->label_MSC_B_RATE->setText(protLevelsQAM4[mpx_info->prot_B]);
    } else if(mpx_info->msc_mode==0) {
        ui->label_MSC_A_RATE->setText(protLevelsQAM16[mpx_info->prot_A]);
        ui->label_MSC_B_RATE->setText(protLevelsQAM16[mpx_info->prot_B]);
    }

    ui->label_MSC_A_LEN->setText(QString::asprintf("%d", mpx_info->length_A[0]+mpx_info->length_A[1]+mpx_info->length_A[2]+mpx_info->length_A[3]));
    ui->label_MSC_B_LEN->setText(QString::asprintf("%d", mpx_info->length_B[0]+mpx_info->length_B[1]+mpx_info->length_B[2]+mpx_info->length_B[3]));

    ui->label_audio_num->setText(QString::asprintf("%d", mpx_info->service_num_a));
    ui->label_data_num->setText(QString::asprintf("%d" , mpx_info->service_num_d));
    ui->label_time->setText(QString::asprintf("%02d:%02d %02ld.%02ld.%04ld", mpx_info->Hours, mpx_info->Minutes, mpx_info->Day, mpx_info->Month, mpx_info->Year));

    int num_srv = mpx_info->service_num_a + mpx_info->service_num_d;

    for(int i=0;i<4;i++) {
        ui->tabWidget->setTabEnabled(i, i < num_srv ? true : false);
        if(i >= mpx_info->service_num_a+mpx_info->service_num_d)
            continue;
        //QWidget *station = ui->tabWidget->widget(i);
        QWidget *station = ui->tabWidget->widget(i);
        qDebug()<< "Widget name:" << station->objectName();
        stations[i]->label_label->setText(srv[i].label);
        stations[i]->label_short_id->setText(QString::asprintf("%d", srv[i].shortId));
        stations[i]->label_service_id->setText(QString::asprintf("%d", srv[i].serviceId));
        stations[i]->label_stream_id->setText(QString::asprintf("%d", srv[i].streamId));

        stations[i]->label_length_A->setText(QString::asprintf("%d Bytes", mpx_info->length_A[i]));
        stations[i]->label_length_B->setText(QString::asprintf("%d Bytes", mpx_info->length_B[i]));
        stations[i]->label_audio_ca->setText(srv[i].audioCaFlag ? "true" : "false");
        stations[i]->label_data_ca->setText(srv[i].dataCaFlag ? "true" : "false");
        stations[i]->label_audio_data->setText(srv[i].audioDataFlag ? "Data" : "Audio");

        if(srv[i].language != 15)
            stations[i]->label_language->setText(QString(languages[srv[i].language]));
        else
            stations[i]->label_language->setText(QString::asprintf("%c%c%c", srv[i].language2[0], srv[i].language2[1], srv[i].language2[2]).toUpper());

        stations[i]->label_prog_type->setText(prog_type[srv[i].serviceDesc]);
        stations[i]->label_audio_coding->setText(audioCodingStr[srv[i].audioCoding]);
        stations[i]->label_sbr_flag->setText(srv[i].sbrFlag ? "true" : "false");
        stations[i]->label_audio_mode->setText(audioModeAacStr[srv[i].audioMode]);
        switch(srv[i].audioCoding) {
        case 0:
            stations[i]->label_samplerate->setText(audioSamplingRateStrAAC[srv[i].audioSamplingRate]);
            break;
        case 3:
            stations[i]->label_samplerate->setText(audioSamplingRateStrXHEAC[srv[i].audioSamplingRate]);
            break;
        default:
            stations[i]->label_samplerate->setText("unknown coding");
        }
        stations[i]->label_text->setText(srv[i].textFlag ? "true" : "false");
        stations[i]->label_enchanced->setText(srv[i].enhancementFlag ? "true" : "false");
        stations[i]->label_coder->setText(QString::asprintf("%d", srv[i].extradataLen));

        QString s;
        for(int j=0;j<srv[i].extradataLen;j++)
            s.append(QString::number(srv[i].extradata[j], 16).toUpper());

        stations[i]->label_codec->setText(s);
    }

}

void TechInfoDialog::setSigInfo(void *s_p)
{
    siginfo_data_t *sig = (siginfo_data_t *) s_p;
    //qDebug() << __FUNCTION__;

    double freqOffset = 192000.0*sig->dc_freq_coarse/432.0 + sig->dc_freq_fine;
    double level_dB = 0.0;
    if(sig->snr_noise  > 0)
        level_dB = 10*log10(sig->snr_signal/sig->snr_noise);

    ui->label_freq_offset->setText(QString::asprintf("%.1f Hz", freqOffset));
    ui->label_CNR->setText(QString::asprintf("%.1f dB", level_dB));
    ui->label_time_offset->setText(QString::asprintf("%d", sig->fine_timeshift));
    ui->label_signal_level->setText(QString::asprintf("%.3f", sig->snr_signal));
    ui->label_noise_level->setText(QString::asprintf("%.3f", sig->snr_noise));
    ui->label_inverted->setText( sig->spectrum_inverted ? "true" : "false");

    ui->label_MERAVG->setText("Not Yet Implemented");
    ui->label_MERPEAK->setText("Not Yet Implemented");

    if(sig->sync_state < SYNC_STATE_FTO_TRY)
        ui->label_sync->setText("none");
    else if(sig->sync_state <= SYNC_STATE_FTO_DONE)
        ui->label_sync->setText("fine time sync");
    else if(sig->sync_state <= SYNC_STATE_IFO_DONE)
        ui->label_sync->setText("int. freq. sync");
    else
        ui->label_sync->setText("sync done");

    //TODO: stations list update...
}

TechInfoDialog::~TechInfoDialog()
{
    for(int i=0;i<4;i++)
        delete stations[i];

    delete ui;
}

