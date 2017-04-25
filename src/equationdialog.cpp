#include "equationdialog.h"
#include "ui_equationdialog.h"

EquationDialog::EquationDialog(QString equation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EquationDialog)
{
    ui->setupUi(this);
    setWindowTitle("Enter Equation to Plot");
    ui->equationEdit->setText(equation);
}

EquationDialog::~EquationDialog()
{
    delete ui;
}

// Called when equation is specified to initiate math channel plotting.
void EquationDialog::plot() {
    accept();
    emit equationEntered(ui->equationEdit->text());
}
