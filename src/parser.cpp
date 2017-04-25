#include "parser.h"

/* A class to parse and store an equation, that can then be solved
 * for given values of A, B and F. Equations are parsed utilising the
 * following grammar (BNF) to ensure proper order of operations.
 *
 * Expression -> Term {(PLUS|MINUS) Term}
 * Term -> Base {(TIMES|DIVIDE) Base}
 * Base -> Unary [POWER Unary]
 * Unary -> [MINUS] Factor
 * Factor -> LPAREN Expression RPAREN | number
 */
Parser::Parser()
{

}

// Checks the symbols of the equation and returns OK if the
// equation is valid.
Error Parser::checkSymbols(QString exp) {
    qDebug() << "Parser::checkSymbols";
    tokens_.clear();
    //resultPlot_->clear();
    error_ = "";
    QStack<QChar> brackets;
    QString currentToken = "";
    requiresA_ = false;
    requiresB_ = false;
    requiresF_ = false;

    for (int i = 0; i < exp.length(); i++) {
        QChar c = exp.at(i);

        if (c.isSpace())
            continue;

        if (!c.isDigit() && !validSymbols.contains(c)) {
            error_ = "invalid symbol " + QString(c);
            return ERROR;
        }

        if (c == 'A') requiresA_ = true;
        else if (c == 'B') requiresB_ = true;
        else if (c == 'F') requiresF_ = true;

        if (c == '(') {
            brackets.push(c);
        } else if (c == ')') {
            if (brackets.isEmpty()) {
                error_ = "missing opening bracket.";
                return ERROR;
            }

            brackets.pop();
        }

        if (c == '.' || c.isDigit()) {
            currentToken += c;

        } else if (c == 'p') {
            if (currentToken != "") {
                tokens_.append(QString(currentToken));
                currentToken = "";
            }

            i++;

            if (i == exp.length() || exp.at(i) != 'i') {
                error_ = "character " + QString::number(i) + "is not a valid symbol.";
                return ERROR;
            }


            tokens_.append("pi");

        } else {

            if (currentToken != "") {
                tokens_.append(currentToken);
                currentToken = "";
            }

            tokens_.append(QString(c));
        }

    }

    if (currentToken != "")
        tokens_.append(QString(currentToken));

    if (!brackets.isEmpty()) {
        error_ = "missing closing bracket.";
        return ERROR;
    }
    qDebug() << "Finished";
    return OK;
}

// Parses the curve for all the points on the provided curves.
Error Parser::parse(QVector<QPointF> a, QVector<QPointF> b,
                    QVector<QPointF> f, quint16 noSamples,
                    double timeDiv, QVector<QPointF> *result) {
    qDebug() << "Parser::parse";
    Error error;
    error_ = "";
    plotA_ = a;
    plotB_ = b;
    plotF_ = f;
    currentAVal_ = 0;
    currentBVal_ = 0;
    currentFVal_ = 0;
    //resultPlot_->clear();
    resultPlot_ = result;

    if ((requiresA_ && plotA_.isEmpty())
            || (requiresB_ && plotB_.isEmpty())
            || (requiresF_ && plotF_.isEmpty()) ) {
        error_ = "cannot use values of a plot with no data.";
        return ERROR;
    }

    int actualSamples;

    if (requiresA_ && plotA_.size() < (timeDiv * 200000) && plotA_.size() < 10000000) {
        actualSamples = plotA_.size();
    } else if (requiresB_) {
        actualSamples = plotB_.size();
    } else if (requiresF_) {
        actualSamples = plotF_.size();
    } else {
        actualSamples = noSamples;
    }

    int sampleDiff = noSamples - actualSamples;

    double currentTime = -5.0 * timeDiv;
    double timeStep = 10.0 * timeDiv / ((double)noSamples - 1.0);

    currentTime += sampleDiff * timeStep;

    for (int i = 0; i < noSamples; i++) {
        if (currentTime > 5.0 * timeDiv) break;

        if (requiresA_) {
            if (i < plotA_.size() && plotA_.at(i).x() == currentTime) {
                currentAVal_ = plotA_.at(i).y();
            }
            else {
                //qDebug() << "x: " << QString::number(plotA_.at(i).x(), 'f', 8) << " currentTime: " << QString::number(currentTime, 'f', 8);
                currentAVal_ = approxVoltage(A, i, currentTime);
            }
        }
        if (requiresB_) {
            if (i < plotB_.size() && plotB_.at(i).x() == currentTime)
                currentBVal_ = plotB_.at(i).y();
            else
                currentBVal_ = approxVoltage(B, i, currentTime);
        }
        if (i < plotF_.size() && requiresF_) {
            if (plotF_.at(i).x() == currentTime)
                currentFVal_ = plotF_.at(i).y();
            else
                currentFVal_ = approxVoltage(F, i, currentTime);
        }

        error = parse(currentTime);
        if (error != OK) break;
        currentTime += timeStep;
    }

    return error;
}

// Begins parsing the provided equation with 3 point values, then stores the
// result in the vector of plot points.
Error Parser::parse(double timeVal) {
    currentToken_ = 0;
    double result = parseExp();
    if (currentToken_ < 0 || currentToken_ != tokens_.count()) {
        if (error_ == "")
            error_ = "because it can't.";
        return ERROR;
    }

    resultPlot_->append(QPointF(timeVal, result));
    return OK;
}

// Getter function returns the vector of points to plot on the curve.
QVector<QPointF> Parser::plotData() {
    return *resultPlot_;
}

// If parse returns ERROR, this function will return an error message.
QString Parser::error() {
    return error_;
}

// Returns true if the equation contains 'A'.
bool Parser::requiresA() {
    return requiresA_;
}

// Returns true if the equation contains 'B'.
bool Parser::requiresB() {
    return requiresB_;
}

// Returns true if the equation contains 'F'.
bool Parser::requiresF() {
    return requiresF_;
}

// Parses an expression of the form:
// Expression -> Term {(PLUS|MINUS) Term}
double Parser::parseExp() {

    double result = 0.0;

    if (currentToken_ == -1)
        return result;

    result = parseTerm();

    while(currentToken_ >= 0 && currentToken_ < tokens_.count()
          && QString("+-").contains(tokens_.at(currentToken_))) {

        QString op = tokens_.at(currentToken_);
        currentToken_++;

        if (op == "+")
            result += parseTerm();
        else if (op == "-")
            result -= parseTerm();
        else {
            error_ = tokens_.at(currentToken_) + " is incorrectly used.";
            currentToken_ = -1;
        }
    }

    return result;
}

// Parses a term of the form:
// Term -> Base {(TIMES|DIVIDE) Base}
double Parser::parseTerm() {

    double result = 0.0;

    if (currentToken_ == -1)
        return result;

    result = parseBase();

    while(currentToken_ >= 0 && currentToken_ < tokens_.count()
          && QString("*/").contains(tokens_.at(currentToken_))) {

        QString op = tokens_.at(currentToken_);

        currentToken_++;

        if (op == "*")
            result *= parseBase();
        else if (op == "/" && tokens_.at(currentToken_) != "0")
            result /= parseBase();
        else {
            error_ = tokens_.at(currentToken_) + " is incorrectly used.";
            currentToken_ = -1;
        }

    }

    return result;
}

// Parses a Base of the form:
// Base -> Unary [POWER Unary]
double Parser::parseBase() {

    double result = 0.0;

    if (currentToken_ == -1)
        return result;

    result = parseUnary();

    if (currentToken_ >= 0 && currentToken_ < tokens_.count()
            && tokens_.at(currentToken_) == "^") {

        currentToken_++;

        result = qPow(result, parseUnary());
    }

    return result;
}

// Parses a Unary of the form:
// Unary -> [MINUS] Factor
double Parser::parseUnary() {

    double result = 0.0;

    if (currentToken_ < 0)
        return result;

    if (currentToken_ < tokens_.count() && tokens_.at(currentToken_) == "-") {
        currentToken_++;

        result = -1.00 * parseFactor();
    } else {
        result = parseFactor();
    }

    return result;
}

// Parses a factor of the form:
// Factor -> LPAREN Expression RPAREN | number
double Parser::parseFactor() {

    double result = 0.0;

    if(currentToken_ == tokens_.count()) {
        error_ = "missing rvalue.";
        currentToken_ = -1;
        return result;
    }

    bool isDouble = false;
    double value = tokens_.at(currentToken_).toDouble(&isDouble);

    if (isDouble)
        result = value;
    else if (tokens_.at(currentToken_) == "(") {
        currentToken_++;
        result = parseExp();

        if(currentToken_ == -1)
            return result;
    }
    else if (tokens_.at(currentToken_) == "A")
        result = currentAVal_;
    else if (tokens_.at(currentToken_) == "B")
        result = currentBVal_;
    else if (tokens_.at(currentToken_) == "F")
        result = currentFVal_;
    else if (tokens_.at(currentToken_) == "pi")
        result = PI;
    else if (tokens_.at(currentToken_) == "e")
        result = E;
    else {
        error_ = tokens_.at(currentToken_) + " is incorrectly used.";
        currentToken_ = -1;
        return result;
    }

    currentToken_++;

    return result;
}

// Returns true if the provided time is within the start and end time of the
// channel's plotted data.
bool Parser::withinBounds(Channel channel, double time) {
    bool withinBounds = true;
    switch(channel) {
        case A:
            if (time < plotA_.at(0).x() || time > plotA_.at(plotA_.size() - 1).x()) {
                withinBounds = false;
            }
            break;
        case B:
            if (time < plotB_.at(0).x() || time > plotB_.at(plotB_.size() - 1).x()) {
                withinBounds = false;
            }
            break;
        case F:
            if (time < plotF_.at(0).x() || time > plotF_.at(plotF_.size() - 1).x()) {
                withinBounds = false;
            }
            break;
        case M:
            break;
    }
    return withinBounds;
}

// Given a channel, index and time, returns the voltage value on that curve
// at the given time in the most efficient way possible. If withinBounds
// returns false for this channel and time, approxVoltage returns 0;
double Parser::approxVoltage(Channel channel, int index, double time) {
    if (!withinBounds(channel, time)) return 0.0;
    double voltage = 0.0;
    switch(channel) {
        case A:
            if (index < plotA_.size() && plotA_.at(index).x() == time)
                return plotA_.at(index).y();
            for (int i = 0; i < plotA_.size(); i++) {
                double timeVal = plotA_.at(i).x();
                if (timeVal == time) {
                    return plotA_.at(i).y();
                } else if (timeVal > time) {
                    double v1 = plotA_.at(i - 1).y();
                    double v2 = plotA_.at(i).y();
                    voltage = v1 + ((v2 - v1) * (time - plotA_.at(i - 1).x()) / (timeVal - plotA_.at(i - 1).x()));
                    //qDebug() << voltage;
                    break;
                }
            }
            break;
        case B:
            if (index < plotB_.size() && plotB_.at(index).x() == time)
                return plotB_.at(index).y();
            for (int i = 0; i < plotB_.size(); i++) {
                double timeVal = plotB_.at(i).x();
                if (timeVal == time) {
                    return plotB_.at(i).y();
                } else if (timeVal > time) {
                    double v1 = plotB_.at(i - 1).y();
                    double v2 = plotB_.at(i).y();
                    voltage = v1 + ((v2 - v1) * (time - plotB_.at(i - 1).x()) / (timeVal - plotB_.at(i - 1).x()));
                    break;
                }
            }
            break;
        case F:
            if (index < plotF_.size() && plotF_.at(index).x() == time)
                return plotF_.at(index).y();
            for (int i = 0; i < plotF_.size(); i++) {
                double timeVal = plotF_.at(i).x();
                if (timeVal == time) {
                    return plotF_.at(i).y();
                } else if (timeVal > time) {
                    double v1 = plotF_.at(i - 1).y();
                    double v2 = plotF_.at(i).y();
                    voltage = v1 + ((v2 - v1) * (time - plotF_.at(i - 1).x()) / (timeVal - plotF_.at(i - 1).x()));
                    break;
                }
            }
            break;
        case M:
            break;
    }
    return voltage;
}
