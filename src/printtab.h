#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

class PrintJob;

class PrintTab: public QWidget {

    Q_OBJECT

public:

    PrintTab( QWidget* parent = nullptr );
    virtual ~PrintTab( ) override;

    bool isPrintButtonEnabled( ) {
        return printButton->isEnabled( );
    }

    void setPrintButtonEnabled( bool value ) {
        printButton->setEnabled( value );
    }

protected:

private:

    QLabel*      exposureTimeLabel               { new QLabel      };
    QLineEdit*   exposureTime                    { new QLineEdit   };
    QLabel*      exposureScaleFactorLabel        { new QLabel      };
    QComboBox*   exposureScaleFactorComboBox     { new QComboBox   };
    QLabel*      powerLevelLabel                 { new QLabel      };
    QLabel*      powerLevelValue                 { new QLabel      };
    QHBoxLayout* powerLevelValueLayout           { new QHBoxLayout };
    QWidget*     powerLevelValueContainer        { new QWidget     };
    QSlider*     powerLevelSlider                { new QSlider     };
    QLabel*      powerLevelSliderLeftLabel       { new QLabel      };
    QLabel*      powerLevelSliderRightLabel      { new QLabel      };
    QHBoxLayout* powerLevelSliderLabelsLayout    { new QHBoxLayout };
    QWidget*     powerLevelSliderLabelsContainer { new QWidget     };
    QVBoxLayout* optionsLayout                   { new QVBoxLayout };
    QWidget*     optionsContainer                { new QWidget     };
    QPushButton* printButton                     { new QPushButton };
    QWidget*     _placeHolder                    { new QWidget     };
    QGridLayout* _layout                         { new QGridLayout };
    PrintJob*    _printJob                       {                 };

signals:

    void printButtonClicked( );

public slots:

    void setPrintJob( PrintJob* printJob );

protected slots:

private slots:

    void exposureTime_editingFinished( );
    void exposureScaleFactorComboBox_currentIndexChanged( int index );
    void powerLevelSlider_valueChanged( int value );
    void printButton_clicked( bool checked );

};

#endif // __PRINTTAB_H__
