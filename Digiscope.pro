#-------------------------------------------------
#
# Project created by QtCreator 2016-03-11T17:56:22
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Digiscope
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/plot.cpp \
    src/channelcurve.cpp \
    src/communicationhandler.cpp \
    src/state.cpp \
    src/connectdialog.cpp \
    src/equationdialog.cpp \
    src/parser.cpp \
    src/voltagepicker.cpp

HEADERS  += \
    include/mainwindow.h \
    include/plot.h \
    include/channelcurve.h \
    include/communicationhandler.h \
    include/state.h \
    include/connectdialog.h \
    include/equationdialog.h \
    include/parser.h \
    include/equationdefinitions.h \
    include/statedefinitions.h \
    include/voltagepicker.h

FORMS    += \
    forms/mainwindow.ui \
    forms/connectdialog.ui \
    forms/equationdialog.ui \
    forms/about.ui

RESOURCES += \
    resources/images.qrc

INCLUDEPATH += include

CONFIG(debug, debug|release) {
    DESTDIR = debug
} else {
    DESTDIR = release
}

OBJECTS_DIR = $$DESTDIR/obj
MOC_DIR = $$DESTDIR/moc
RCC_DIR = $$DESTDIR/qrc
UI_DIR = $$DESTDIR/ui

# Windows specific compiler actions
win32 {
    DEFINES += OS_WIN32

   # CONFIG += qwt

    CONFIG(release, debug|release) {
        LIBS += -LD:/Qwt-6.1.2/lib/ -lqwt
    } else {
        CONFIG(debug, debug|release): LIBS += -LD:/Qwt-6.1.2/lib/ -lqwtd
    }

    LIBS += -lws2_32 -lwsock32 \
        -LD:/fftw-3.3.4-dll32/ -llibfftw3-3 \
        -LD:/fftw-3.3.4-dll32/ -llibfftw3f-3 \
        -LD:/fftw-3.3.4-dll32/ -llibfftw3l-3 \
        -LD:/DSPFilters/ -lDSPFilters

    INCLUDEPATH += \
        D:/fftw-3.3.4-dll32 \
        D:/Qwt-6.1.2/include \
        D:/DSPFilters/include


    DEPENDPATH += \
        D:/fftw-3.3.4-dll32 \
        D:/Qwt-6.1.2/include \
        D:/DSPFilters/include
}

# OS X specific compiler actions
macx {
    include (/usr/local/qwt-6.1.2/features/qwt.prf)

    LIBS += -F/usr/local/qwt-6.1.2/lib/ -framework qwt \
        -L/usr/local/lib/ -lfftw3 \
        -L/Users/Bec/Library/DSPFilters/ -lDSPFiltersMacOS

    INCLUDEPATH += /usr/local/qwt-6.1.2 \
        /usr/local/include \
        /Users/Bec/Library/DSPFilters/include

    DEPENDPATH += /usr/local/qwt-6.1.2 \
        /usr/local/include \
        /Users/Bec/Library/DSPFilters/include

    PRE_TARGETDEPS += /usr/local/lib/libfftw3.a \
        /Users/Bec/Library/DSPFilters/libDSPFiltersMacOS.a
}

