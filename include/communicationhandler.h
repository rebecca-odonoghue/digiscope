#ifndef COMMUNICATIONHANDLER_H
#define COMMUNICATIONHANDLER_H

#ifdef OS_WIN32
#include <Winsock2.h>
#include <windows.h>
#endif

#ifdef Q_OS_OSX
#include <arpa/inet.h>
#endif

#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QTimer>
#include <QAbstractButton>

#include <state.h>

typedef enum {HANDSHAKE,
              RUN,
              STOP,
              VOLTAGE_ERROR,
              ACQUISITION,
              NO_SAMPLES,
              BIT_MODE,
              FILTER_MODE,
              COUPLING,
              VERTICAL_RANGE,
              OFFSET,
              HORIZONTAL_RANGE,
              TRIGGER_STATUS,
              TRIGGER_CHANNEL,
              TRIGGER_MODE,
              TRIGGER_TYPE,
              TRIGGER_THRESHOLD,
              FORCE_TRIGGER,
              REARM_TRIGGER,
              FUNCTION_STATE,
              FUNCTION_WAVE,
              FUNCTION_VOLTAGE,
              FUNCTION_OFFSET,
              FUNCTION_FREQ } MessageType;

// Class to handle the TCP connection between the Tiva Launchpad and the
// Digiscope software.
class CommunicationHandler : public QObject
{
    Q_OBJECT

public:
    CommunicationHandler(State* state = NULL, QObject* parent = NULL);
    bool isConnected();

private:
    QTcpSocket* socket_;
    QDataStream stream_;
    State* state_;
    bool connected_;
    QTimer* readTimer_, *keepAliveTimer_;

public slots:
    // Slots connected to GUI widget signals that are emitted when
    // the GUI controls are changed.
    void connect(QString);
    void closeConnection();
    void write(QByteArray);
    void errorAcknowledged(QAbstractButton*);
    void setNumSamples(quint16);
    void setBitMode(int);
    void setAFilterMode(bool);
    void setACoupling(bool);
    void setBCoupling(bool);
    void scaleA(int);
    void scaleB(int);
    void setAOffset(quint16);
    void setBOffset(quint16);
    void scaleH(int);
    void setTriggerChannel(bool);
    void setTriggerMode(int);
    void setTriggerType(int);
    void setTriggerValue(quint16);
    void forceTrigger();
    void rearmTrigger();
    void enableFunctionGen(bool);
    void setFunctionWave(int);
    void setFunctionVoltage(int);
    void setFunctionOffset(quint16);
    void setFunctionFreq(quint16);

private slots:
    // Functions called when a TCP signal is emitted.
    void connected();
    void disconnected();
    void read();
    void bytesWritten(qint64);
    void closing();
    void handShake();

signals:
    // Signals emitted to alert the main window that settings
    // have changed, either because of an ethernet message or
    // because the ethernet is not connected.
    void deviceConnected(QString);
    void deviceDisconnected();
    void connectionError(QString);
    void voltageError();
    void acquisitionReady(int);
    void numSamplesChanged(quint16);
    void bitModeChanged(int);
    void aFilterModeChanged(int);
    void couplingChanged(int,int);
    void vScaleChanged(int, int);
    void offsetChanged(int, quint16);
    void hScaleChanged(int);
    void triggerStatusChanged(int);
    void triggerChannelChanged(int);    
    void triggerModeChanged(int);
    void triggerTypeChanged(int);
    void triggerThresholdChanged(quint16);
    void functionGenEnabled(bool);
    void functionGenWaveChanged(int);
    void functionGenVoltageChanged(int);
    void functionGenOffsetChanged(quint16);
    void functionGenFreqChanged(quint16);
};

#endif // COMMUNICATIONHANDLER_H
