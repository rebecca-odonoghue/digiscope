#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStyleFactory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Register types to be used in Signal/Slot relationships between threads.
    qRegisterMetaType<State>();
    qRegisterMetaType<QVector<QPointF>>();


    if( IS_WINDOWS ) {
        QApplication::setStyle(QStyleFactory::create("Fusion"));
    }

    // Set up the main window
    ui->setupUi(this);
    setWindowTitle("Digiscope");

    // Initialise the state.
    state_ = new State();

    // Generate the top toolbar.
    generateOscilloscopeToolbar();

    // Initialise the ethernet communication.
    comHandler_ = new CommunicationHandler(state_, this);

    QObject::connect(comHandler_, &CommunicationHandler::deviceConnected, this, &MainWindow::connected);
    QObject::connect(comHandler_, &CommunicationHandler::deviceDisconnected, this, &MainWindow::disconnected);
    QObject::connect(comHandler_, &CommunicationHandler::connectionError, this, &MainWindow::catchError);

    // Initialise plot
    createPlot();

    // Connect the controls in MainWindow to their appropriate functions.
    connectDials();

    // Initialise the voltage error message box.
    voltageError_ = new QMessageBox();
    voltageError_->setIcon(QMessageBox::Warning);
    voltageError_->setText("Input voltage out of range.");
    QObject::connect(voltageError_, &QMessageBox::buttonClicked, comHandler_, &CommunicationHandler::errorAcknowledged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Called when a new curve has been measured, to set the label text in the GUI.
void MainWindow::newMeasurements(int channel, double vMax, double vMin,
                                 double vPp, double avg, double stdDev) {
    switch((Channel)channel) {
        case A:
            ui->aMaxLabel->setText(valueToUnits(vMax) + "V");
            ui->aMinLabel->setText(valueToUnits(vMin) + "V");
            ui->aPpLabel->setText(valueToUnits(vPp) + "V");
            ui->aAvgLabel->setText(valueToUnits(avg) + "V");
            ui->aStdDevLabel->setText(QString::number(stdDev));
            break;
        case B:
            ui->bMaxLabel->setText(valueToUnits(vMax) + "V");
            ui->bMinLabel->setText(valueToUnits(vMin) + "V");
            ui->bPpLabel->setText(valueToUnits(vPp) + "V");
            ui->bAvgLabel->setText(valueToUnits(avg) + "V");
            ui->bStdDevLabel->setText(QString::number(stdDev));
            break;
        case F:
            ui->fMaxLabel->setText(valueToUnits(vMax) + "V");
            ui->fMinLabel->setText(valueToUnits(vMin) + "V");
            ui->fPpLabel->setText(valueToUnits(vPp) + "V");
            ui->fAvgLabel->setText(valueToUnits(avg) + "V");
            ui->fStdDevLabel->setText(QString::number(stdDev));
            break;
        case M:
            ui->mMaxLabel->setText(valueToUnits(vMax) + "V");
            ui->mMinLabel->setText(valueToUnits(vMin) + "V");
            ui->mPpLabel->setText(valueToUnits(vPp) + "V");
            ui->mAvgLabel->setText(valueToUnits(avg) + "V");
            ui->mStdDevLabel->setText(QString::number(stdDev));
            break;
    }
}

// Called when a new frequency has been calculated to display in the GUI.
void MainWindow::newFrequency(int channel, double freq) {
    switch((Channel)channel) {
        case A:
            ui->aFreqLabel->setText(valueToUnits(freq) + "Hz");
            break;
        case B:
            ui->bFreqLabel->setText(valueToUnits(freq) + "Hz");
            break;
        case F:
            ui->fFreqLabel->setText(valueToUnits(freq) + "Hz");
            break;
        case M:
            ui->mFreqLabel->setText(valueToUnits(freq) + "Hz");
            break;
    }
}

// Called when a channel is hidden to change the labels in the GUI.
void MainWindow::channelHidden(int channel) {
    switch((Channel)channel) {
        case A:
            ui->aMaxLabel->setText("-");
            ui->aMinLabel->setText("-");
            ui->aPpLabel->setText("-");
            ui->aAvgLabel->setText("-");
            ui->aStdDevLabel->setText("-");
            ui->aFreqLabel->setText("-");
            break;
        case B:
            ui->bMaxLabel->setText("-");
            ui->bMinLabel->setText("-");
            ui->bPpLabel->setText("-");
            ui->bAvgLabel->setText("-");
            ui->bStdDevLabel->setText("-");
            ui->bFreqLabel->setText("-");
            break;
        case F:
            ui->fMaxLabel->setText("-");
            ui->fMinLabel->setText("-");
            ui->fPpLabel->setText("-");
            ui->fAvgLabel->setText("-");
            ui->fStdDevLabel->setText("-");
            ui->fFreqLabel->setText("-");
            break;
        case M:
            ui->mMaxLabel->setText("-");
            ui->mMinLabel->setText("-");
            ui->mPpLabel->setText("-");
            ui->mAvgLabel->setText("-");
            ui->mStdDevLabel->setText("-");
            ui->mFreqLabel->setText("-");
            break;
    }
}

/* Helper function for MainWindow, generates and adds oscilloscope
 * tools to the first toolbar in the main window.
 */
void MainWindow::generateOscilloscopeToolbar() {

    QWidget *spacer1 = new QWidget();
    spacer1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer1->setMinimumWidth(5);

    hostStatusIcon_ = new QLabel();
    hostStatusIcon_->setPixmap(QPixmap(":/icons/disabled.png"));
    hostStatusIcon_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hostStatusIcon_->setMaximumSize(22, 22);

    hostStatus_ = new QLabel("Device not connected  ");
    hostStatus_->setStyleSheet("QLabel { color: #3c3c3c;}");

    QLabel* samplesLabel = new QLabel("Number of samples to acquire:  ");
    samplesLabel->setStyleSheet("QLabel { color: #3c3c3c;}");

    noSamplesSelect_ = new QSpinBox();
    noSamplesSelect_->setMinimum(10);
    noSamplesSelect_->setMaximum(50000);
    noSamplesSelect_->setSingleStep(10);
    noSamplesSelect_->setValue(state_->getNoSamples());

    QLabel* bitModeLabel = new QLabel("Voltage resolution: ");
    bitModeLabel->setStyleSheet("QLabel { color: #3c3c3c;}");

    bitModeSelect_ = new QComboBox();    
    bitModeSelect_->addItem("12 bit");
    bitModeSelect_->addItem("8 bit");

    int sampleRate = state_->getNoSamples() / (horizontalDivisions.at(state_->getTimeDiv()) / 100);
    sampleRateLabel_ = new QLabel("Sample Rate: " + valueToUnits(sampleRate) + "Sps");
    sampleRateLabel_->setStyleSheet("QLabel { color: #3c3c3c;}");

    deviceStatus_ = new QLabel();
    deviceStatus_->setText("DISCONNECTED");
    deviceStatus_->setStyleSheet("QLabel { "
                                 "color: #828282;"
                                 "font-size: 18px; "
                                 "border: 1px solid #828282; }");
    deviceStatus_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    deviceStatus_->setMaximumHeight(22);

    QWidget *spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QWidget *spacer3 = new QWidget();
    spacer3->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer3->setMinimumWidth(3);

    ui->oscilloscopeToolBar->addWidget(spacer1);
    ui->oscilloscopeToolBar->addWidget(hostStatusIcon_);
    ui->oscilloscopeToolBar->addWidget(hostStatus_);
    ui->oscilloscopeToolBar->addSeparator();
    ui->oscilloscopeToolBar->addWidget(samplesLabel);
    ui->oscilloscopeToolBar->addWidget(noSamplesSelect_);
    ui->oscilloscopeToolBar->addSeparator();
    ui->oscilloscopeToolBar->addWidget(bitModeLabel);
    ui->oscilloscopeToolBar->addWidget(bitModeSelect_);
    ui->oscilloscopeToolBar->addSeparator();
    ui->oscilloscopeToolBar->addWidget(sampleRateLabel_);
    ui->oscilloscopeToolBar->addWidget(spacer2);
    ui->oscilloscopeToolBar->addWidget(deviceStatus_);
    ui->oscilloscopeToolBar->addWidget(spacer3);
}

/* Helper function for MainWindow, initialises the plot that will display
 * data received from the hardware device.
 */
void MainWindow::createPlot() {

    QGridLayout *windowLayout = new QGridLayout();
    windowLayout->setSpacing(5);
    windowLayout->setContentsMargins(0,0,0,0);

    plot_ = new Plot(state_, this);

    QObject::connect(plot_, &Plot::error, this, &MainWindow::catchError);
    QObject::connect(plot_, &Plot::newMeasurements, this, &MainWindow::newMeasurements);
    QObject::connect(plot_, &Plot::newFrequency, this, &MainWindow::newFrequency);
    QObject::connect(plot_, &Plot::channelHidden, this, &MainWindow::channelHidden);

    QFrame *legend = new QFrame();

    QwtPlot* markerPlot = new QwtPlot();

    plot_->setLegend(legend);
    plot_->setMarkerPlot(markerPlot);

    QString style = "QLabel {color: white;}";

    QLabel* filterLabel = new QLabel("Filter: ");
    filterInfo_ = new QLabel("None");
    QLabel* mathLabel = new QLabel("Equation: ");
    equationInfo_ = new QLabel("None");
    QLabel* functionLabel = new QLabel("Function Generator Output: ");
    functionGenInfo_ = new QLabel("Off");

    plot_->setEquationLabel(equationInfo_);

    filterLabel->setStyleSheet(style);
    filterInfo_->setStyleSheet("QLabel {color: #00FFFF;}");
    mathLabel->setStyleSheet(style);
    equationInfo_->setStyleSheet("QLabel {color: #FF00FF;}");
    functionLabel->setStyleSheet(style);
    functionGenInfo_->setStyleSheet(style);

    windowLayout->addWidget(legend, 0, 1, 1, 5);
    windowLayout->addWidget(plot_, 1, 1, 1, 5);
    windowLayout->addWidget(markerPlot, 1, 0, 1, 1);

    windowLayout->addWidget(filterLabel, 2, 1, 1, 1);
    windowLayout->addWidget(filterInfo_, 2, 2, 1, 2);
    windowLayout->addWidget(mathLabel, 2, 4, 1, 1);
    windowLayout->addWidget(equationInfo_, 2, 5, 1, 1);
    windowLayout->addWidget(functionLabel, 3, 1, 1, 2);
    windowLayout->addWidget(functionGenInfo_, 3, 3, 1, 3);

    windowLayout->setRowStretch(1, 1);
    windowLayout->setColumnStretch(2, 1);
    windowLayout->setColumnStretch(3, 1);
    windowLayout->setColumnStretch(4, 1);
    windowLayout->setColumnStretch(5, 5);

    ui->plotFrame->setLayout(windowLayout);
}

// Connect all controls in the main window to their appropriate functions.
void MainWindow::connectDials() {
    // Signal to handle a voltage error event.
    connect(comHandler_, &CommunicationHandler::voltageError, this, &MainWindow::voltageError);

    // Signal to handle an acquisition event.
    connect(comHandler_, &CommunicationHandler::acquisitionReady, plot_, &Plot::plotAcquisition);

    // Signals to handle a number of samples change.
    connect(comHandler_, &CommunicationHandler::numSamplesChanged, this, &MainWindow::numSamplesChanged);
    connect(noSamplesSelect_, &QSpinBox::editingFinished, this, &MainWindow::numSamplesReady);

    // Signals to handle a bit mode change.
    connect(comHandler_, &CommunicationHandler::bitModeChanged, this, &MainWindow::bitModeChanged);
    connect(bitModeSelect_, SIGNAL(currentIndexChanged(int)), comHandler_, SLOT(setBitMode(int)));

    // Signals to handle a channel A filtering mode change.
    connect(comHandler_, &CommunicationHandler::aFilterModeChanged, this, &MainWindow::aFilterModeChanged);
    connect(ui->BandpassButton, &QRadioButton::toggled, comHandler_, &CommunicationHandler::setAFilterMode);

    // Signals to handle coupling mode changes.
    connect(comHandler_, &CommunicationHandler::couplingChanged, this, &MainWindow::couplingChanged);
    connect(ui->DCRadioButtonA, &QRadioButton::toggled, comHandler_, &CommunicationHandler::setACoupling);
    connect(ui->DCRadioButtonB, &QRadioButton::toggled, comHandler_, &CommunicationHandler::setBCoupling);

    // Signals to handle vertical scale changes.
    connect(comHandler_, &CommunicationHandler::vScaleChanged, this, &MainWindow::vRangeChanged);
    connect(ui->rangeDialA, &QDial::valueChanged, comHandler_, &CommunicationHandler::scaleA);
    connect(ui->rangeDialB, &QDial::valueChanged, comHandler_, &CommunicationHandler::scaleB);

    connect(ui->rangeDialF, &QDial::valueChanged, plot_, &Plot::scaleF);

    connect(ui->rangeDialM, &QDial::valueChanged, plot_, &Plot::scaleM);

    // Signals to handle a channel offset change.
    connect(comHandler_, &CommunicationHandler::offsetChanged, this, &MainWindow::channelOffsetChanged);
    connect(ui->offsetDialA, &QDial::valueChanged, comHandler_, &CommunicationHandler::setAOffset);
    connect(ui->offsetDialB, &QDial::valueChanged, comHandler_, &CommunicationHandler::setBOffset);

    // Signals to handle horizontal range changes.
    connect(comHandler_, &CommunicationHandler::hScaleChanged, this, &MainWindow::hRangeChanged);
    connect(ui->horizontalRangeDial, &QDial::valueChanged, comHandler_, &CommunicationHandler::scaleH);

    // Signal to handle a trigger state change event.
    connect(comHandler_, &CommunicationHandler::triggerStatusChanged, this, &MainWindow::triggerStatusChanged);

    // Signals to handle trigger option changes.
    connect(comHandler_, &CommunicationHandler::triggerChannelChanged, this, &MainWindow::triggerChannelChanged);
    connect(ui->triggerChannelA, &QRadioButton::toggled, comHandler_, &CommunicationHandler::setTriggerChannel);

    connect(comHandler_, &CommunicationHandler::triggerModeChanged, this, &MainWindow::triggerModeChanged);
    connect(ui->triggerModeComboBox, SIGNAL(currentIndexChanged(int)), comHandler_, SLOT(setTriggerMode(int)));

    connect(comHandler_, &CommunicationHandler::triggerTypeChanged, this, &MainWindow::triggerTypeChanged);
    connect(ui->triggerTypeComboBox, SIGNAL(currentIndexChanged(int)), comHandler_, SLOT(setTriggerType(int)));

    connect(comHandler_, &CommunicationHandler::triggerThresholdChanged, this, &MainWindow::triggerValueChanged);
    connect(ui->triggerThresholdDial, &QDial::valueChanged, comHandler_, &CommunicationHandler::setTriggerValue);

    // Signals to force or rearm the trigger
    connect(ui->forceTriggerButton, &QPushButton::clicked, comHandler_, &CommunicationHandler::forceTrigger);

    connect(ui->rearmButton, &QPushButton::clicked, comHandler_, &CommunicationHandler::rearmTrigger);

    // Signals to handle function generator options changes
    connect(comHandler_, &CommunicationHandler::functionGenEnabled, this, &MainWindow::functionStateChanged);
    connect(ui->functionGenCheckBox, &QCheckBox::toggled, comHandler_, &CommunicationHandler::enableFunctionGen);

    connect(comHandler_, &CommunicationHandler::functionGenWaveChanged, this, &MainWindow::functionWaveChanged);
    connect(ui->functionGenWaveType, SIGNAL(currentIndexChanged(int)), comHandler_, SLOT(setFunctionWave(int)));

    connect(comHandler_, &CommunicationHandler::functionGenVoltageChanged, this, &MainWindow::functionVoltageChanged);
    connect(ui->functionVoltageDial, &QDial::valueChanged, comHandler_, &CommunicationHandler::setFunctionVoltage);

    connect(comHandler_, &CommunicationHandler::functionGenOffsetChanged, this, &MainWindow::functionOffsetChanged);
    connect(ui->functionOffsetDial, &QDial::valueChanged, comHandler_, &CommunicationHandler::setFunctionOffset);

    connect(comHandler_, &CommunicationHandler::functionGenFreqChanged, this, &MainWindow::functionFreqChanged);
    connect(ui->functionFreqDial, &QDial::valueChanged, comHandler_, &CommunicationHandler::setFunctionFreq);

}

// Called when a voltage out of range message is received.
void MainWindow::voltageError() {
    //catchError("Input voltage out of range.");
    voltageError_->exec();
}

// Called when the number of samples is changed in the GUI, to check if this
// number is valid, and changes it if not, before calling the communication
// handler function.
void MainWindow::numSamplesReady() {
    int numSamples = noSamplesSelect_->value();
    // Find the maximum number of samples for the given time division.
    int maxSamples = horizontalDivisions.at(state_->getTimeDiv()) * 10000;
    int maxBitSamples = state_->getBitMode() == EIGHT_BIT ? 50000 : 25000;

    if (maxSamples > maxBitSamples)
        maxSamples = maxBitSamples;
    // If the number of samples selected indicates a sample rate greater
    // than 1Msps, change it.
    if (numSamples > maxSamples) {
            numSamples = maxSamples;
    } else if (numSamples%10 != 0) {
        numSamples = ( numSamples / 10 ) * 10;
    }


    comHandler_->setNumSamples((quint16)numSamples);
}

// Called when the number of samples have been changed, sets the number of
// samples and the sample rate in the GUI.
void MainWindow::numSamplesChanged(quint16 numSamples) {
    state_->setNoSamples(numSamples);

    int sampleRate = (int)round((double)state_->getNoSamples() / (horizontalDivisions.at(state_->getTimeDiv()) / 100.0));
    if (sampleRate > 1000000) sampleRate = 1000000;
    sampleRateLabel_->setText("Sample Rate: " + valueToUnits(sampleRate) + "Sps");

    const bool wasBlocked = noSamplesSelect_->blockSignals(true);
    noSamplesSelect_->setValue((int)numSamples);
    noSamplesSelect_->blockSignals(wasBlocked);
}

// Called when the bit mode is changed, sets this value in the GUI.
void MainWindow::bitModeChanged(int mode) {
    state_->setBitMode((VoltageResolution)mode);
    qDebug() << "bit mode changed: " << mode;

    const bool wasBlocked = bitModeSelect_->blockSignals(true);
    bitModeSelect_->setCurrentIndex(mode);
    bitModeSelect_->blockSignals(wasBlocked);
}

// Called when the A filter mode is changed, sets this value in the GUI.
void MainWindow::aFilterModeChanged(int mode) {
    state_->setFilterMode((FilteringMode)mode);
    //plot_->aFilterChanged((FilteringMode)mode == BANDPASS);

    const bool wasBlockedL = ui->LowpassButton->blockSignals(true);
    const bool wasBlockedB = ui->BandpassButton->blockSignals(true);

    ui->LowpassButton->setChecked((FilteringMode)mode == LOWPASS);
    ui->BandpassButton->setChecked((FilteringMode)mode == BANDPASS);

    ui->LowpassButton->blockSignals(wasBlockedL);
    ui->BandpassButton->blockSignals(wasBlockedB);
}

// Called when a coupling mode is changed, sets this value in the GUI.
void MainWindow::couplingChanged(int channel, int coupling) {
    state_->setCouplingType((Channel)channel, (VoltageCoupling)coupling);

    if ((Channel)channel == A) {
        const bool wasBlocked = ui->DCRadioButtonA->blockSignals(true);
        const bool wasBlocked2 = ui->ACRadioButtonA->blockSignals(true);
        ui->DCRadioButtonA->setChecked((VoltageCoupling)coupling == DC);
        ui->ACRadioButtonA->setChecked((VoltageCoupling)coupling == AC);
        ui->DCRadioButtonA->blockSignals(wasBlocked);
        ui->ACRadioButtonA->blockSignals(wasBlocked2);
    } else if ((Channel)channel == B) {
        const bool wasBlocked = ui->DCRadioButtonB->blockSignals(true);
        const bool wasBlocked2 = ui->ACRadioButtonB->blockSignals(true);
        ui->DCRadioButtonB->setChecked((VoltageCoupling)coupling == DC);
        ui->ACRadioButtonB->setChecked((VoltageCoupling)coupling == AC);
        ui->DCRadioButtonB->blockSignals(wasBlocked);
        ui->ACRadioButtonB->blockSignals(wasBlocked2);
    }
}

// Called when a vertical range is changed, sets this value in the GUI.
void MainWindow::vRangeChanged(int channel, int division) {
    state_->setVoltageDiv((Channel)channel, division);
    if((Channel)channel == A) {
        plot_->scaleA(division);
        const bool wasBlocked = ui->rangeDialA->blockSignals(true);
        ui->rangeDialA->setValue(division);
        ui->rangeDialA->blockSignals(wasBlocked);
    } else if ((Channel)channel == B) {
        plot_->scaleB(division);
        const bool wasBlocked = ui->rangeDialB->blockSignals(true);
        ui->rangeDialB->setValue(division);
        ui->rangeDialB->blockSignals(wasBlocked);
    }
}

// Called when an offset is changed, sets this value in the GUI.
void MainWindow::channelOffsetChanged(int channel, quint16 offset) {
    state_->setChannelOffset((Channel)channel, offset);

    if((Channel)channel == A) {
        const bool wasBlocked = ui->offsetDialA->blockSignals(true);
        ui->offsetDialA->setValue((int)offset);
        ui->offsetDialA->blockSignals(wasBlocked);
    } else if ((Channel)channel == B) {
        const bool wasBlocked = ui->offsetDialB->blockSignals(true);
        ui->offsetDialB->setValue((int)offset);
        ui->offsetDialB->blockSignals(wasBlocked);
    }
}

// Called when the horizontal range is changed, validates the number of samples
// and sets these values in the GUI.
void MainWindow::hRangeChanged(int division) {
    state_->setTimeDiv(division);

    // Find the maximum number of samples for the given time division.
    int maxSamples = horizontalDivisions.at(state_->getTimeDiv()) * 10000;
    // If the number of samples selected indicates a sample rate greater
    // than 1Msps, change it.
    if (state_->getNoSamples() > maxSamples) {
        noSamplesSelect_->setValue(maxSamples);
        noSamplesSelect_->editingFinished();
    } else {
        int sampleRate = (int)round((double)state_->getNoSamples() / (horizontalDivisions.at(state_->getTimeDiv()) / 100.0));
        if (sampleRate > 1000000) sampleRate = 1000000;
        sampleRateLabel_->setText("Sample Rate: " + valueToUnits(sampleRate) + "Sps");
    }

    plot_->scaleH(division);

    const bool wasBlocked = ui->horizontalRangeDial->blockSignals(true);
    ui->horizontalRangeDial->setValue(division);
    ui->horizontalRangeDial->blockSignals(wasBlocked);
}

// Called to update the text describing the function generator.
void MainWindow::updateFunctionGenText() {
    if (state_->getFunctionState() == OFF) {
        functionGenInfo_->setText("Off");
    } else {
        int freq = state_->functionFreqs_.at(state_->getFunctionFreq());
        double voltage = functionVoltages.at(state_->getFunctionVoltage());
        QString wave = waveNames.at(state_->getWaveType());
        double offset = ((5.0 / qPow(2, 10)) * state_->getFunctionOffset()) - 2.5;

        QString description = valueToUnits(freq) + "Hz "
                    + valueToUnits(voltage) + "V p-p "
                    + wave +
                    ", " + valueToUnits(offset) + "V offset";

        functionGenInfo_->setText(description);
    }
}

// Called when the trigger status is changed, sets this value in the GUI.
void MainWindow::triggerStatusChanged(int status) {
    TriggerState state = (TriggerState)status;
    state_->setTriggerState(state);
    checkRearm(ui->triggerModeComboBox->currentIndex());
    switch(state) {
        case ARMED:
            deviceStatus_->setText("ARMED");
            deviceStatus_->setStyleSheet("QLabel { "
                                         "color: #339900;"
                                         "font-size: 18px; "
                                         "border: 1px solid #339900; }");
            break;
        case TRIGGERED:
            deviceStatus_->setText("TRIGGERED");
            deviceStatus_->setStyleSheet("QLabel { "
                                         "color: #e69900;"
                                         "font-size: 18px; "
                                         "border: 1px solid #e69900; }");
            break;
        case STOPPED:
            deviceStatus_->setText("STOPPED");
            deviceStatus_->setStyleSheet("QLabel { "
                                         "color: #e62e00;"
                                         "font-size: 18px; "
                                         "border: 1px solid #e62e00; }");
            break;
    }
}

// Called when the trigger channel is changed, sets this value in the GUI.
void MainWindow::triggerChannelChanged(int channel) {
    state_->setTriggerChannel((Channel)channel);
    plot_->changeTrigger();
    const bool wasBlocked = ui->triggerChannelA->blockSignals(true);
    const bool wasBlocked2 = ui->triggerChannelB->blockSignals(true);
    ui->triggerChannelA->setChecked((Channel)channel == A);
    ui->triggerChannelB->setChecked((Channel)channel == B);
    ui->triggerChannelA->blockSignals(wasBlocked);
    ui->triggerChannelB->blockSignals(wasBlocked2);
}

// Called when the trigger mode is changed, sets this value in the GUI.
void MainWindow::triggerModeChanged(int mode) {
    state_->setTriggerMode((TriggerMode)mode);
    const bool wasBlocked = ui->triggerModeComboBox->blockSignals(true);
    ui->triggerModeComboBox->setCurrentIndex(mode);
    ui->triggerModeComboBox->blockSignals(wasBlocked);
}

// Called when the trigger type is changed, sets this value in the GUI.
void MainWindow::triggerTypeChanged(int type) {
    state_->setTriggerType((TriggerType)type);

    const bool wasBlocked = ui->triggerTypeComboBox->blockSignals(true);
    ui->triggerTypeComboBox->setCurrentIndex(type);
    ui->triggerTypeComboBox->blockSignals(wasBlocked);
}

// Called when the trigger threshold is changed, sets this value in the GUI.
void MainWindow::triggerValueChanged(quint16 value) {
    state_->setTriggerThreshold(value);

    double voltage = plot_->changeTrigger();

    ui->triggerValueLabel->setText(valueToUnits(voltage) + "V");
    const bool wasBlocked = ui->triggerThresholdDial->blockSignals(true);
    ui->triggerThresholdDial->setValue(value);
    ui->triggerThresholdDial->blockSignals(wasBlocked);
}

// Called when the function generator is enabled or disabled, sets
// this value in the GUI.
void MainWindow::functionStateChanged(bool enabled) {
    FunctionGenState state = enabled ? ON : OFF;
    state_->setFunctionState(state);
    updateFunctionGenText();

    foreach (QObject *child, ui->functionGenControl->children()) {
        if (child->inherits("QWidget") && !child->inherits("QCheckBox"))
            ((QWidget*)child)->setEnabled(enabled);
    }

    const bool wasBlocked = ui->functionGenCheckBox->blockSignals(true);
    ui->functionGenCheckBox->setChecked(enabled);
    ui->functionGenCheckBox->blockSignals(wasBlocked);
}

// Called when the function generator wave type is changed, sets this value
// in the GUI.
void MainWindow::functionWaveChanged(int wave) {
    state_->setWaveType((FunctionWaveType)wave);
    updateFunctionGenText();

    const bool wasBlocked = ui->functionGenWaveType->blockSignals(true);
    ui->functionGenWaveType->setCurrentIndex(wave);
    ui->functionGenWaveType->blockSignals(wasBlocked);
}

// Called when the function generator voltage is changed, sets this
// value in the GUI.
void MainWindow::functionVoltageChanged(int voltage) {
    state_->setFunctionVoltage(voltage);
    updateFunctionGenText();

    const bool wasBlocked = ui->functionVoltageDial->blockSignals(true);
    ui->functionVoltageDial->setValue(voltage);
    ui->functionVoltageDial->blockSignals(wasBlocked);
}

// Called when the function generator offset is changed, sets this
// value in the GUI.
void MainWindow::functionOffsetChanged(quint16 offset) {
    state_->setFunctionOffset(offset);
    updateFunctionGenText();

    const bool wasBlocked = ui->functionOffsetDial->blockSignals(true);
    ui->functionOffsetDial->setValue((int)offset);
    ui->functionOffsetDial->blockSignals(wasBlocked);
}

// Called when the function generator frequency is changed, sets this
// value in the GUI.
void MainWindow::functionFreqChanged(quint16 freq) {
    state_->setFunctionFreq(freq);
    updateFunctionGenText();

    const bool wasBlocked = ui->functionFreqDial->blockSignals(true);
    ui->functionFreqDial->setValue((int)freq);
    ui->functionFreqDial->blockSignals(wasBlocked);
}

// Called when channel A's visibility is changed, to enable or disable
// channel A's controls.
void MainWindow::setAVisible(int checked) {
    bool visible = checked == 2 ? true : false;

    foreach (QObject *child, ui->controlsA->children()) {
        if (child->inherits("QWidget")&& !child->inherits("QCheckBox"))
            ((QWidget*)child)->setEnabled(visible);
    }

    plot_->setCurveVisible(A, visible);
}

// Called when channel B's visibility is changed, to enable or disable
// channel B's controls.
void MainWindow::setBVisible(int checked) {
    bool visible = checked == 2 ? true : false;

    foreach (QObject *child, ui->controlsB->children()) {
        if (child->inherits("QWidget")&& !child->inherits("QCheckBox"))
            ((QWidget*)child)->setEnabled(visible);
    }

    plot_->setCurveVisible(B, visible);
}

// Called when channel F's visibility is changed, to enable or disable
// channel F's controls.
void MainWindow::setFVisible(int checked) {
    bool visible = checked == 2 ? true : false;

    foreach (QObject *child, ui->controlsF->children()) {
        if (child->inherits("QWidget")&& !child->inherits("QCheckBox")) {
            ((QWidget*)child)->setEnabled(visible);
        }
    }

    plot_->setCurveVisible(F, visible);
}

// Called when channel M's visibility is changed, to enable or disable
// channel M's controls.
void MainWindow::setMVisible(int checked) {

    bool visible = checked == 2 ? true : false;

    foreach (QObject *child, ui->controlsM->children()) {
        if (child->inherits("QWidget") && !child->inherits("QCheckBox"))
            ((QWidget*)child)->setEnabled(visible);
    }

    plot_->setCurveVisible(M, visible);

}

// Called to check if the rearm trigger button should be enabled.
void MainWindow::checkRearm(int index) {
    ui->rearmButton->setEnabled((index == 2
                                 && state_->getTriggerState() != ARMED
                                 && comHandler_->isConnected()) ? true : false);
}

// Called when the user selects 'connect to host' to display a dialog.
void MainWindow::connectToHost() {
    ConnectDialog* connectWindow = new ConnectDialog(this);
    QObject::connect(connectWindow, &ConnectDialog::hostSelected, comHandler_, &CommunicationHandler::connect);
    connectWindow->exec();
}

// Called when the Tiva is connected to reflect this in the GUI.
void MainWindow::connected(QString hostName) {
    hostStatus_->setText("Connected to device at " + hostName + "  ");
    hostStatusIcon_->setPixmap(QPixmap(":/icons/enabled.png"));
}

// Called when the Tiva is disconnected to reflect this in the GUI.
void MainWindow::disconnected() {
    hostStatus_->setText("Device not connected  ");
    hostStatusIcon_->setPixmap(QPixmap(":/icons/disabled.png"));

    state_->setTriggerState(STOPPED);
    ui->rearmButton->setEnabled(false);
    deviceStatus_->setText("DISCONNECTED");
    deviceStatus_->setStyleSheet("QLabel { "
                                 "color: #828282;"
                                 "font-size: 18px; "
                                 "border: 1px solid #828282; }");
}

// Called when the user selects to enter an equation, and displays the dialog.
void MainWindow::enterEquation() {
    EquationDialog* equationWindow = new EquationDialog(state_->getEquation(), this);
    QObject::connect(equationWindow, &EquationDialog::equationEntered, plot_, &Plot::solveEquation);
    equationWindow->exec();
}

// Called when the filter channel is changed, to check that the filter can be
// applied to that channel, then either doing so or emitting an error.
void MainWindow::filterChannelChanged(bool) {
    QString channel = "";
    if (state_->filterEnabled()) {
        if (ui->filterChannelA->isChecked()) {
            state_->setFilterChannel(A);
            channel = "A";
        } else if (ui->filterChannelB->isChecked()) {
            state_->setFilterChannel(B);
            channel = "B";
        } else {
            if (state_->getEquation().contains("F")) {
                catchError("The math channel cannot be filtered while dependent on the filter channel.");
                if (state_->getFilterChannel() == A) {
                    const bool wasBlocked = ui->filterChannelA->blockSignals(true);
                    ui->filterChannelA->setChecked(true);
                    ui->filterChannelA->blockSignals(wasBlocked);
                } else {
                    const bool wasBlocked = ui->filterChannelB->blockSignals(true);
                    ui->filterChannelB->setChecked(true);
                    ui->filterChannelB->blockSignals(wasBlocked);
                }
                return;
            } else {
                state_->setFilterChannel(M);
                channel = "M";
            }
        }
        plot_->calculateFilter();
        QString type = state_->getFilterType() == FIR ? "FIR" : "IIR";
        filterInfo_->setText(type + " filter applied to " + channel);
    }
}

// Called when the user selects to add a new filter, opens the file select
// dialog.
void MainWindow::selectFilter() {
    QFileDialog dialog(this);
    QString homeDir = QStandardPaths::writableLocation(
                QStandardPaths::DocumentsLocation);
    QString fileName = dialog.getOpenFileName(this, tr("Select Filter File"),
                                              homeDir, tr("CSV files (*.csv)"));
    loadFilter(fileName);
}

// Called when the user selects a new filter file and reads the data from
// the specified file.
void MainWindow::loadFilter(QString fileName) {
    QList<double> aVals, bVals;
    Channel channel;
    QString channelName, type;
    QFile filterFile(fileName);

    if (ui->filterChannelA->isChecked()) {
        channel = A;
        channelName = "channel A";
    } else if (ui->filterChannelB->isChecked()) {
        channel = B;
        channelName = "channel B";
    } else {
        channel = M;
        channelName = "Math channel";
    }

    if (filterFile.open(QIODevice::ReadOnly)) {
        while (!filterFile.atEnd()) {

            QByteArray line = filterFile.readLine();
            if (line.endsWith("\r\n")) {
                line.chop(2);
            } else if (line.endsWith("\n")) {
                line.chop(1);
            }

            QList<QByteArray> cells = line.split(',');
            if (cells.size() != 1 && cells.size() != 2) {
                filterFile.close();
                catchError("Specified filter file is incorrectly formatted.");
                return;
            }

            QVariant v = QString(cells.at(0));
            if (v.convert(QMetaType::Double)) {
                bVals.append(cells.at(0).toDouble());
            } else {
                filterFile.close();
                catchError("Specified filter file is incorrectly formatted.");
                return;
            }

            if (cells.size() == 2) {
                v = QString(cells.at(1));
                if (v.convert(QMetaType::Double)) {
                    aVals.append(cells.at(1).toDouble());
                } else {
                    filterFile.close();
                    catchError("Specified filter file is incorrectly formatted.");
                    return;
                }
            }
        }

        filterFile.close();

        if (aVals.size() != bVals.size() && aVals.size() != 0) {
            catchError("Specified filter file is incorrectly formatted.");
            return;
        }

        if (aVals.size() == 0) {
            state_->setFilterType(FIR);
            type = "FIR";
        } else {
            state_->setFilterType(IIR);
            type = "IIR";
        }

        filterInfo_->setText(type + " filter applied to " + channelName);
        state_->setFilterChannel(channel);
        state_->setATaps(aVals);
        state_->setBTaps(bVals);
        state_->setFilterEnabled(true);
        plot_->calculateFilter();
    }
}

// Function called to display a string in an error dialog.
void MainWindow::catchError(QString error) {
    QMessageBox errorBox;
    errorBox.setIcon(QMessageBox::Warning);
    errorBox.setText(error);
    errorBox.exec();
}

// Shows the about dialog
void MainWindow::about() {
    QDialog* aboutWindow = new QDialog();
    Ui::About uiAbout;
    uiAbout.setupUi(aboutWindow);
    aboutWindow->setWindowTitle("About Digiscope");
    aboutWindow->show();
}

// Called when the application is about to close, handles correct shut
// down of the TCP socket and the channel threads.
void MainWindow::closeEvent(QCloseEvent *event) {
    comHandler_->closeConnection();
    plot_->exit();
    event->accept();
}
