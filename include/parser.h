#ifndef PARSER_H
#define PARSER_H

#include <QChar>
#include <QString>
#include <QStringList>
#include <QStack>
#include <QDebug>
#include <QVector>
#include <QPointF>
#include <QtMath>

#include <equationdefinitions.h>
#include <statedefinitions.h>

// Class used for parsing and solving an equation for the given plots.
class Parser
{
public:
    Parser();
    Error checkSymbols(QString);
    bool requiresA();
    bool requiresB();
    bool requiresF();
    Error parse(QVector<QPointF>, QVector<QPointF>, QVector<QPointF>,
                quint16, double, QVector<QPointF>*);
    QVector<QPointF> plotData();
    QString error();

private:
    // Parsing functions.
    Error parse(double);
    double parseExp();
    double parseTerm();
    double parseBase();
    double parseUnary();
    double parseFactor();

    // Functions to provide the correct voltage for a given time.
    bool withinBounds(Channel, double);
    double approxVoltage(Channel, int, double);
    QString error_;
    QStringList tokens_;
    int currentToken_;
    QVector<QPointF>* resultPlot_;
    QVector<QPointF> plotA_, plotB_, plotF_;
    bool requiresA_, requiresB_, requiresF_;
    double currentAVal_, currentBVal_, currentFVal_;
};

#endif // PARSER_H
