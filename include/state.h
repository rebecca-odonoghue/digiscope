#ifndef STATE_H
#define STATE_H

#include <QVector>
#include <QPointF>
#include <QtMath>
#include <QMetaType>

#include <statedefinitions.h>

//A class containing all current state information to share across
//all Digiscope classes that affect or are affected by the state.
//Contains getter and setter methods to access all variables.
class State
{
public:
    State();
    State(const State &state);
    ~State();
    QVector<int> functionFreqs_;

    // Sampling configuration
    quint16 getNoSamples() const;
    void setNoSamples(quint16);
    VoltageResolution getBitMode() const;
    void setBitMode(VoltageResolution);

    // Channel specific settings
    FilteringMode getFilterMode() const;
    void setFilterMode(FilteringMode);
    VoltageCoupling getCouplingType(Channel) const;
    void setCouplingType(Channel, VoltageCoupling);
    int getVoltageDiv(Channel) const;
    void setVoltageDiv(Channel, int);
    int getTimeDiv() const;
    void setTimeDiv(int);
    quint16 getChannelOffset(Channel) const;
    void setChannelOffset(Channel, quint16);

    // Trigger settings
    TriggerState getTriggerState() const;
    void setTriggerState(TriggerState);
    quint16 getTriggerThreshold() const;
    void setTriggerThreshold(quint16);
    Channel getTriggerChannel() const;
    void setTriggerChannel(Channel);
    TriggerMode getTriggerMode() const;
    void setTriggerMode(TriggerMode);
    TriggerType getTriggerType() const;
    void setTriggerType(TriggerType);

    // Currently plotted data
    QVector<QPointF> getPlotPoints(Channel) const;
    void setPlotPoints(Channel, QVector<QPointF>);
    void clearPlot(Channel);

    // Function generator
    int getFunctionVoltage() const;
    void setFunctionVoltage(int);
    FunctionGenState getFunctionState() const;
    void setFunctionState(FunctionGenState);
    FunctionWaveType getWaveType() const;
    void setWaveType(FunctionWaveType);
    quint16 getFunctionOffset() const;
    void setFunctionOffset(quint16);
    quint16 getFunctionFreq() const;
    void setFunctionFreq(quint16);

    // Acquisition data
    QList<quint16> getAcquisition(Channel) const;
    void setAcquisition(Channel, QList<quint16>);
    void clearAcquisition(Channel);
    bool triggerForced() const;
    void setTriggerForced(bool);
    int getTriggerIndex() const;
    void setTriggerIndex(int);

    // Filter data
    QList<double> getATaps() const;
    void setATaps(QList<double>);
    QList<double> getBTaps() const;
    void setBTaps(QList<double>);
    Channel getFilterChannel() const;
    void setFilterChannel(Channel);
    FilterType getFilterType() const;
    void setFilterType(FilterType);
    bool filterEnabled() const;
    void setFilterEnabled(bool);

    // Math data
    QString getEquation() const;
    void setEquation(QString);

private:
    quint16 noSamples_, aOffset_, bOffset_, triggerThreshold_;
    TriggerState triggerState_;
    VoltageResolution bitMode_;
    int aDiv_, bDiv_, fDiv_, mDiv_, hDiv_, functionVoltage_;
    FilteringMode aFilterMode_;
    VoltageCoupling aCouplingType_, bCouplingType_;
    Channel triggerChannel_, filterChannel_;
    TriggerMode triggerMode_;
    TriggerType triggerType_;
    QVector<QPointF> aData_, bData_, fData_, mData_;
    FunctionGenState functionState_;
    FunctionWaveType functionWave_;
    quint16  functionOffset_, functionFreq_;
    QList<quint16> aAcquisition_, bAcquisition_;
    QList<double> aTaps_, bTaps_;
    QString equation_;
    FilterType filterType_;
    bool filterEnabled_;
    bool triggerForced_;
    int triggerIndex_;
};

Q_DECLARE_METATYPE(State)

#endif // STATE_H
