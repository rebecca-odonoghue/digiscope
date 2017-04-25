#ifndef EQUATIONDIALOG_H
#define EQUATIONDIALOG_H

#include <QDialog>

namespace Ui {
class EquationDialog;
}

// Class that handles the functionality of the 'Enter Equation' dialog.
class EquationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EquationDialog(QString, QWidget *parent = 0);
    ~EquationDialog();

private:
    Ui::EquationDialog *ui;

public slots:
    void plot();

signals:
    void equationEntered(QString);
};

#endif // EQUATIONDIALOG_H
