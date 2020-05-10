#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QtCore>
#include <QtWidgets>

class ProgressDialog: public QDialog
{
    Q_OBJECT

public:
    ProgressDialog(QWidget* parent = nullptr);
    void setMessage(const QString &message);
    void setProgress(int percentage);

protected:
    QLabel* _message;
    QProgressBar* _progress;
};

#endif // PROGRESSDIALOG_H
