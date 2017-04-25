#include "channelcurve.h"

ChannelCurve::ChannelCurve(Channel channel, const QString &title,
                           QObject* parent) : QObject(parent),
    QwtPlotCurve(title)
{

    voltageDiv_ = verticalDivisions.at(0);
    timeDiv_ = horizontalDivisions.at(0);
    channel_ = channel;
    selected_ = false;
}

// Sets the vertical scale specific to this curve.
void ChannelCurve::setScale(double scale) {
    voltageDiv_ = scale;
    measureCurve();
}

// Returns the Channel that this curve corresponds to.
Channel ChannelCurve::channel() const {
    return channel_;
}

// Setter function to set the curve as selected.
void ChannelCurve::setSelected(bool selected) {
    selected_ = selected;
}

// Getter function returns true if the curve is connected.
bool ChannelCurve::selected() const {
    return selected_;
}

// Setter function for the horizonal scale.
void ChannelCurve::setTimeDiv(double division) {
    timeDiv_ = division;
}

// Transforms paint co-ordinates to co-ordinates on the current scale.
double ChannelCurve::transform(const QwtScaleMap& yMap, double y) const {
    double minY = voltageDiv_ * -5.0;
    double maxY = voltageDiv_ * 5.0;
    QwtScaleMap newYMap (yMap);
    newYMap.setScaleInterval (minY, maxY);
    return newYMap.transform(y);
}

// Transforms co-ordinates on the current scale to paint co-ordinates.
double ChannelCurve::invTransform(const QwtScaleMap& yMap, double y) const {
    double minY = voltageDiv_ * -5.0;
    double maxY = voltageDiv_ * 5.0;
    QwtScaleMap newYMap (yMap);
    newYMap.setScaleInterval (minY, maxY);
    return newYMap.invTransform(y);
}

// Returns true if the curve has 0 samples.
bool ChannelCurve::isEmpty() const {
    return dataSize() == 0;
}

// Called with the current state to begin processing the curve on its own thread.
void ChannelCurve::process(State state) {
    mutex_.lock();

    state_ = state;

    if (channel_ == M) {
        evaluate();
    } else if (channel_ == F) {
        filter();
    } else {
        setSamples();
    }

    measureCurve();

    findFrequency();

    mutex_.unlock();
}

// Processing function called to translate an acquisition of bit values to
// voltage values.
void ChannelCurve::setSamples() {

    QVector<QPointF> data;
    double voltageDiv = verticalDivisions.at(state_.getVoltageDiv(channel_));
    double timeDiv = horizontalDivisions.at(state_.getTimeDiv());
    double resolution = (state_.getBitMode() == EIGHT_BIT) ? qPow(2.0, 8) : qPow(2.0, 12);
    double voltageStep = 10.0 * voltageDiv / resolution;
    double timeStep = 10.0 * timeDiv / ((double)state_.getNoSamples() - 1.0);
    double currentVoltage, currentTime = -5.0 * timeDiv;

    int sampleDiff = state_.getNoSamples() - state_.getAcquisition(channel_).size();

    if (sampleDiff > 0)
        currentTime += sampleDiff * timeStep;

    for (int i = 0; i < state_.getAcquisition(channel_).size(); i++) {
        quint16 val = state_.getAcquisition(channel_).at(i);
        currentVoltage = (val * voltageStep) - (5 * voltageDiv);

        data.append(QPointF(currentTime, currentVoltage));
        currentTime += timeStep;

        if (currentTime == 5.0 * timeDiv) break;
    }

    QwtPlotCurve::setSamples(data);

    if (channel_ == A && state_.getFilterMode() == BANDPASS)
        processSamples();
    else
        emit plotReady((int)channel_, data);
}

// Processing function called to calculate and plot the points of the math
// channel.
void ChannelCurve::evaluate() {
    parser_ = new Parser();
    QVector<QPointF> result;

    if (parser_->checkSymbols(state_.getEquation()) != OK
            || parser_->parse(state_.getPlotPoints(A),
                              state_.getPlotPoints(B),
                              state_.getPlotPoints(F),
                              state_.getNoSamples(),
                              horizontalDivisions.at(state_.getTimeDiv()),
                              &result) != OK) {
        emit error("Expression could not be evaluated: " + parser_->error());
    } else {
        QwtPlotCurve::setSamples(result);

        emit plotReady((int)channel_, result);
    }

    delete parser_;
}

// Processing function to apply the filter and plot the points of the filter
// channel.
void ChannelCurve::filter() {
    QVector<QPointF> samples;
    QVector<QPointF> output;

    samples = state_.getPlotPoints(state_.getFilterChannel());

    for (int n = 0; n < samples.size(); n++) {
        double inputVal, outputVal;
        double filteredVal = 0;

        for (int i = 0; i < state_.getBTaps().size(); i++) {
            if (n - i < 0) {
                inputVal = 0;
            } else {
                inputVal = samples.at(n - i).y();
            }

            filteredVal += state_.getBTaps().at(i) * inputVal;
        }

        if (state_.getFilterType() == IIR) {
            for (int j = 1; j < state_.getATaps().size(); j++) {
                if (n - j < 0) {
                    outputVal = 0;
                } else {
                   outputVal = output.at(n - j).y();
                }

                filteredVal -= state_.getATaps().at(j) * outputVal;

            }

            filteredVal /= state_.getATaps().at(0);
        }

        output.append(QPointF(samples.at(n).x(), filteredVal));
    }

    QwtPlotCurve::setSamples(output);

    emit plotReady((int)channel_, output);
}

// Processing function called to process bandpass samples.
void ChannelCurve::processSamples() {

    QVector<QPointF> newSamples;

    // Lower the upsample frequency for higher timespans to limit
    // sample size to 10M.
    int idealFreq = timeDiv_ > 50 ? (MAX_FREQ / (timeDiv_ / 50)) : MAX_FREQ;

    int numSamples = (int)dataSize();
    double sampleFreq = (double)numSamples / (timeDiv_ / 100.0);
    int iR = (int)(idealFreq / sampleFreq);
    double highFreq = iR * sampleFreq;

    double currentTime = timeDiv_ * -5.0;
    double timeStep = (timeDiv_ * 10.0) / (double)(iR * numSamples);

    double* upSamples[1];
    upSamples[0] = new double[iR*numSamples];
    newSamples.reserve(numSamples * iR);

    for (int i = 0; i < iR*numSamples; i++) {
        if (i%iR == 0) {
            upSamples[0][i] = sample(i/iR).y();
        } else {
            upSamples[0][i] = 0.0;
        }
    }

    Dsp::Filter* f = new Dsp::FilterDesign <Dsp::Butterworth::Design::LowPass <10>, 1>;
    Dsp::Params params;
    params[0] = (int)highFreq; // sample rate
    params[1] = 10; // order
    params[2] = (highFreq / 2.0) / iR; // cutoff frequency
    f->setParams(params);
    f->process(iR*numSamples, upSamples);

    for (int i = 0; i < iR*numSamples; i++) {
        double carrier = iR*qSin((double)sampleFreq * (i/(double)highFreq) * (2.0 * 3.14159));
        upSamples[0][i] *= carrier;
    }

    Dsp::Filter* f2 = new Dsp::FilterDesign <Dsp::Butterworth::Design::BandPass <4>, 1>;
    Dsp::Params params2;
    params2[0] = (int)highFreq; // sample rate
    params2[1] = 4; // order
    params2[2] = 725000; // centre frequency
    params2[3] = 150000; // bandwidth

    f2->setParams (params2);
    // this will cause a runtime assertion
    f2->process (iR*numSamples, upSamples);


    for (int i = 0; i < iR*numSamples; i++) {
        newSamples.append(QPointF(currentTime, upSamples[0][i]));
        currentTime += timeStep;
    }

    QwtPlotCurve::setSamples(newSamples);

    emit plotReady((int)channel_, newSamples);

    delete[] upSamples[0];
}

// Function called after plotting to measure the voltage values of the curve.
void ChannelCurve::measureCurve() {

    if (isEmpty()) return;

    double voltageDiv = verticalDivisions.at(state_.getVoltageDiv(channel_));

    double vMax = sample(0).y();
    double vMin = sample(0).y();
    double vPp = 0.0;
    double vAvg = 0.0;
    double stdDev = 0.0;
    double total = 0.0;
    int numSamples = 0;

    for (int i = 0; i < (int)dataSize(); i++) {
        if (sample(i).y() > vMax) {
            vMax = sample(i).y();
        }
        if (sample(i).y() < vMin) {
            vMin = sample(i).y();
        }

        if (sample(i).y() <= voltageDiv * 5.0 && sample(i).y() >= voltageDiv * -5.0) {
            total += sample(i).y();
            numSamples++;
        }
    }

    vMax = qMin(vMax, voltageDiv * 5.0);
    vMin = qMax(vMin, voltageDiv * -5.0);
    vPp = vMax - vMin;
    vAvg = total / (double)(numSamples);

    for (int i = 0; i < (int)dataSize(); i++) {
        if (sample(i).y() < voltageDiv * 5.0 && sample(i).y() >= voltageDiv * -5.0) {
            stdDev += qPow(sample(i).y() - vAvg, 2);
        }
    }

    stdDev = qSqrt(stdDev / (double)(numSamples));

    emit measured((int)channel_, vMax, vMin, vPp, vAvg, stdDev);
}

// Processing function called after plotting to find the frequency of the
// curve.
void ChannelCurve::findFrequency() {

        int size = (int)dataSize();
        if (size == 0) {
            emit freqCalculated((int)channel_, 0.0);
            return;
        }

        double *samples = new double[size];

        double *fft = new double[size];

        for(int i = 0; i < size; i++) {
            samples[i] = sample(i).y();
        }

        fftw_plan plan = fftw_plan_r2r_1d(size, samples, fft, FFTW_R2HC, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
        fftw_execute(plan);

        double max = 0.0;
        double finalFreq = 0.0;
        bool peakFound = false;
        double peakVal = 0.0;

        double freqStep = 1 / (((timeDiv_ / 100.0) / (double)dataSize()) * size);
        double currentFreq = 0.0;

        for (int i = 0; i < (size / 2) + 1; i++) {

            fft[i] = qFabs(fft[i]);

            if (i > 0 && fft[i] > (max + 0.000001) && (!peakFound || fft[i] > 1.2 * peakVal)) {
                max = fft[i];
                finalFreq = currentFreq;
            }

            if ( i > 0 && fft[i] < (fft[i-1] - 0.000001) && finalFreq > 0.0 && finalFreq == currentFreq - freqStep) {
                peakFound = true;
                peakVal = fft[i-1];
            }

            currentFreq += freqStep;
        }

        fftw_destroy_plan(plan);

        delete [] samples;
        delete [] fft;

        emit freqCalculated((int)channel_, finalFreq);
}

// Override function to set the channels unique scale before calling
// QwtPlotCurve::draw.
void ChannelCurve::draw (QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& rect) const
{
    double minY = voltageDiv_ * -5.0;
    double maxY = voltageDiv_ * 5.0;
    QwtScaleMap newYMap (yMap);
    newYMap.setScaleInterval (minY, maxY);
    QwtPlotCurve::draw (painter, xMap, newYMap, rect);
}
