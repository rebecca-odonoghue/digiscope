#ifndef PLOT_H
#define PLOT_H

#include <cmath>

#include <QPalette>
#include <QBrush>
#include <QList>
#include <QTimer>
#include <QLabel>
#include <QHBoxLayout>
#include <QThread>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_layout.h>
#include <qwt_painter.h>
#include <qwt_plot_directpainter.h>
#include <qwt_symbol.h>

#include <channelcurve.h>
#include <state.h>
#include <voltagepicker.h>

QString valueToUnits(double);

// Class that manages all plotting and interfaces between the channel curves
// and the main window.
class Plot : public QwtPlot
{
    Q_OBJECT

public:
    Plot(State* state = NULL, QWidget* = NULL);
    void setCurveVisible(Channel, bool);

    // Pass in widgets to update.
    void setLegend(QFrame*);
    void setMarkerPlot(QwtPlot*);
    void setEquationLabel(QLabel*);
    void calculateFilter();
    void exit();

private:
    void styleCanvas();
    void scaleTriggerPlot(double);
    void selectClosePoint();
    State* state_;
    ChannelCurve *channelA_, *channelB_, *channelF_, *channelM_;
    QwtPlotMarker *triggerThreshold_, *pickedVoltage_;
    QLabel *hDivLabel_, *aDivLabel_, *bDivLabel_, *fDivLabel_, *mDivLabel_;
    QwtPlot *markerPlot_;
    QwtPlotMarker *aOffsetMarker_, *bOffsetMarker_, *tMarker_, *y0Marker_;
    QLabel* equationLabel_;
    VoltagePicker* picker_;
    QwtPlot* freqPlot_;
    QThread *aThread_, *bThread_, *fThread_, *mThread_;


public slots:
    // Slots connected to widget signals in MainWindow.
    void scaleA(int);
    void scaleB(int);
    void scaleF(int);
    void scaleM(int);
    void scaleH(int);
    double changeTrigger();
    void solveEquation(QString);
    void plotAcquisition(int);
    void plotReady(int, QVector<QPointF>);

private slots:
    // Slots connected to local signals.
    void hideTrigger();
    void catchError(QString);
    void pointSelected(int, int);
    void pointDeselected();

signals:
    // Signals emitted to make changes in MainWindow.
    void error(QString);
    void newMeasurements(int, double, double, double, double, double);
    void newFrequency(int, double);
    void channelHidden(int);

    // Functions called to plot a new curve using a copy of the current state.
    void processA(State);
    void processB(State);
    void processF(State);
    void processM(State);
};

#endif // PLOT_H
