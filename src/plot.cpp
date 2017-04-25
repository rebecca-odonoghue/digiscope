#include "plot.h"
#include <QDebug>

QString valueToUnits(double value) {
    if (value == 0.0) {
        return QString::number(value);
    } else if (qAbs(value) < 0.001) {
        return QString::number(value*1000000.0) + "μ";
    } else if (qAbs(value) < 1) {
        return QString::number(value*1000.0) + "m";
    } else if (qAbs(value) < 1000) {
        return QString::number(value);
    } else if (qAbs(value) < 1000000) {
        return QString::number(value / 1000.0) + "k";
    } else {
        return QString::number(value / 1000000.0) + "M";
    }
}

Plot::Plot(State *state, QWidget *parent) : QwtPlot(parent)
{
    canvas()->setFocusPolicy(Qt::ClickFocus);

    state_ = state;

    //QThread* mathThread = new QThread();
    //mathThread->start();

    //mathPlotter_ = new MathPlotter();
    //mathPlotter_->moveToThread(mathThread);
//    filterPlotter_ = new FilterPlotter(state_);
    //acquisitionPlotter_ = new AcquisitionPlotter(channelA_, channelB_, state_, this);
//    QObject::connect(mathPlotter_, &MathPlotter::plotReady, this, &Plot::plotMathCurve);
//    QObject::connect(mathPlotter_, &MathPlotter::error, this, &Plot::catchError);
//    QObject::connect(filterPlotter_, &FilterPlotter::plotReady, this, &Plot::plotFilterCurve);
//    QObject::connect(acquisitionPlotter_, &AcquisitionPlotter::plotReady, this, &Plot::plotAcquisition);

    // Style the appearance of the general plot
    styleCanvas();

    picker_ = new VoltagePicker(this);
    canvas()->installEventFilter(picker_);
    QObject::connect(picker_, &VoltagePicker::pointSelected, this, &Plot::pointSelected);
    QObject::connect(picker_, &VoltagePicker::deselected, this, &Plot::pointDeselected);

    // Initialise the curve for each channel
    channelA_ = new ChannelCurve(A, "Channel A");
    channelB_ = new ChannelCurve(B, "Channel B");
    channelF_ = new ChannelCurve(F, "Filter Channel");
    channelM_ = new ChannelCurve(M, "Math Channel");

    // Disable Auto-scaling
    channelA_->setItemAttribute(QwtPlotItem::AutoScale, false);
    channelB_->setItemAttribute(QwtPlotItem::AutoScale, false);
    channelF_->setItemAttribute(QwtPlotItem::AutoScale, false);
    channelM_->setItemAttribute(QwtPlotItem::AutoScale, false);

    channelA_->setPen(Qt::yellow, 2.0);
    channelA_->attach(this);

    channelB_->setPen(Qt::green, 2.0);
    channelB_->attach(this);

    channelM_->setPen(Qt::magenta, 2.0);
    channelM_->setVisible(false);
    channelM_->attach(this);

    channelF_->setPen(Qt::cyan, 2.0);
    channelF_->setVisible(false);
    channelF_->attach(this);

    aThread_ = new QThread(this);
    channelA_->moveToThread(aThread_);
    aThread_->start();

    bThread_ = new QThread(this);
    channelB_->moveToThread(bThread_);
    bThread_->start();

    fThread_ = new QThread(this);
    channelF_->moveToThread(fThread_);
    fThread_->start();

    mThread_ = new QThread(this);
    channelM_->moveToThread(mThread_);
    mThread_->start();

    QObject::connect(this, &Plot::processA, channelA_, &ChannelCurve::process);
    QObject::connect(channelA_, &ChannelCurve::plotReady, this, &Plot::plotReady);
    QObject::connect(channelA_, &ChannelCurve::measured, this, &Plot::newMeasurements);
    QObject::connect(channelA_, &ChannelCurve::freqCalculated, this, &Plot::newFrequency);

    QObject::connect(this, &Plot::processB, channelB_, &ChannelCurve::process);
    QObject::connect(channelB_, &ChannelCurve::plotReady, this, &Plot::plotReady);
    QObject::connect(channelB_, &ChannelCurve::measured, this, &Plot::newMeasurements);
    QObject::connect(channelB_, &ChannelCurve::freqCalculated, this, &Plot::newFrequency);

    QObject::connect(this, &Plot::processF, channelF_, &ChannelCurve::process);
    QObject::connect(channelF_, &ChannelCurve::plotReady, this, &Plot::plotReady);
    QObject::connect(channelF_, &ChannelCurve::measured, this, &Plot::newMeasurements);
    QObject::connect(channelF_, &ChannelCurve::freqCalculated, this, &Plot::newFrequency);

    QObject::connect(this, &Plot::processM, channelM_, &ChannelCurve::process);
    QObject::connect(channelM_, &ChannelCurve::plotReady, this, &Plot::plotReady);
    QObject::connect(channelM_, &ChannelCurve::measured, this, &Plot::newMeasurements);
    QObject::connect(channelM_, &ChannelCurve::freqCalculated, this, &Plot::newFrequency);
    QObject::connect(channelM_, &ChannelCurve::error, this, &Plot::catchError);

    pickedVoltage_ = new QwtPlotMarker();
    pickedVoltage_->setLineStyle(QwtPlotMarker::NoLine);
    pickedVoltage_->setXValue(0);
    pickedVoltage_->setYValue(0);
    pickedVoltage_->setVisible(false);
    pickedVoltage_->attach(this);

    replot();

}

void Plot::pointSelected(int channel, int index) {
    ChannelCurve* curve;
    switch((Channel)channel) {
        case A:
            curve = channelA_;
            qDebug() << "Point " << index << " selected on channel A";
            break;
        case B:
            curve = channelB_;
            qDebug() << "Point " << index << " selected on channel B";
            break;
        case F:
            curve = channelF_;
            qDebug() << "Point " << index << " selected on channel F";
            break;
        case M:
            curve = channelM_;
            qDebug() << "Point " << index << " selected on channel M";
            break;
    }

        pickedVoltage_->setVisible(false);
        QPen pen = curve->pen();
        QColor colour = pen.color();

        //QwtSymbol *symbol = const_cast<QwtSymbol *>( curve->symbol() );
        QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse, colour, pen, QSize( 4, 4 ) );
        pickedVoltage_->setSymbol(symbol);

        //qDebug() << "    Plot:";

        double scaledY = canvasMap(yLeft).invTransform(curve->transform(canvasMap(yLeft), curve->sample(index).y()));
        //pickedVoltage_->setYValue(curve->invTransform(axisStepSize(yLeft), curve->sample(index).y()));
        //qDebug() << "Old Y: " << curve->sample(index).y() << " New Y: " << scaledY;
        pickedVoltage_->setYValue(scaledY);
        pickedVoltage_->setXValue(curve->sample(index).x());
        QwtText voltageLabel = QwtText(valueToUnits(curve->sample(index).y()) + "V");
        voltageLabel.setColor(colour);
        QPen backgroundPen = QPen(QColor(40,40,40));
        voltageLabel.setBackgroundBrush(backgroundPen.brush());
        pickedVoltage_->setLabel(voltageLabel);
        Qt::Alignment vAlign = (scaledY < 4.5 * axisStepSize(yLeft) ) ? Qt::AlignTop : Qt::AlignBottom;
        Qt::Alignment hAlign = (curve->sample(index).x() < 3.5* axisStepSize(xBottom)) ? Qt::AlignRight : Qt::AlignLeft;
        pickedVoltage_->setLabelAlignment(vAlign | hAlign);
        pickedVoltage_->setVisible(true);

        replot();
}

void Plot::pointDeselected() {
    pickedVoltage_->setVisible(false);
    replot();
}

void Plot::plotReady(int channel, QVector<QPointF> points) {
    state_->setPlotPoints((Channel)channel, points);

    switch((Channel)channel) {
        case A:
            state_->clearAcquisition(A);
            if (channelA_->selected()) {
                selectClosePoint();
            }
            break;
        case B:
            state_->clearAcquisition(B);
            if (channelB_->selected()) {
                selectClosePoint();
            }
            break;
        case F:
            if (channelF_->selected()) {
                selectClosePoint();
            }
            break;
        case M:
            if (channelM_->selected()) {
                selectClosePoint();
            }
            break;
    }

    replot();

    if (state_->getEquation().contains('A' + channel)) {
        emit processM(*state_);
    }

    if (!channelF_->isEmpty() && state_->getFilterChannel() == channel) {
        emit processF(*state_);
    }

}

void Plot::plotAcquisition(int channel) {
    if ((Channel)channel == A)
        emit processA(*state_);
    else if ((Channel)channel == B)
        emit processB(*state_);
}

void Plot::selectClosePoint() {
    QPoint pos = QPoint(canvasMap(xBottom).transform(pickedVoltage_->xValue()), canvasMap(yLeft).transform(pickedVoltage_->yValue()));
    picker_->select(pos, true);
}

void Plot::solveEquation(QString equation) {
    if (state_->getFilterChannel() == M && equation.contains("F")) {
        catchError("Math channel can not be dependent on the filter channel whilst the math channel is being filtered.");
        return;
    }

    state_->setEquation(equation);
    emit processM(*state_);
    equationLabel_->setText(equation);
}

void Plot::calculateFilter() {
    emit processF(*state_);
}

void Plot::catchError(QString err) {
    emit error(err);
}

void Plot::setLegend(QFrame *legend) {
    QString whiteText = "QLabel {color: white;}";

    QLabel *aLabel = new QLabel("A:");
    aLabel->setStyleSheet(whiteText);
    aDivLabel_ = new QLabel("2V/");
    aDivLabel_->setMinimumWidth(70);
    aDivLabel_->setStyleSheet("QLabel {color: #FFFF00;}");

    QLabel *bLabel = new QLabel("B:");
    bLabel->setStyleSheet(whiteText);
    bDivLabel_ = new QLabel("2V/");
    bDivLabel_->setMinimumWidth(70);
    bDivLabel_->setStyleSheet("QLabel {color: #00FF00;}");

    QLabel *fLabel = new QLabel("Filter:");
    fLabel->setStyleSheet(whiteText);
    fDivLabel_ = new QLabel("2V/");
    fDivLabel_->setMinimumWidth(70);
    fDivLabel_->setStyleSheet("QLabel {color: #282828;}");

    QLabel *mLabel = new QLabel("Math:");
    mLabel->setStyleSheet(whiteText);
    mDivLabel_ = new QLabel("2V/");
    mDivLabel_->setMinimumWidth(70);
    mDivLabel_->setStyleSheet("QLabel {color: #282828;}");

    hDivLabel_ = new QLabel("1s/");
    hDivLabel_->setMinimumWidth(70);
    hDivLabel_->setStyleSheet(whiteText);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *legendLayout = new QHBoxLayout();
    legendLayout->addWidget(aLabel);
    legendLayout->addWidget(aDivLabel_);
    legendLayout->addWidget(bLabel);
    legendLayout->addWidget(bDivLabel_);
    legendLayout->addWidget(fLabel);
    legendLayout->addWidget(fDivLabel_);
    legendLayout->addWidget(mLabel);
    legendLayout->addWidget(mDivLabel_);
    legendLayout->addWidget(spacer);
    legendLayout->addWidget(hDivLabel_);

    legendLayout->setContentsMargins(10,3,10,5);
    legendLayout->setSpacing(3);

    legend->setLayout(legendLayout);
}

void Plot::setMarkerPlot(QwtPlot *markerPlot) {
    markerPlot_ = markerPlot;

    markerPlot_->plotLayout()->setCanvasMargin(0);
    markerPlot_->plotLayout()->setCanvasMargin(4, QwtPlot::yLeft);

    ((QwtPlotCanvas*)(markerPlot_->canvas()))->setStyleSheet("QwtPlotCanvas { border: 1px solid #282828; "
                                              "border-radius: 0px; }");

    markerPlot_->enableAxis(QwtPlot::xBottom, false);
    markerPlot_->enableAxis(QwtPlot::yLeft, false);

    markerPlot_->setAxisScale(QwtPlot::yLeft, -10, 10, 2);
    markerPlot_->setAxisScale(QwtPlot::xBottom, 0, 18, 1);

    markerPlot_->setMinimumWidth(20);

    tMarker_ = new QwtPlotMarker();
    tMarker_->setLineStyle(QwtPlotMarker::NoLine);
    tMarker_->setYValue(0.0);
    tMarker_->setXValue(15);

    QwtSymbol* triggerSymbol = new QwtSymbol(QwtSymbol::RTriangle, QBrush(Qt::white), QPen(Qt::white), QSize(5,5));
    tMarker_->setSymbol(triggerSymbol);

    QwtText *markerLabel = new QwtText("T     ");
    markerLabel->setColor(Qt::white);
    tMarker_->setLabel(*markerLabel);

    tMarker_->attach(markerPlot_);
}

void Plot::setEquationLabel(QLabel* label) {
    equationLabel_ = label;
}

void Plot::setCurveVisible(Channel channel, bool visible) {
    QString color;

    if(!visible) {
        emit channelHidden((int)channel);
    }

    switch(channel) {
        case A :
            channelA_->setVisible(visible);

            color = visible ? "#00FF00}" : "#282828}";
            aDivLabel_->setStyleSheet("QLabel{ color: " + color);

            if (!visible && channelA_->selected()) {
                channelA_->setSelected(false);
                pointDeselected();
            }
            break;
        case B :
            channelB_->setVisible(visible);

            color = visible ? "#FFFF00}" : "#282828}";
            bDivLabel_->setStyleSheet("QLabel{ color: " + color);

            if (!visible && channelB_->selected()) {
                channelB_->setSelected(false);
                pointDeselected();
            }
            break;
        case F :
            channelF_->setVisible(visible);

            color = visible ? "#00FFFF}" : "#282828}";
            fDivLabel_->setStyleSheet("QLabel{ color: " + color);

            if (!visible && channelF_->selected()) {
                channelF_->setSelected(false);
                pointDeselected();
            }
            break;
        case M :
            channelM_->setVisible(visible);

            color = visible ? "#FF00FF}" : "#282828}";
            mDivLabel_->setStyleSheet("QLabel{ color: " + color);

            if (!visible && channelM_->selected()) {
                channelM_->setSelected(false);
                pointDeselected();
            }
            break;
    }

    replot();
}

void Plot::scaleA(int value) {
    double division = verticalDivisions.at(value);
    channelA_->setScale(division);
    aDivLabel_->setText(valueToUnits(division) + "V/");
    state_->setVoltageDiv(A, value);
    if (state_->getTriggerChannel() == A) {

        changeTrigger();
    }

    if (channelA_->selected()) {
        selectClosePoint();
    }

    replot();
}

void Plot::scaleB(int value) {
    double division = verticalDivisions.at(value);
    channelB_->setScale(division);
    bDivLabel_->setText(valueToUnits(division) + "V/");
    state_->setVoltageDiv(B, value);
    if (state_->getTriggerChannel() == B) {

        changeTrigger();
    }

    if (channelB_->selected()) {
        selectClosePoint();
    }

    replot();
}

void Plot::scaleF(int value) {
    double division = verticalDivisions.at(value);
    channelF_->setScale(division);
    fDivLabel_->setText(valueToUnits(division) + "V/");
    state_->setVoltageDiv(F, value);

    if (channelF_->selected()) {
        selectClosePoint();
    }

    replot();
}

void Plot::scaleM(int value) {
    double division = verticalDivisions.at(value);
    channelM_->setScale(division);
    mDivLabel_->setText(valueToUnits(division) + "V/");
    state_->setVoltageDiv(M, value);

    if (channelM_->selected()) {
        selectClosePoint();
    }

    replot();
}

void Plot::scaleH(int value) {
    double division = horizontalDivisions.at(value);
    setAxisScale(QwtPlot::xBottom, -5.0 * division, 5.0 * division, division);

    hDivLabel_->setText(valueToUnits(division / 1000.0) + "s/");

    state_->setTimeDiv(value);
    channelA_->setTimeDiv(division);
    channelB_->setTimeDiv(division);
    channelF_->setTimeDiv(division);
    channelM_->setTimeDiv(division);

    replot();
}

double Plot::changeTrigger() {
    int value = state_->getTriggerThreshold();
    double voltageDiv = verticalDivisions.at(state_->getVoltageDiv(state_->getTriggerChannel()));
    double voltageVal = ((20.0 / qPow(2.0, 12)) * value ) - 10.0;
    voltageVal = round(voltageVal * 1000.0) / 1000.0;

    triggerThreshold_->setYValue(voltageVal);
    triggerThreshold_->setVisible(true);

    scaleTriggerPlot(voltageDiv);

    replot();

    QTimer::singleShot(3000, this, SLOT(hideTrigger()));
    return voltageVal;
}

void Plot::hideTrigger() {
    triggerThreshold_->setVisible(false);
    replot();
}


void Plot::scaleTriggerPlot(double division) {
    setAxisScale(QwtPlot::yLeft, -5.0 * division, 5.0* division, division);
    y0Marker_->setYValue((5.0 * division) - (0.05 * division));
    markerPlot_->setAxisScale(QwtPlot::yLeft, -5.0 * division, 5.0* division, division);
    double maxVoltage = (5.0 * division) - (0.08 * division);

    if (triggerThreshold_->yValue() > maxVoltage) {
        tMarker_->setYValue(maxVoltage);
    } else if (triggerThreshold_->yValue() < -1 * maxVoltage) {
        tMarker_->setYValue(-1 * maxVoltage);
    } else {
        tMarker_->setYValue(triggerThreshold_->yValue());
    }

    markerPlot_->replot();
}

void Plot::styleCanvas() {
    ((QwtPlotCanvas*)canvas())->setStyleSheet("QwtPlotCanvas { border: 1px solid white; "
                                              "border-radius: 0px; }");
    double voltageDiv = verticalDivisions.at(state_->getVoltageDiv(state_->getTriggerChannel()));
    double timeDiv = horizontalDivisions.at(state_->getTimeDiv());

    plotLayout()->setCanvasMargin(0);

    enableAxis(QwtPlot::xBottom, false);
    enableAxis(QwtPlot::yLeft, false);

    setAxisScale(QwtPlot::yLeft, -5.0 * voltageDiv, 5.0* voltageDiv, voltageDiv);
    setAxisScale(QwtPlot::xBottom, -5.0 * timeDiv, 5.0 * timeDiv, timeDiv);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setPen( Qt::white, 0.0, Qt::DotLine );
    grid->enableX(true);
    grid->enableY(true);
    grid->attach(this);

    QwtPlotMarker* v_origin = new QwtPlotMarker();
    v_origin->setLineStyle(QwtPlotMarker::HLine);
    v_origin->setYValue(0.0);
    v_origin->setLinePen(Qt::white, 0.5, Qt::DashLine);
    v_origin->attach(this);

    triggerThreshold_ = new QwtPlotMarker();
    triggerThreshold_->setLineStyle(QwtPlotMarker::HLine);
    triggerThreshold_->setYValue(0.0);
    triggerThreshold_->setLinePen(QColor(255, 179, 0), 2, Qt::SolidLine);
    triggerThreshold_->setVisible(false);
    triggerThreshold_->attach(this);

    y0Marker_ = new QwtPlotMarker();
    y0Marker_->setLineStyle(QwtPlotMarker::NoLine);
    y0Marker_->setYValue((5.0 * voltageDiv) - (0.05 * voltageDiv));
    y0Marker_->setXValue(0);

    QwtSymbol* y0Arrow = new QwtSymbol(QwtSymbol::DTriangle, QBrush(QColor(255, 179, 0)), QPen(QColor(255, 179, 0)), QSize(5,5));
    y0Marker_->setSymbol(y0Arrow);
    y0Marker_->attach(this);
}

void Plot::exit() {
    aThread_->quit();
    bThread_->quit();
    fThread_->quit();
    mThread_->quit();
}
