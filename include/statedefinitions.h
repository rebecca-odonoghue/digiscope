#ifndef STATEDEFINITIONS_H
#define STATEDEFINITIONS_H

#include <QVector>
#include <QString>
#include <QList>

// Global definitions.

enum Channel {A, B, F, M};

enum VoltageResolution {TWELVE_BIT, EIGHT_BIT};

static const QVector<double> verticalDivisions = {2.0, 1.0, 0.5, 0.2, 0.1,
                                                  0.05, 0.02};

static const QVector<double> horizontalDivisions = {1000, 500, 200, 100, 50,
                                                    20, 10, 5, 2, 1, 0.5, 0.2,
                                                    0.1, 0.05, 0.02, 0.01,
                                                    0.005, 0.002, 0.001};

enum VoltageCoupling {DC, AC};

enum FilterType {IIR, FIR};

enum FilteringMode {LOWPASS, BANDPASS};

enum TriggerState {ARMED, TRIGGERED, STOPPED};

enum TriggerMode {AUTO, NORMAL, SINGLE};

enum TriggerType {RISING, FALLING, LEVEL};

enum FunctionGenState {ON, OFF};

enum FunctionWaveType {SINE, SQUARE, TRIANGLE, RAMP, NOISE};

static const QList<QString> waveNames = {"sine wave", "square wave", "triangle wave",
                                         "ramp wave", "random noise"};

static const QVector<double> functionVoltages = {0.1, 0.2, 0.5, 1, 2};

#endif // STATEDEFINITIONS_H
