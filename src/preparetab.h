#ifndef __PREPARETAB_H__
#define __PREPARETAB_H__

class PrintJob;
class SvgRenderer;

class PrepareTab: public QWidget {

    Q_OBJECT

public:

    PrepareTab( QWidget* parent = nullptr );
    virtual ~PrepareTab( ) override;

    bool isSliceButtonEnabled( ) const { return sliceButton->isEnabled( ); }

    void setSliceButtonEnabled( bool value ) {
        sliceButton->setEnabled( value );
    }

protected:

    virtual void showEvent( QShowEvent* ev ) override;

private:

    QProcess*              slicerProcess          { };
    SvgRenderer*           svgRenderer            { };
    PrintJob*              _printJob              { };

    QLabel*                layerThicknessLabel    { new QLabel      };
    QComboBox*             layerThicknessComboBox { new QComboBox   };

    QLabel*                sliceProgressLabel     { new QLabel      };
    QLabel*                sliceStatus            { new QLabel      };
    QLabel*                renderProgressLabel    { new QLabel      };
    QLabel*                renderStatus           { new QLabel      };
    QLabel*                currentSliceLabel      { new QLabel      };
    QLabel*                currentSliceImage      { new QLabel      };
    QVBoxLayout*           currentSliceLayout     { new QVBoxLayout };

    QVBoxLayout*           optionsLayout          { new QVBoxLayout };
    QWidget*               optionsContainer       { new QWidget     };
    QPushButton*           sliceButton            { new QPushButton };

    QGroupBox*             _prepareGroup          { new QGroupBox   };

    QGridLayout*           _layout                { new QGridLayout };

    std::function<void( )> _initialShowEventFunc;

signals:

    void sliceStarted( );
    void sliceComplete( bool const success );
    void renderStarted( );
    void renderComplete( bool const success );

public slots:

    void setPrintJob( PrintJob* printJob );

protected slots:

private slots:

    void layerThicknessComboBox_currentIndexChanged( int index );
    void sliceButton_clicked( bool );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void svgRenderer_progress( int const currentLayer );
    void svgRenderer_done( int const totalLayers );

};

#endif // __PREPARETAB_H__
