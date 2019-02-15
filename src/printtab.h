#ifndef __PRINTTAB_H__
#define __PRINTTAB_H__

class PrintJob;

class PrintTab: public QWidget {

    Q_OBJECT

public:

    PrintTab( QWidget* parent = nullptr );
    virtual ~PrintTab( ) override;

    PrintJob* printJob( ) const {
        return _printJob;
    }

    bool isPrintButtonEnabled( ) const {
        return printButton->isEnabled( );
    }

protected:

private:

    enum class BuildPlatformState {
        Extended,
        Retracting,
        Retracted,
        Extending,
    };

    PrintJob*          _printJob                 { };
    BuildPlatformState _buildPlatformState       { BuildPlatformState::Extended };

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

    QPushButton* _adjustBedHeightButton          { new QPushButton };
    QPushButton* _retractOrExtendButton          { new QPushButton };
    QPushButton* _moveUpButton                   { new QPushButton };
    QPushButton* _moveDownButton                 { new QPushButton };
    QHBoxLayout* _adjustmentsHBox                { new QHBoxLayout };
    QVBoxLayout* _adjustmentsVBox                { new QVBoxLayout };
    QGroupBox*   _adjustmentsGroup               { new QGroupBox   };

    QGridLayout* _layout                         { new QGridLayout };

signals:

    void printButtonClicked( );
    void adjustBedHeight( double const newHeight );
    void retractBuildPlatform( );
    void extendBuildPlatform( );
    void moveBuildPlatformUp( );
    void moveBuildPlatformDown( );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setPrintButtonEnabled( bool const value );

    void retractBuildPlatformComplete( bool const success );
    void extendBuildPlatformComplete( bool const success );
    void moveBuildPlatformUpComplete( bool const success );
    void moveBuildPlatformDownComplete( bool const success );

protected slots:

private slots:

    void exposureTime_editingFinished( );
    void exposureScaleFactorComboBox_currentIndexChanged( int index );
    void powerLevelSlider_valueChanged( int value );
    void printButton_clicked( bool checked );
    void _adjustBedHeightButton_clicked( bool checked );
    void _retractOrExtendButton_clicked( bool checked );
    void _moveUpButton_clicked( bool checked );
    void _moveDownButton_clicked( bool checked );

};

#endif // __PRINTTAB_H__
