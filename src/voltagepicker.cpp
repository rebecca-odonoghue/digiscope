#include "voltagepicker.h"

VoltagePicker::VoltagePicker(QwtPlot *plot) : QObject(plot)
{
    selectedCurve_ = NULL;
    selectedPoint_ = -1;
}

Channel VoltagePicker::currentChannel() {
    if (selectedCurve_ == NULL)
        return A;

    return selectedCurve_->channel();
}

// The event filter that is applied to the Plot Canvas to filter
// mouse and key events.
bool VoltagePicker::eventFilter( QObject *object, QEvent *event )
{
    if ( plot() == NULL || object != plot()->canvas() )
        return false;

    switch( event->type() ) {
        case QEvent::MouseButtonRelease:
        {
            const QMouseEvent *mouseEvent = (QMouseEvent*)event;
            select( mouseEvent->pos(), false );
            return true;
        }
        case QEvent::KeyPress:
        {
            const QKeyEvent *keyEvent = (QKeyEvent*)event;

            switch( keyEvent->key() ) {
                case Qt::Key_Up:
                    shiftPointCursor( true, 10 );
                    return true;
                case Qt::Key_Right:
                    shiftPointCursor( true, 1 );
                    return true;
                case Qt::Key_Down:
                    shiftPointCursor( false, 10 );
                    return true;
                case Qt::Key_Left:
                    shiftPointCursor( false, 1 );
                    return true;
            }
        }
        case QEvent::Wheel:
        {
            const QWheelEvent *wheelEvent = (QWheelEvent*)event;

            QPoint numPixels = wheelEvent->pixelDelta();
            QPoint numDegrees = wheelEvent->angleDelta() / 8;

            if (!numPixels.isNull()) {
                shiftPointCursor(true, numPixels.y());
            } else if (!numDegrees.isNull()) {
                QPoint numSteps = numDegrees / 15;
                shiftPointCursor(true, numSteps.y());
            }

            event->accept();
            return true;
        }
        default:
            break;
    }

    return QObject::eventFilter( object, event );
}

// Returns this objects parent Plot.
QwtPlot *VoltagePicker::plot()
{
    return qobject_cast<QwtPlot *>( parent() );
}


// Select the point at a position. If there is no point
// deselect the selected point
void VoltagePicker::select(const QPoint &pos , bool keepCurve)
{
    ChannelCurve *curve = NULL;
    double dist = 10e10;
    int index = -1;


    if (keepCurve && selectedCurve_ != NULL) {
        double newY = plot()->canvasMap(QwtPlot::yLeft).transform(selectedCurve_->invTransform(plot()->canvasMap(QwtPlot::yLeft), pos.y()));
        QPoint scaledPos = QPoint(pos.x(), newY);

        double d;
        int idx = selectedCurve_->closestPoint( scaledPos, &d );
        if ( d < dist )
        {
            curve = selectedCurve_;
            index = idx;
            dist = d;
        }
    } else if (!keepCurve) {
        const QwtPlotItemList& itmList = plot()->itemList();
        for ( QwtPlotItemIterator it = itmList.begin();
            it != itmList.end(); ++it )
        {
            if ( ( *it )->rtti() == QwtPlotItem::Rtti_PlotCurve )
            {
                ChannelCurve *c = static_cast<ChannelCurve *>( *it );

                if (c->isEmpty() || !c->isVisible()) continue;

                double newY = plot()->canvasMap(QwtPlot::yLeft).transform(
                            c->invTransform(
                                plot()->canvasMap(QwtPlot::yLeft), pos.y()));

                QPoint scaledPos = QPoint(pos.x(), newY);

                double d;
                int idx = c->closestPoint( scaledPos, &d );
                if ( d < dist )
                {
                    curve = c;
                    index = idx;
                    dist = d;
                }
            }
        }
    }

    selectedCurve_ = NULL;
    selectedPoint_ = -1;

    if ( dist < 10 ) // 10 pixels tolerance
    {
        selectedCurve_ = curve;
        selectedPoint_ = index;
        showCursor( true );
    } else {
        emit deselected();
    }
}


// Hightlight the selected point
void VoltagePicker::showCursor( bool showIt )
{
    if ( !selectedCurve_ || selectedCurve_->dataSize() == 0 )
        return;

    selectedCurve_->setSelected(true);
    emit pointSelected((int)selectedCurve_->channel(), selectedPoint_);
}

// Select the next/previous neighbour of the selected point
void VoltagePicker::shiftPointCursor( bool up, int steps )
{
    if ( !selectedCurve_ )
        return;
    int index = selectedPoint_ + (( up ? 1 : -1 ) * (steps));
    index = ( index + selectedCurve_->dataSize() ) % selectedCurve_->dataSize();

    if ( index != selectedPoint_ )
    {
        selectedPoint_ = index;
        emit pointSelected((int)selectedCurve_->channel(), selectedPoint_);
    }
}
