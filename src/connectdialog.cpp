#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    setWindowTitle("Connect to Host");
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}

// Called when the IP address is specified to initiate connect.
void ConnectDialog::connect() {
    emit hostSelected(ui->ipEdit->text());
    accept();
}
