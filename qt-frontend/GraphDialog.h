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


#ifndef GRAPHDIALOG_H
#define GRAPHDIALOG_H

#include <QDialog>
#include <QtCharts>

namespace Ui {
class GraphDialog;
}

QT_CHARTS_USE_NAMESPACE

class GraphDialog : public QDialog
{
    Q_OBJECT

public:
    enum DiagramType { Spectrum, Equalizer, IQDiagram };
    explicit GraphDialog(QWidget *parent = 0, DiagramType t = Spectrum);
    void newSpectrum(QVector<QVector<QPointF> > *spectrum);
    void newEqualizer(QVector<QVector<QPointF> > *points);
    void newIQ();

    void spectrum_update(void *sp_p, void *);
    void equalizer_update(void *eq_p, void *);
    void iq_update(void *iq_p, void *ct_p);

    ~GraphDialog();
    void reject();

public slots:
    virtual void resizeEvent(QResizeEvent *event);
signals:
    void GraphClose(int t);
private:

    int range_update;
    QVector<QVector<QPointF> > vector_data;
    QVector<QPointF> cells_pil;
    QVector<QPointF> cells_fac_sdc;
    QVector<QPointF> cells_msc;

    DiagramType g_type;
    Ui::GraphDialog *ui;

    QChartView *chartView;
    QChart *chart;
    QAbstractAxis *axisX;
    QAbstractAxis *axisY1;
    QAbstractAxis *axisY2;
};

#endif // GRAPHDIALOG_H
