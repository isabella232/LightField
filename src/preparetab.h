#ifndef __PREPARETAB_H__
#define __PREPARETAB_H__

class Hasher;
class PrintJob;
class Shepherd;
class SvgRenderer;

class PrepareTab: public QWidget {

    Q_OBJECT

public:

    PrepareTab( QWidget* parent = nullptr );
    virtual ~PrepareTab( ) override;

    bool      isPrepareButtonEnabled( ) const { return _prepareButton->isEnabled( ); }
    bool      isSliceButtonEnabled( )   const { return sliceButton->isEnabled( );    }
    Shepherd* shepherd( )               const { return _shepherd;                    }

protected:

    virtual void showEvent( QShowEvent* ev ) override;

private:

    PrintJob*              _printJob                   { };
    Shepherd*              _shepherd                   { };
    QProcess*              slicerProcess               { };
    SvgRenderer*           svgRenderer                 { };
    Hasher*                _hasher                     { };
    int                    _visibleLayer               { };
    int                    _renderedLayers             { };
    bool                   _preSliced                  { };

    QLabel*                layerThicknessLabel         { new QLabel       };
    QRadioButton*          layerThickness50Button      { new QRadioButton };
    QRadioButton*          layerThickness100Button     { new QRadioButton };
    QVBoxLayout*           layerThicknessButtonsLayout { new QVBoxLayout  };

    QLabel*                sliceStatusLabel            { new QLabel       };
    QLabel*                sliceStatus                 { new QLabel       };
    QLabel*                imageGeneratorStatusLabel   { new QLabel       };
    QLabel*                imageGeneratorStatus        { new QLabel       };

    QGroupBox*             _prepareGroup               { new QGroupBox    };
    QLabel*                _prepareMessage             { new QLabel       };
    QProgressBar*          _prepareProgress            { new QProgressBar };
    QPushButton*           _prepareButton              { new QPushButton  };
    QVBoxLayout*           _prepareLayout              { new QVBoxLayout  };

    QVBoxLayout*           optionsLayout               { new QVBoxLayout  };
    QWidget*               optionsContainer            { new QWidget      };
    QPushButton*           sliceButton                 { new QPushButton  };

    QGroupBox*             currentSliceGroup           { new QGroupBox    };
    QLabel*                currentSliceImage           { new QLabel       };
    QVBoxLayout*           currentSliceLayout          { new QVBoxLayout  };

    QPushButton*           navigateFirst               { new QPushButton  };
    QPushButton*           navigatePrevious            { new QPushButton  };
    QLabel*                navigateCurrentLabel        { new QLabel       };
    QPushButton*           navigateNext                { new QPushButton  };
    QPushButton*           navigateLast                { new QPushButton  };
    QHBoxLayout*           navigationLayout            {                  };

    QGridLayout*           _layout                     { new QGridLayout  };

    std::function<void( )> _initialShowEventFunc;

    void _initialShowEvent( );
    bool _checkPreSlicedFiles( );
    void _setNavigationButtonsEnabled( bool const enabled );
    void _showLayerImage( int const layer );

signals:

    void sliceStarted( );
    void sliceComplete( bool const success );
    void renderStarted( );
    void renderComplete( bool const success );

    void preparePrinterStarted( );
    void preparePrinterComplete( bool const success );

    void alreadySliced( );

public slots:

    void setPrepareButtonEnabled( bool const enabled );
    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );
    void setSliceButtonEnabled( bool const enabled );

    void resetState( );

    void modelSelected( );

protected slots:

private slots:

    void layerThickness50Button_clicked( bool );
    void layerThickness100Button_clicked( bool );

    void navigateFirst_clicked( bool );
    void navigatePrevious_clicked( bool );
    void navigateNext_clicked( bool );
    void navigateLast_clicked( bool );

    void sliceButton_clicked( bool );

    void hasher_resultReady( QString const hash );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void svgRenderer_layerCount( int const totalLayers );
    void svgRenderer_layerComplete( int const currentLayer );
    void svgRenderer_done( bool const success );

    void _prepareButton_clicked( bool );
    void _shepherd_homeComplete( bool const success );
    void _adjustBuildPlatform_complete( bool );
    void _shepherd_resinLoadMoveToComplete( bool const success );

};

#endif // __PREPARETAB_H__
