#ifndef __PREPARETAB_H__
#define __PREPARETAB_H__

class PrintJob;
class Shepherd;
class SvgRenderer;

class PrepareTab: public QWidget {

    Q_OBJECT

public:

    PrepareTab( QWidget* parent = nullptr );
    virtual ~PrepareTab( ) override;

    Shepherd* shepherd( ) const { return _shepherd; }

    bool isPrepareButtonEnabled( ) const { return _prepareButton->isEnabled( ); }
    bool   isSliceButtonEnabled( ) const { return    sliceButton->isEnabled( ); }

protected:

    virtual void showEvent( QShowEvent* ev ) override;

private:

    PrintJob*              _printJob              { };
    Shepherd*              _shepherd              { };
    QProcess*              slicerProcess          { };
    SvgRenderer*           svgRenderer            { };

    QLabel*                layerThicknessLabel    { new QLabel       };
    QComboBox*             layerThicknessComboBox { new QComboBox    };

    QLabel*                sliceProgressLabel     { new QLabel       };
    QLabel*                sliceStatus            { new QLabel       };
    QLabel*                renderProgressLabel    { new QLabel       };
    QLabel*                renderStatus           { new QLabel       };
    QLabel*                currentSliceLabel      { new QLabel       };
    QLabel*                currentSliceImage      { new QLabel       };
    QVBoxLayout*           currentSliceLayout     { new QVBoxLayout  };

    QVBoxLayout*           optionsLayout          { new QVBoxLayout  };
    QWidget*               optionsContainer       { new QWidget      };
    QPushButton*           sliceButton            { new QPushButton  };

    QGroupBox*             _prepareGroup          { new QGroupBox    };
    QLabel*                _prepareMessage        { new QLabel       };
    QProgressBar*          _prepareProgress       { new QProgressBar };
    QPushButton*           _prepareButton         { new QPushButton  };
    QVBoxLayout*           _prepareLayout         { new QVBoxLayout  };

    QGridLayout*           _layout                { new QGridLayout  };

    int                    _maxSliceImageWidth    { -1               };

    std::function<void( )> _initialShowEventFunc;

    void _initialShowEvent( );

signals:

    void sliceStarted( );
    void sliceComplete( bool const success );
    void renderStarted( );
    void renderComplete( bool const success );

    void preparePrinterComplete( bool const success );

public slots:

    void setPrepareButtonEnabled( bool const value );
    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );
    void setSliceButtonEnabled( bool const value );

protected slots:

private slots:

    void layerThicknessComboBox_currentIndexChanged( int index );
    void sliceButton_clicked( bool );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void svgRenderer_progress( int const currentLayer );
    void svgRenderer_done( int const totalLayers );

    void _prepareButton_clicked( bool );
    void _sendHome_complete( bool const success );
    void _adjustBuildPlatform_complete( bool );
    void _sendResinLoadMove_complete( bool const success );
    void _loadPrintSolution_complete( bool );
    void _sendExtend_complete( bool const success );

};

#endif // __PREPARETAB_H__
