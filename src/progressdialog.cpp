#include <QtCore>
#include <QtWidgets>
#include "utils.h"
#include "progressdialog.h"

ProgressDialog::ProgressDialog(QWidget* parent):
    QDialog(parent)
{
    _message = new QLabel;
    _message->setText("Please wait");
    _message->setAlignment(Qt::AlignCenter);

    _progress = new QProgressBar;
    _progress->setRange(0, 100);

    setLayout(WrapWidgetsInVBox(_message, _progress));
    setFixedSize(320, 100);
}

void ProgressDialog::setMessage(const QString &message)
{
    _message->setText(message);
}

void ProgressDialog::setProgress(int percentage)
{
    _progress->setValue(percentage);
}
