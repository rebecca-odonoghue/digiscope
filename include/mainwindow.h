#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QComboBox>
#include <QButtonGroup>
#include <QActionGroup>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QThread>
#include <QtMath>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>

#include <plot.h>
#include <connectdialog.h>
#include <equationdialog.h>
#include <communicationhandler.h>
#include <state.h>

#include <ui_about.h>

#ifdef OS_WIN32
#define IS_WINDOWS true
#endif

#ifndef OS_WIN32
#define IS_WINDOWS false
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void generateOscilloscopeToolbar();
    void createPlot();
    void connectDials();
    void closeEvent(QCloseEvent*);
    void loadFilter(QString);
    void updateFunctionGenText();
    Plot *plot_;
    CommunicationHandler *comHandler_;
    QLabel *hostStatus_, *hostStatusIcon_, *deviceStatus_;
    QComboBox* bitModeSelect_;
    QSpinBox* noSamplesSelect_;
    State* state_;
    QLabel *filterInfo_, *equationInfo_, *functionGenInfo_, *sampleRateLabel_;
    QMessageBox* voltageError_;
    QThread* plotThread_;

public slots:
    void setAVisible(int);
    void setBVisible(int);
    void setFVisible(int);
    void setMVisible(int);
    void filterChannelChanged(bool);
    void checkRearm(int);
    void connectToHost();
    void enterEquation();
    void selectFilter();
    void catchError(QString);

private slots:
    void connected(QString);
    void disconnected();
    void voltageError();
    void numSamplesReady();
    void numSamplesChanged(quint16);
    void bitModeChanged(int);
    void aFilterModeChanged(int);
    void couplingChanged(int, int);
    void vRangeChanged(int, int);
    void channelOffsetChanged(int, quint16);
    void hRangeChanged(int);
    void triggerStatusChanged(int);
    void triggerChannelChanged(int);
    void triggerModeChanged(int);
    void triggerTypeChanged(int);
    void triggerValueChanged(quint16);
    void functionStateChanged(bool);
    void functionWaveChanged(int);
    void functionVoltageChanged(int);
    void functionOffsetChanged(quint16);
    void functionFreqChanged(quint16);
    void newMeasurements(int, double, double, double, double, double);
    void newFrequency(int, double);
    void channelHidden(int);
    void about();

};

#endif // MAINWINDOW_H
