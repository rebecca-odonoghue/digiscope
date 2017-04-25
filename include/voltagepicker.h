#ifndef VOLTAGEPICKER_H
#define VOLTAGEPICKER_H

#include <QApplication>
#include <QObject>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_marker.h>

#include <statedefinitions.h>
#include <channelcurve.h>

// Provides functionality to allow points to be selected
// on a ChannelCurve.
class VoltagePicker : public QObject
{
    Q_OBJECT
public:
    explicit VoltagePicker(QwtPlot *plot = 0);
    virtual bool eventFilter( QObject *, QEvent * );
    Channel currentChannel();
    void select( const QPoint &, bool );

private:
    QwtPlot *plot();
    const QwtPlot *plot() const;
    void release();
    void showCursor( bool );
    void shiftPointCursor(bool, int);
    ChannelCurve* selectedCurve_;
    int selectedPoint_;
    QwtPlotMarker* marker_;

signals:
    void pointSelected(int, int);
    void deselected();

public slots:
};

#endif // VOLTAGEPICKER_H
