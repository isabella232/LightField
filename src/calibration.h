#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

class CalibrationDialog: public QDialog {

    Q_OBJECT

public:

    CalibrationDialog( QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags( ) );
    virtual ~CalibrationDialog( ) override;

protected:

private:

    QPushButton* _prevButton   { new QPushButton };
    QPushButton* _nextButton   { new QPushButton };
    QPushButton* _cancelButton { new QPushButton };
    QHBoxLayout* _buttonsHBox  { new QHBoxLayout };
    QVBoxLayout* _layout       { new QVBoxLayout };

signals:

    void dummy( );

public slots:

protected slots:

private slots:

    void _prevButton_clicked( bool );
    void _nextButton_clicked( bool );
    void _cancelButton_clicked( bool );

};

#endif // __CALIBRATION_H__
