#ifndef __BEDHEIGHTADJUSTMENT_H__
#define __BEDHEIGHTADJUSTMENT_H__

class BedHeightAdjustmentDialog: public QDialog {

    Q_OBJECT

public:

    BedHeightAdjustmentDialog( QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags( ) );
    virtual ~BedHeightAdjustmentDialog( ) override;

    double newBedHeight( ) const { return _newBedHeight; }

protected:

    virtual void resizeEvent( QResizeEvent* event ) override;

private:

    QLabel*      _bedHeightLabel                 { new QLabel      };
    QLabel*      _bedHeightValue                 { new QLabel      };
    QHBoxLayout* _bedHeightLabelLayout           { new QHBoxLayout };
    QDial*       _bedHeightDial                  { new QDial       };
    QLabel*      _bedHeightDialTopLabel          { new QLabel      };
    QLabel*      _bedHeightDialLeftLabel         { new QLabel      };
    QLabel*      _bedHeightDialRightLabel        { new QLabel      };
    QHBoxLayout* _bedHeightDialLowerLabelsLayout { new QHBoxLayout };
    QPushButton* _okButton                       { new QPushButton };
    QPushButton* _cancelButton                   { new QPushButton };
    QHBoxLayout* _buttonsHBox                    { new QHBoxLayout };
    QFrame*      _frame                          { new QFrame      };
    QVBoxLayout* _layout                         { new QVBoxLayout };
    QGridLayout* _dialogLayout                   { new QGridLayout };

    double       _newBedHeight                   {                 };

signals:

public slots:

protected slots:

private slots:

    void _bedHeightDial_valueChanged( int value );

};

#endif // __BEDHEIGHTADJUSTMENT_H__
