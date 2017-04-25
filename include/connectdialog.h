#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectDialog;
}

// Class that handles the functionality of the 'Connect to Host' dialog.
class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();

private:
    Ui::ConnectDialog *ui;

public slots:
    void connect();

signals:
    void hostSelected(QString);
};

#endif // CONNECTDIALOG_H
