#include "state.h"

State::State()
{
    noSamples_ = 25000;
    aFilterMode_ = LOWPASS;
    aCouplingType_ = DC;
    bCouplingType_ = DC;
    aOffset_ = 512;
    bOffset_ = 512;
    bitMode_ = TWELVE_BIT;
    aDiv_ = 0;
    bDiv_ = 0;
    fDiv_ = 0;
    mDiv_ = 0;
    hDiv_ = 0;
    triggerState_ = STOPPED;
    triggerChannel_ = A;
    triggerMode_ = AUTO;
    triggerType_ = RISING;
    triggerThreshold_ = 2047;
    triggerForced_ = false;
    triggerIndex_ = 0;

    functionState_ = OFF;
    functionWave_ = SINE;
    functionVoltage_ = 2;
    functionFreq_ = 189;
    functionOffset_ = 512;

    equation_ = "";
    filterChannel_ = A;   
    filterEnabled_ = false;

    int currentFreq = 1;
    for (int i = 1; i < 431; i++) {
        functionFreqs_.append(currentFreq);
        if (i < 100) {
            currentFreq++;
        } else if (i < 190) {
            currentFreq += 10;
        } else {
            currentFreq += 100;
        }
    }
}

State::~State() { }

// Public copy constructor.
State::State(const State &state) {
    noSamples_ = state.getNoSamples();
    aOffset_ = state.getChannelOffset(A);
    bOffset_ = state.getChannelOffset(B);
    triggerThreshold_ = state.getTriggerThreshold();
    triggerState_ = state.getTriggerState();
    bitMode_ = state.getBitMode();
    aDiv_ = state.getVoltageDiv(A);
    bDiv_ = state.getVoltageDiv(B);
    fDiv_ = state.getVoltageDiv(F);
    mDiv_ = state.getVoltageDiv(M);
    hDiv_ = state.getTimeDiv();
    functionVoltage_ = state.getFunctionVoltage();
    aFilterMode_ = state.getFilterMode();
    aCouplingType_ = state.getCouplingType(A);
    bCouplingType_ = state.getCouplingType(B);
    triggerChannel_ = state.getTriggerChannel();
    triggerMode_ = state.getTriggerMode();
    triggerType_ = state.getTriggerType();
    aData_ = state.getPlotPoints(A);
    bData_ = state.getPlotPoints(B);
    fData_ = state.getPlotPoints(F);
    mData_ = state.getPlotPoints(M);
    functionState_ = state.getFunctionState();
    functionWave_ = state.getWaveType();
    functionOffset_ = state.getFunctionOffset();
    functionFreq_ = state.getFunctionFreq();
    aAcquisition_ = state.getAcquisition(A);
    bAcquisition_ = state.getAcquisition(B);
    filterChannel_ = state.getFilterChannel();
    filterEnabled_ = state.filterEnabled();
    aTaps_ = state.getATaps();
    bTaps_ = state.getBTaps();
    filterType_ = state.getFilterType();
    equation_ = state.getEquation();
    triggerForced_ = state.triggerForced();
    triggerIndex_ = state.getTriggerIndex();

    functionFreqs_ = state.functionFreqs_;
}

// Returns the current number of samples.
quint16 State::getNoSamples() const {
    return noSamples_;
}

// Sets the current number of samples.
void State::setNoSamples(quint16 noSamples) {
    noSamples_ = noSamples;
}

// Gets the current bit mode.
VoltageResolution State::getBitMode() const {
    return bitMode_;
}

// Sets the current bit mode.
void State::setBitMode(VoltageResolution mode) {
    bitMode_ = mode;
}

// Gets the trigger threshold.
quint16 State::getTriggerThreshold() const {
    return triggerThreshold_;
}

// Sets the trigger threshold.
void State::setTriggerThreshold(quint16 value) {
    triggerThreshold_ = value;
}

// Gets the filtering mode applied to A.
FilteringMode State::getFilterMode() const {
    return aFilterMode_;
}

// Sets the filtering mode applied to A.
void State::setFilterMode(FilteringMode mode) {
    aFilterMode_ = mode;
}

// Gets the coupling type of the specified channel.
VoltageCoupling State::getCouplingType(Channel channel) const {
    if (channel == A) return aCouplingType_;
    else return bCouplingType_;
}

// Sets the coupling type of the specified channel.
void State::setCouplingType(Channel channel, VoltageCoupling type) {
    if (channel == A) aCouplingType_ = type;
    else bCouplingType_ = type;
}

// Gets the voltage division value for the specified channel.
int State::getVoltageDiv(Channel channel) const {
    switch(channel) {
        case A:
            return aDiv_;
        case B:
            return bDiv_;
        case F:
            return fDiv_;
        case M:
            return mDiv_;
    }
}

// Sets the voltage division value for the specified channel.
void State::setVoltageDiv(Channel channel, int value) {
    switch(channel) {
        case A:
            aDiv_ = value;
            break;
        case B:
            bDiv_ = value;
            break;
        case F:
            fDiv_ = value;
            break;
        case M:
            mDiv_ = value;
            break;
    }
}

// Gets the current time division value.
int State::getTimeDiv() const {
    return hDiv_;
}

// Sets the current time division value.
void State::setTimeDiv(int value) {
    hDiv_ = value;
}

// Gets the channel offset of the specified channel.
quint16 State::getChannelOffset(Channel channel) const {
    if (channel == A) return aOffset_;
    else return bOffset_;
}

// Sets the channel offset of the specified channel.
void State::setChannelOffset(Channel channel, quint16 offset) {
    if(channel == A) aOffset_ = offset;
    else bOffset_ = offset;
}

// Gets the current trigger state.
TriggerState State::getTriggerState() const {
    return triggerState_;
}

// Sets the current trigger state.
void State::setTriggerState(TriggerState state) {
    triggerState_ = state;
}

// Gets the current trigger channel.
Channel State::getTriggerChannel() const {
    return triggerChannel_;
}

// Sets the current trigger channel.
void State::setTriggerChannel(Channel channel) {
    triggerChannel_ = channel;
}

// Gets the current trigger mode.
TriggerMode State::getTriggerMode() const {
    return triggerMode_;
}

// Sets the current trigger mode.
void State::setTriggerMode(TriggerMode mode) {
    triggerMode_ = mode;
}

// Gets the current trigger type.
TriggerType State::getTriggerType() const {
    return triggerType_;
}

// Sets the current trigger type.
void State::setTriggerType(TriggerType type) {
    triggerType_ = type;
}

// Returns the plot points of the specified channel.
QVector<QPointF> State::getPlotPoints(Channel channel) const {
    switch(channel) {
        case A:
            return aData_;
        case B:
            return bData_;
        case F:
            return fData_;
        case M:
            return mData_;
    }
}

// Sets the plot points of the specified channel.
void State::setPlotPoints(Channel channel, QVector<QPointF> data) {
    switch(channel) {
        case A:
            aData_ = data;
            break;
        case B:
            bData_ = data;
            break;
        case F:
            fData_ = data;
            break;
        case M:
            mData_ = data;
            break;
    }
}

// Clears the plot of the specified channel.
void State::clearPlot(Channel channel) {
    switch(channel) {
        case A:
            aData_.clear();
            break;
        case B:
            bData_.clear();
            break;
        case F:
            fData_.clear();
            break;
        case M:
            mData_.clear();
            break;
    }
}

// Gets the current function generator peak to peak value.
int State::getFunctionVoltage() const {
    return functionVoltage_;
}

// Sets the current function generator peak to peak value.
void State::setFunctionVoltage(int voltage) {
    functionVoltage_ = voltage;
}

// Gets the current function generator state.
FunctionGenState State::getFunctionState() const {
    return functionState_;
}

// Sets the current function generator state.
void State::setFunctionState(FunctionGenState state) {
    functionState_ = state;
}

// Gets the current function generator wave type.
FunctionWaveType State::getWaveType() const {
    return functionWave_;
}

// Sets the current function generator wave type.
void State::setWaveType(FunctionWaveType type) {
    functionWave_ = type;
}

// Gets the current function generator offset.
quint16 State::getFunctionOffset() const {
    return functionOffset_;
}

// Sets the current function generator offset.
void State::setFunctionOffset(quint16 offset) {
    functionOffset_ = offset;
}

// Gets the current function generator frequency.
quint16 State::getFunctionFreq() const {
    return functionFreq_;
}

// Sets the current function generator frequency.
void State::setFunctionFreq(quint16 freq) {
    functionFreq_ = freq;
}

// Returns the acquisition of the specified channel.
QList<quint16> State::getAcquisition(Channel channel) const {
    if (channel == A) {
        return aAcquisition_;
    } else {
        return bAcquisition_;
    }
}

// Sets the acquisition of the specified channel.
void State::setAcquisition(Channel channel, QList<quint16> data) {
    if (channel == A) {
        aAcquisition_.append(data);
    } else {
        bAcquisition_.append(data);
    }
}

// Clears the acquisition of the specified channel.
void State::clearAcquisition(Channel channel) {
    if (channel == A) {
        aAcquisition_.clear();
    } else {
        bAcquisition_.clear();
    }
}

// Returns true if the most recent trigger event was forced.
bool State::triggerForced() const {
    return triggerForced_;
}

// Sets the most recent trigger event to be forced.
void State::setTriggerForced(bool forced) {
    triggerForced_ = forced;
}

// Returns the index of the trigger sample.
int State::getTriggerIndex() const {
    return triggerIndex_;
}

// Sets the index of the trigger sample.
void State::setTriggerIndex(int index) {
    triggerIndex_ = index;
}

// Returns the list of A taps for the current filter.
QList<double> State::getATaps() const {
    return aTaps_;
}

// Sets the list of A taps for the current filter.
void State::setATaps(QList<double> taps) {
    aTaps_ = taps;
}

// Gets the list of B taps for the current filter.
QList<double> State::getBTaps() const {
    return bTaps_;
}

// Sets the list of B taps for the current filter.
void State::setBTaps(QList<double> taps) {
    bTaps_ = taps;
}

// Returns the channel currently being filtered.
Channel State::getFilterChannel() const {
    return filterChannel_;
}

// Sets the channel currently being filtered.
void State::setFilterChannel(Channel channel) {
    filterChannel_ = channel;
}

// Gets the enabled state of the filter.
bool State::filterEnabled() const {
    return filterEnabled_;
}

// Sets the enabled state of the filter
void State::setFilterEnabled(bool enabled) {
    filterEnabled_ = enabled;
}

// Returns the current type of the filter.
FilterType State::getFilterType() const{
    return filterType_;
}

// Sets the current type of the filter.
void State::setFilterType(FilterType type) {
    filterType_ = type;
}

// Gets the current math equation.
QString State::getEquation() const {
    return equation_;
}

// Sets the current math equation.
void State::setEquation(QString equation) {
    equation_ = equation;
}
