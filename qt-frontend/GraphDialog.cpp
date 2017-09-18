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
#include "GraphDialog.h"
#include "ui_GraphDialog.h"


#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLogValueAxis>

#include "mainwindow.h"
#include "DRMWorker.h"
#include "drmplus.h"

QT_CHARTS_USE_NAMESPACE


extern "C" void spectrum_callback(void *priv_data, void *sp_p, int callbackType, void *p)
{
    if(priv_data && callbackType == SIGSP_CALLBACK)
        static_cast<GraphDialog *>(priv_data)->spectrum_update(sp_p, p);
}

extern "C" void equalizer_callback(void *priv_data, void *eq_p, int callbackType, void *p)
{
    if(priv_data && callbackType == SIGEQ_CALLBACK)
        static_cast<GraphDialog *>(priv_data)->equalizer_update(eq_p, p);
}

extern "C" void iq_callback(void *priv_data, void *eq_p, int callbackType, void *p)
{
    if(priv_data && callbackType == SIGIQ_CALLBACK)
        static_cast<GraphDialog *>(priv_data)->iq_update(eq_p, p);
}




void GraphDialog::iq_update(void *iq_p, void *ct_p)
{
    double *iq = (double *) iq_p;
    uint8_t *markers = (uint8_t *) ct_p;

    if(!iq || !markers) return;

    //qDebug() << __FUNCTION__ << markers[213];

    for(int i=0;i<213;i++) {
        switch(markers[i]) {
        case CELL_TYPE_PIL:
        case CELL_TYPE_PIL2X:
        case CELL_TYPE_AFS:
            cells_pil.append(QPointF(iq[i*2], iq[i*2+1]));
            break;
        case CELL_TYPE_FAC:
        case CELL_TYPE_SDC:
            cells_fac_sdc.append(QPointF(iq[i*2], iq[i*2+1]));
            break;

        case CELL_TYPE_MSC:
            cells_msc.append(QPointF(iq[i*2], iq[i*2+1]));
            break;
        case CELL_TYPE_SDC_OR_MSC:
        case CELL_TYPE_AFS_OR_MSC:
        case CELL_TYPE_NONE:
        default:
            continue;
            break;
        }
    }

    if(markers[213] == 39 && markers[214] == 3) {
        //if(vector_data.size() == 40)
            newIQ();
        cells_pil.clear();
        cells_fac_sdc.clear();
        cells_msc.clear();
    }
}



void GraphDialog::equalizer_update(void *eq_p, void *)
{
    //having 213 carriers
     double *iq = (double *) eq_p;

    QVector<QPointF> energyVector;
    QVector<QPointF> phaseVector;
    //energyVector.reserve(213+1);
    //phaseVector.reserve(213+1);

     double minEn = 1000000.0;
     double maxEn = 0.0000001;
     double minPh = 1000000.0;
     double maxPh = -1000000.0;
     for(int i=0;i<213;i++) {
         double energy = sqrt(iq[i*2]*iq[i*2] + iq[i*2+1]*iq[i*2+1]);
         double phase = atan2(iq[i*2+1], iq[i*2]);
         energyVector.append(QPointF((double)(i-106)*(10.0/9 * 400), energy));
         phaseVector.append(QPointF((double)(i-106)*(10.0/9 * 400), phase));
         if(energy < minEn) minEn = energy;
         if(energy > maxEn) maxEn = energy;
         if(phase < minPh) minPh = phase;
         if(phase > maxPh) maxPh = phase;
     }
     energyVector.append(QPointF(minEn,maxEn));
     phaseVector.append(QPointF(minPh,maxPh));

     vector_data.append(energyVector);
     vector_data.append(phaseVector);
     if(vector_data.size() == 40*2) {
         newEqualizer(&vector_data);
         vector_data.clear();
     }
}


void GraphDialog::spectrum_update(void *sp_p, void *)
{
    //qDebug() << __FUNCTION__;
    double *spectrum = (double *) sp_p;

    QVector<QPointF> spectrumVector;
    //spectrumVector.reserve(432+1);

    double min = 10000000;
    double max = 0.00000001;
     for(int i=0;i<432;i++) {
         double val = sqrt(spectrum[i]);
         spectrumVector.append(QPointF((double)(i-432/2)*(10.0/9 * 400), val));
         if(val < min) min=val;
         if(val > max) max=val;
     }
     spectrumVector.append(QPointF(min, max));

     vector_data.append(spectrumVector);
     if(vector_data.size() == 40) {
         newSpectrum(&vector_data);
         vector_data.clear();
     }
}






GraphDialog::GraphDialog(QWidget *parent, DiagramType t) :
    QDialog(parent),
    g_type(t),
    ui(new Ui::GraphDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(GraphClose(int)), parent, SLOT(GraphClosed(int)));

    DRMMainWindow *drm_p = static_cast<DRMMainWindow *>(parent);

    axisX=NULL;
    axisY1=NULL;
    axisY2=NULL;
    range_update=0;

    chart = new QChart();
    chart->setBackgroundBrush(QBrush("#1F2F2F"));
    chart->setTitleBrush(QBrush("white"));
    //chart->legend()->hide();

    //axisX->setGridLinePen(QPen(QBrush("#008080", Qt::Dense1Pattern), 1));

    switch(g_type) {
    case Spectrum: {
        axisX = new QValueAxis;
        //axisX->setRange(0,432);
        axisX->setRange(-432.0/2.0*(10.0/9 * 400),432.0/2.0*(10.0/9 * 400));
        axisX->setGridLineColor("#4F5F5F");
        axisX->setTitleText("frequency");
        axisX->setTitleBrush(QBrush("#AADDDD"));

        QLogValueAxis * axisYlog = new QLogValueAxis;
        axisYlog->setRange(0.1, 1.0);
        axisYlog->setLabelFormat("%g");
        axisYlog->setBase(10.0);
        axisYlog->setMinorTickCount(-1);
        axisYlog->setMinorGridLineColor("#3F4F4F");
        //shared params
        axisYlog->setGridLineColor("#4F5F5F");
        axisYlog->setTitleText("energy");
        axisYlog->setTitleBrush(QBrush("#AADDDD"));
        axisY1 = axisYlog;

        //axisX->setMinorTickCount(5);

        chart->setTitle("Input Signal Spectrum");
        chart->legend()->hide();

        vector_data.reserve(40);
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGSP_CALLBACK, spectrum_callback, this);
    }
        break;
    case Equalizer: {
        axisX = new QValueAxis;
        axisX->setRange(-106.0*(10.0/9 * 400),107.0*(10.0/9 * 400));
        axisX->setGridLineColor("#4F5F5F");
        axisX->setTitleText("frequency");
        axisX->setTitleBrush(QBrush("#AADDDD"));


#if 1
        QLogValueAxis * axisYlog = new QLogValueAxis;
        axisYlog->setRange(0.01, 2.0);
        axisYlog->setLabelFormat("%g");
        axisYlog->setBase(10.0);
        axisYlog->setMinorTickCount(-1);
        axisYlog->setMinorGridLineColor("#3F4F4F");
#else
        QValueAxis * axisYlog = new QValueAxis;
        axisYlog->setRange(0.005, 1.0);
        axisYlog->setLabelFormat("%g");
#endif
        axisYlog->setGridLineColor("#4F5F5F");
        axisYlog->setTitleText("energy");
        axisYlog->setTitleBrush(QBrush("#AADDDD"));
        axisY1 = axisYlog;

        axisY2 = new QValueAxis;
        axisY2->setRange(-M_PI, M_PI);
        axisY2->setGridLineColor("#6F8F8F");
        axisY2->setLabelsColor("#C4B4C4");
        axisY2->setTitleBrush(QBrush("#AADDDD"));
        axisY2->setTitleText("phase");


        chart->setTitle("Equalizer Amplitude/Phase Correction function");
        chart->legend()->hide();

        vector_data.reserve(80);
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGEQ_CALLBACK, equalizer_callback, this);


    }
        break;
    case IQDiagram: {
        //axisX = new QValueAxis;
        QValueAxis * axisXlog = new QValueAxis;
        axisXlog->setMinorTickCount(0.2);
        axisXlog->setRange(-1.2,1.2);
        axisXlog->setMinorGridLineColor("#3F4F4F");
        axisX = axisXlog;
        axisX->setGridLineColor("#4F5F5F");
        axisX->setTitleText("I");
        axisX->setTitleBrush(QBrush("#AADDDD"));

        QValueAxis * axisYlog = new QValueAxis;
        axisYlog->setMinorTickCount(0.2);
        axisYlog->setRange(-1.2,1.2);
        axisXlog->setMinorGridLineColor("#3F4F4F");
        axisY1 = axisYlog;
        axisY1->setGridLineColor("#4F5F5F");
        axisY1->setTitleText("Q");
        axisY1->setTitleBrush(QBrush("#AADDDD"));

        //axisX->setMinorTickCount(5);

        chart->setTitle("FAC/SDC/MSC I/Q Constellation");
        //chart->legend()->hide();
        chart->legend()->setColor("AADDDD");

        vector_data.reserve(CELL_TYPE_ENDLIST);
#if 0
        vector_data.at(CELL_TYPE_NONE).reserve(213*40);
        vector_data[CELL_TYPE_PIL].reserve(213*40);
        vector_data[CELL_TYPE_PIL2X].reserve(213*40);
        vector_data[CELL_TYPE_AFS].reserve(213*40);
        vector_data[CELL_TYPE_FAC].reserve(213*40);
        vector_data[CELL_TYPE_SDC].reserve(213*40);
        vector_data[CELL_TYPE_MSC].reserve(213*40);
        vector_data[CELL_TYPE_SDC_OR_MSC].reserve(213*40);
        vector_data[CELL_TYPE_AFS_OR_MSC].reserve(213*40);
#endif
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGIQ_CALLBACK, iq_callback, this);
    }
        break;
    default:
        axisY1 = new QValueAxis;
        chart->setTitle("Unknown");
        break;
    }

    axisX->setLabelsColor("#C4C4C4");
    axisY1->setLabelsColor("#C4C4C4");

    if(g_type== Equalizer) {
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY1, Qt::AlignLeft);
        chart->addAxis(axisY2, Qt::AlignRight);
    } else {
        chart->addAxis(axisX, Qt::AlignBottom);
        chart->addAxis(axisY1, Qt::AlignLeft);
    }

    chartView = new QChartView(chart, this);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->resize(this->size());
    //chartView->setSizePolicy(QSizePolicy::Preferred);

    //this->resize(300, 300);
    //this->
    //ui->centralwidget->
    //window.setCentralWidget(chartView);
    setWindowTitle(chart->title());
}

void GraphDialog::newEqualizer(QVector<QVector<QPointF> > *points)
{
    QList<QAbstractSeries *> series = chart->series();

    //qDebug() << __FUNCTION__ << points->size() << "/" << series.size();

    for(int i=0; i< points->size(); i++) {
        QLineSeries *simpleGLSeries=NULL;
        if(series.size() <= i) {
            simpleGLSeries = new QLineSeries();
            simpleGLSeries->setPointLabelsVisible(false);
            simpleGLSeries->setUseOpenGL(true);
            simpleGLSeries->replace(points->at(i).mid(0, 213));
            chart->addSeries(simpleGLSeries);
            if(i%2==0) {
                simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1FF%1").arg(230 - 5*(i/2), 2, 16, QLatin1Char('0')))), 1));
                simpleGLSeries->attachAxis(axisY1);
            } else {
                simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1%1FF").arg(230 - 5*(i/2), 2, 16, QLatin1Char('0')))), 1));
                simpleGLSeries->attachAxis(axisY2);
            }
            simpleGLSeries->attachAxis(axisX);
        } else {
            simpleGLSeries=static_cast<QLineSeries *>(series.at(i));
            simpleGLSeries->replace(points->at(i).mid(0, 213));
        }


        if(i%2==0) {
            QLogValueAxis * axisYlog = static_cast<QLogValueAxis *> (axisY1);
            QPointF minmax = points->at(i).at(213);
            if(minmax.rx() < axisYlog->min()) {
                axisYlog->setMin(minmax.rx());
                range_update=0;
            } else
                range_update++;
            if(minmax.ry() > axisYlog->max()) {
                axisYlog->setMax(minmax.ry());
                range_update=0;
            } else
                range_update++;

            if(range_update == 1000) {
#ifdef AXIS_UPDATE
                qDebug() << "Axis updated!";
                axisYlog->setMin(minmax.rx());
                axisYlog->setMax(minmax.ry());
#endif
                range_update=0;
            }
        } else {
            QValueAxis * axisYlog = static_cast<QValueAxis *> (axisY2);
            QPointF minmax = points->at(i).at(213);
            if(minmax.rx() < axisYlog->min())
                axisYlog->setMin(minmax.rx());
            if(minmax.ry() > axisYlog->max())
                axisYlog->setMax(minmax.ry());
        }
    }
}

void GraphDialog::newSpectrum(QVector<QVector<QPointF> > *spectrum)
{
    QList<QAbstractSeries *> series = chart->series();
    for(int i=0; i< spectrum->size(); i++) {
        QLineSeries *simpleGLSeries=NULL;
        if(series.size() <= i) {
            simpleGLSeries = new QLineSeries();
            simpleGLSeries->setPointLabelsVisible(false);
            simpleGLSeries->setUseOpenGL(true);
            simpleGLSeries->replace(spectrum->at(i).mid(0, 432));
            simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1FF%1").arg(230 - 5*i, 2, 16, QLatin1Char('0')))), 1));
            chart->addSeries(simpleGLSeries);
            simpleGLSeries->attachAxis(axisX);
            simpleGLSeries->attachAxis(axisY1);
        } else {
            simpleGLSeries=static_cast<QLineSeries *>(series.at(i));
            simpleGLSeries->replace(spectrum->at(i).mid(0, 432));
        }

        //tune min/max values of graph if needed.
        QLogValueAxis * axisYlog = static_cast<QLogValueAxis *> (axisY1);
        QPointF minmax = spectrum->at(i).at(432);

        if(minmax.rx() < axisYlog->min()) {
            axisYlog->setMin(minmax.rx());
            range_update=0;
        } else
            range_update++;

        if(minmax.ry() > axisYlog->max()) {
            axisYlog->setMax(minmax.ry());
            range_update=0;
        } else
            range_update++;

        if(range_update == 1000) {
#ifdef AXIS_UPDATE
            qDebug() << "Axis updated?!";
            axisYlog->setMin(minmax.rx());
            axisYlog->setMax(minmax.ry());
#endif
            range_update=0;
        }

    }
}



//QScatterSeries *series0 = new QScatterSeries();
//series0->setName("scatter1");
//series0->setMarkerShape(QScatterSeries::MarkerShapeCircle);
//series0->setMarkerSize(15.0);

//QScatterSeries *series1 = new QScatterSeries();
//series1->setName("scatter2");
//series1->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
//series1->setMarkerSize(20.0);

//QScatterSeries *series2 = new QScatterSeries();
//series2->setName("scatter3");
//series2->setMarkerShape(QScatterSeries::MarkerShapeRectangle);
//series2->setMarkerSize(30.0);


void GraphDialog::newIQ()
{
    QList<QAbstractSeries *> series = chart->series();
    QScatterSeries *simpleGLSeries;

    //qDebug() << __FUNCTION__ << points->size() << "/" << series.size();
    if(series.size() == 0) {        
        simpleGLSeries = new QScatterSeries();
        simpleGLSeries->setPointLabelsVisible(true);
        simpleGLSeries->setUseOpenGL(true);
        simpleGLSeries->setName("pilots/afs");
        //simpleGLSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        simpleGLSeries->setMarkerSize(1.0);
        chart->addSeries(simpleGLSeries);
        //simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1FF%1").arg(230 - 5*(i/2), 2, 16, QLatin1Char('0')))), 1));
        simpleGLSeries->attachAxis(axisY1);
        simpleGLSeries->attachAxis(axisX);
        simpleGLSeries->replace(cells_pil);

        simpleGLSeries = new QScatterSeries();
        simpleGLSeries->setPointLabelsVisible(true);
        simpleGLSeries->setUseOpenGL(true);
        simpleGLSeries->setName("FAC/SDC");
        //simpleGLSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        simpleGLSeries->setMarkerSize(1.0);
        chart->addSeries(simpleGLSeries);
        //simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1FF%1").arg(230 - 5*(i/2), 2, 16, QLatin1Char('0')))), 1));
        simpleGLSeries->attachAxis(axisY1);
        simpleGLSeries->attachAxis(axisX);
        simpleGLSeries->replace(cells_fac_sdc);

        simpleGLSeries = new QScatterSeries();
        simpleGLSeries->setPointLabelsVisible(true);
        simpleGLSeries->setUseOpenGL(true);
        simpleGLSeries->setName("MSC");
        //simpleGLSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        simpleGLSeries->setMarkerSize(1.0);
        chart->addSeries(simpleGLSeries);
        //simpleGLSeries->setPen(QPen(QBrush(QColor(QString("#%1FF%1").arg(230 - 5*(i/2), 2, 16, QLatin1Char('0')))), 1));
        simpleGLSeries->attachAxis(axisY1);
        simpleGLSeries->attachAxis(axisX);
        simpleGLSeries->replace(cells_msc);
    } else {
        simpleGLSeries=static_cast<QScatterSeries *>(series.at(0));
        simpleGLSeries->replace(cells_pil);
        simpleGLSeries=static_cast<QScatterSeries *>(series.at(1));
        simpleGLSeries->replace(cells_fac_sdc);
        simpleGLSeries=static_cast<QScatterSeries *>(series.at(2));
        simpleGLSeries->replace(cells_msc);
    }
}


void GraphDialog::resizeEvent(QResizeEvent *event)
{
    qDebug() << __FUNCTION__ << event;
    chartView->resize(this->size());
}

void GraphDialog::reject()
{
    DRMMainWindow *drm_p = static_cast<DRMMainWindow *>(parent());

    qDebug() << __FUNCTION__;



    switch(g_type) {
    case Spectrum: {
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGSP_CALLBACK, NULL, NULL);
    }
        break;
    case Equalizer: {
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGEQ_CALLBACK, NULL, NULL);
        break;
    }
    case IQDiagram: {
        if(drm_p->worker->drmHandle)
            drmplusSetCallback(drm_p->worker->drmHandle, SIGIQ_CALLBACK, NULL, NULL);
        break;
    }
    default:
        break;
    }

    emit GraphClose(g_type);
    QDialog::reject();
    deleteLater();
}

GraphDialog::~GraphDialog()
{
    qDebug() << __FUNCTION__;
    delete ui;
}
