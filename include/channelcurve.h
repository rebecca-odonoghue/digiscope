#ifndef CHANNELCURVE_H
#define CHANNELCURVE_H

#include <math.h>

#include <QPainter>
#include <QRectF>
#include <QVector>
#include <QDebug>
#include <QtMath>
#include <QObject>
#include <QMutex>

#include <qwt_plot_curve.h>
#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_transform.h>

#include <fftw3.h>

#include <DspFilters/dsp.h>

#include <state.h>
#include <parser.h>

#define MAX_FREQ 20000000

// A Class to manage the processing and plotting of channel voltage data,
// each ChannelCurve instance corresponds to a Digiscope channel, and will
// operate on its own thread.
class ChannelCurve : public QObject, public QwtPlotCurve
{
    Q_OBJECT

public:
    ChannelCurve(Channel channel = A, const QString &title = QString::null,
                 QObject *parent = 0);

    // Methods to transform points between paint co-ordinates and the
    // curve co-ordinates.
    double transform(const QwtScaleMap&, double) const;
    double invTransform(const QwtScaleMap&, double) const;

    // Getter & setter functions.
    void setScale(double);
    Channel channel() const;
    void setSelected(bool);
    void setTimeDiv(double);
    bool selected() const;
    bool isEmpty() const;

private:
    // Private processing functions.
    void setSamples();
    void processSamples();
    void evaluate();
    void filter();
    void measureCurve();
    void findFrequency();
    Parser* parser_;
    Channel channel_;
    bool selected_;
    double voltageDiv_, timeDiv_;
    State state_;
    QMutex mutex_;

public slots:
    // Functions that interact with the GUI thread and process or plot data.
    void process(State);
    void draw(QPainter*, const QwtScaleMap&, const QwtScaleMap&,
              const QRectF&) const;

signals:
    // Functions that interact with the GUI thread to provide processed data.
    void plotReady(int, QVector<QPointF>);
    void measured(int, double, double, double, double, double);
    void freqCalculated(int, double);
    void error(QString);

};

#endif // CHANNELCURVE_H
