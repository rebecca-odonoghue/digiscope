#ifndef EQUATIONDEFINITIONS_H
#define EQUATIONDEFINITIONS_H

#include <QString>

// Definitions relevant to the math channel.

const double PI = 3.14159;

const double E = 2.71828;

typedef enum {OP, CONST} ExpType;

typedef enum {PLUS, MINUS, TIMES, DIVIDE, EXP} Op;

typedef enum {OK, ERROR} Error;

const QString validSymbols = "ABF+-*/^pe.()";

#endif // EQUATIONDEFINITIONS_H
