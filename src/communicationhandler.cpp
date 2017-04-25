#include "communicationhandler.h"

CommunicationHandler::CommunicationHandler(State *state, QObject* parent) : QObject(parent)
{
    connected_ = false;
    socket_ = new QTcpSocket();
    stream_.setDevice(socket_);
    state_ = state;

    readTimer_ = new QTimer(this);
    readTimer_->setInterval(2);

    keepAliveTimer_ = new QTimer(this);
    keepAliveTimer_->setInterval(1000);

    QObject::connect(readTimer_, &QTimer::timeout, this, &CommunicationHandler::read);
    QObject::connect(keepAliveTimer_, &QTimer::timeout, this, &CommunicationHandler::handShake);

    QObject::connect(socket_, &QTcpSocket::connected, this, &CommunicationHandler::connected);
    QObject::connect(socket_, &QTcpSocket::disconnected, this, &CommunicationHandler::disconnected);
    QObject::connect(socket_, &QTcpSocket::bytesWritten, this, &CommunicationHandler::bytesWritten);
    QObject::connect(socket_, &QTcpSocket::aboutToClose, this, &CommunicationHandler::closing);
}

void CommunicationHandler::connect(QString hostName) {
    socket_->connectToHost(hostName, 62421);

    if(!socket_->waitForConnected(5000))
    {
        qDebug() << "Error: " << socket_->errorString();
        emit connectionError("Error: " + socket_->errorString());
        socket_->close();
    }
}

void CommunicationHandler::closeConnection() {
    if (socket_->isOpen()) {
        socket_->close();
    }
}

void CommunicationHandler::read() {

    if (socket_->bytesAvailable() == 0) return;

    qDebug() << socket_->bytesAvailable() << " bytes available";

    char *header = (char*)malloc(sizeof(char));
    char *channel = (char*)malloc(sizeof(char));
    char *option = (char*)malloc(sizeof(char));

    quint16 data;
    quint8 byte;
    quint16 mask = state_->getBitMode() == EIGHT_BIT ? 0xFF : 0xFFF;

    quint16 numSamples = 0, lastFlag = 0;
    QList<quint16> acquisition;
    acquisition.clear();

    stream_.readRawData(header, 1);
    MessageType type = (MessageType)(*header - 'a');

    switch(type) {
        case VOLTAGE_ERROR:
            emit voltageError();
            break;
        case ACQUISITION:
            if (socket_->bytesAvailable() < 3)
                break;

            stream_.readRawData(channel, 1);          
            stream_ >> numSamples;
            numSamples = ntohs(numSamples);
            lastFlag = 0x8000 & numSamples;
            numSamples = 0x7FFF & numSamples;

            qDebug() << "Samples in packet: " << numSamples;

            if (state_->getBitMode() == EIGHT_BIT)
                qDebug() << "8 Bit";
            for (int i = 0; i < numSamples; i++) {
                if (state_->getBitMode() == TWELVE_BIT) {
                    stream_ >> data;
                    acquisition.append(ntohs(data) & mask);
                } else {
                    stream_ >> byte;
                    acquisition.append((quint16)ntohs(byte));
                }
            }

            qDebug() << "Number of samples: " << acquisition.size();

            state_->setAcquisition((Channel)(*channel - '0'), acquisition);

            qDebug() << acquisition;
            if (lastFlag != 0) {
                qDebug() << "A size: " << state_->getAcquisition(A).size();
                emit acquisitionReady((int)(*channel - '0'));
                acquisition.clear();
            }

            break;
        case NO_SAMPLES:
            if (socket_->bytesAvailable() < 2)
                break;
            stream_ >> data;
            emit numSamplesChanged(ntohs(data));
            break;
        case BIT_MODE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit bitModeChanged((int)('B' - *option));
            break;
        case FILTER_MODE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit aFilterModeChanged((int)(*option - 'A'));
            break;
        case COUPLING:
            if (socket_->bytesAvailable() < 2)
                break;
            stream_.readRawData(channel, 1);
            stream_.readRawData(option, 1);
            emit couplingChanged((int)(*channel - '0'), (int)(*option - 'A'));
            break;
        case VERTICAL_RANGE:
            if (socket_->bytesAvailable() < 2)
                break;
            stream_.readRawData(channel, 1);
            stream_.readRawData(option, 1);
            emit vScaleChanged((int)(*channel - '0'), (int)('G' - *option));
            break;
        case OFFSET:
            if (socket_->bytesAvailable() < 3)
                break;
            stream_.readRawData(channel, 1);
            stream_ >> data;
            emit offsetChanged((int)(*channel - '0'), ntohs(data));
            break;
        case HORIZONTAL_RANGE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit hScaleChanged((int)('S' - *option));
            break;
        case TRIGGER_STATUS:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit triggerStatusChanged((int)(*option - 'A'));
            break;
        case TRIGGER_CHANNEL:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(channel, 1);
            emit triggerChannelChanged((int)(*channel - '0'));
            break;
        case TRIGGER_MODE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit triggerModeChanged((int)(*option - 'A'));
            break;
        case TRIGGER_TYPE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit triggerTypeChanged((int)(*option - 'A'));
            break;
        case TRIGGER_THRESHOLD:
            if (socket_->bytesAvailable() < 2)
                break;
            stream_ >> data;
            emit triggerThresholdChanged(ntohs(data));
            break;
        case FUNCTION_STATE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit functionGenEnabled(((int)(*option - 'A') == ON));
            break;
        case FUNCTION_WAVE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit functionGenWaveChanged((int)(*option - 'A'));
            break;
        case FUNCTION_VOLTAGE:
            if (socket_->bytesAvailable() < 1)
                break;
            stream_.readRawData(option, 1);
            emit functionGenVoltageChanged((int)(*option - 'A'));
            break;
        case FUNCTION_OFFSET:
            if (socket_->bytesAvailable() < 2)
                break;
            stream_ >> data;
            emit functionGenOffsetChanged(ntohs(data));
            break;
        case FUNCTION_FREQ:
        {
            if (socket_->bytesAvailable() < 2)
                break;
            stream_ >> data;
            quint16 freq = state_->functionFreqs_.indexOf(ntohs(data));
            if (freq > 0) {
                emit functionGenFreqChanged(freq);
            }
            break;
        }
        default:
            qDebug() << "Invalid message type: " << QString(header);
    }
}

void CommunicationHandler::write(QByteArray msg) {
    qDebug() << "Writing: " << msg;

    stream_.writeRawData(msg.constData(), msg.size());
    socket_->waitForBytesWritten(30);
}

void CommunicationHandler::errorAcknowledged(QAbstractButton*) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + VOLTAGE_ERROR);
        write(msg);
    }
}

void CommunicationHandler::setNumSamples(quint16 numSamples) {
    qDebug() << numSamples;
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + NO_SAMPLES);
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(numSamples);

        socket_->waitForBytesWritten(30);
    } else {
        emit numSamplesChanged(numSamples);
    }
}

void CommunicationHandler::setBitMode(int mode) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + BIT_MODE);
        msg.append('B' - mode);
        write(msg);
    } else {
        emit bitModeChanged(mode);
    }
}

void CommunicationHandler::setAFilterMode(bool bandpass) {
    FilteringMode mode = bandpass ? BANDPASS : LOWPASS;
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + FILTER_MODE);
        msg.append('A' + mode);
        write(msg);
    } else {
        emit aFilterModeChanged(mode);
    }
}

void CommunicationHandler::setACoupling(bool isDC) {
    VoltageCoupling coupling = isDC ? DC : AC;

    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + COUPLING);
        msg.append('0' + A);
        msg.append('A' + coupling);
        write(msg);
    } else {
        emit couplingChanged((int)A, (int)coupling);
    }
}

void CommunicationHandler::setBCoupling(bool isDC) {
    VoltageCoupling coupling = isDC ? DC : AC;
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + COUPLING);
        msg.append('0' + B);
        msg.append('A' + coupling);
        write(msg);
    } else {
        emit couplingChanged((int)B, (int)coupling);
    }
}


void CommunicationHandler::scaleA(int division) {
    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + VERTICAL_RANGE);
        msg.append('0' + A);
        msg.append('G' - division);
        write(msg);
    } else {
        emit vScaleChanged((int)A, division);
    }
}

void CommunicationHandler::scaleB(int division) {
    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + VERTICAL_RANGE);
        msg.append('0' + B);
        msg.append('G' - division);
        write(msg);
    } else {
        emit vScaleChanged((int)B, division);
    }
}

void CommunicationHandler::setAOffset(quint16 offset) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + OFFSET);
        msg.append('0');
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(offset);
        socket_->waitForBytesWritten(30);
        //write(msg);
    } else {
        emit offsetChanged((int)A, offset);
    }
}

void CommunicationHandler::setBOffset(quint16 offset) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + OFFSET);
        msg.append('1');
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(offset);
        socket_->waitForBytesWritten(30);
        //write(msg);
    } else {
        emit offsetChanged((int)B, offset);
    }
}

void CommunicationHandler::scaleH(int division) {
    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + HORIZONTAL_RANGE);
        msg.append('S' - division);
        write(msg);
    } else {
        emit hScaleChanged(division);
    }
}

// Check this
void CommunicationHandler::setTriggerChannel(bool channelA) {
    Channel channel = channelA ? A : B;

    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + TRIGGER_CHANNEL);
        msg.append('0' + (int)channel);
        write(msg);
    } else {
        emit triggerChannelChanged((int)channel);
    }
}

void CommunicationHandler::setTriggerMode(int mode) {
    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + TRIGGER_MODE);
        msg.append('A' + mode);
        write(msg);
    } else {
        emit triggerModeChanged(mode);
    }
}

void CommunicationHandler::setTriggerType(int type) {
    if(socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + TRIGGER_TYPE);
        msg.append('A' + type);
        write(msg);
    } else {
        emit triggerTypeChanged(type);
    }
}

void CommunicationHandler::setTriggerValue(quint16 value) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + TRIGGER_THRESHOLD);
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(value);
        socket_->waitForBytesWritten(30);
    } else {
        emit triggerThresholdChanged(value);
    }
}

void CommunicationHandler::forceTrigger() {
    if (socket_->isOpen()) {
        state_->setTriggerForced(true);
        QByteArray msg;
        msg.append('a' + FORCE_TRIGGER);
        write(msg);
    }
}

void CommunicationHandler::rearmTrigger() {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + REARM_TRIGGER);
        write(msg);
    }
}

void CommunicationHandler::enableFunctionGen(bool enabled) {
    FunctionGenState state = enabled? ON : OFF;
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + FUNCTION_STATE);
        msg.append('A' + state);
        write(msg);
    } else {
        emit functionGenEnabled(enabled);
    }
}

void CommunicationHandler::setFunctionWave(int wave) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + FUNCTION_WAVE);
        msg.append('A' + wave);
        write(msg);
    } else {
        emit functionGenWaveChanged(wave);
    }
}

void CommunicationHandler::setFunctionVoltage(int voltage) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + FUNCTION_VOLTAGE);
        msg.append('A' + voltage);
        write(msg);
    } else {
        emit functionGenVoltageChanged(voltage);
    }
}

void CommunicationHandler::setFunctionOffset(quint16 offset) {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + FUNCTION_OFFSET);
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(offset);
        socket_->waitForBytesWritten(30);
    } else {
        emit functionGenOffsetChanged(offset);
    }
}

void CommunicationHandler::setFunctionFreq(quint16 freq) {
    if (socket_->isOpen()) {
        QByteArray msg;
        quint16 actualFreq = state_->functionFreqs_.at(freq);
        msg.append('a' + FUNCTION_FREQ);
        stream_.writeRawData(msg.constData(), msg.size());
        stream_ << htons(actualFreq);
        socket_->waitForBytesWritten(30);
    } else {
        emit functionGenFreqChanged(freq);
    }
}

void CommunicationHandler::handShake() {
    if (socket_->isOpen()) {
        QByteArray msg;
        msg.append('a' + HANDSHAKE);
        write(msg);
    }
}

bool CommunicationHandler::isConnected() {
    return connected_;
}

void CommunicationHandler::bytesWritten(qint64 bytes) {
    qDebug() << QString::number(bytes) << " bytes written.";
}

void CommunicationHandler::connected() {
    qDebug() << "Connected to " << socket_->peerName();
    connected_ = true;
    emit deviceConnected(socket_->peerName());
    readTimer_->start();
    //keepAliveTimer_->start();
    handShake();
}

void CommunicationHandler::disconnected() {
    qDebug() << "Disconnected.";
    connected_ = false;
    readTimer_->stop();
    //keepAliveTimer_->stop();
    if (socket_->isOpen())
        socket_->close();
    emit deviceDisconnected();
}

void CommunicationHandler::closing() {
    qDebug() << "Closing connection...";
}


