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

private:

    QProcess*          slicerProcess          { };
    SvgRenderer*       svgRenderer            { };

    QLabel*            layerThicknessLabel    { new QLabel      };
    QComboBox*         layerThicknessComboBox { new QComboBox   };
    QVBoxLayout*       optionsLayout          { new QVBoxLayout };
    QWidget*           optionsContainer       { new QWidget     };
    QPushButton*       sliceButton            { new QPushButton };
    QPushButton*       calibrateButton        { new QPushButton };

    QLabel*            sliceProgress          { new QLabel      };
    QLabel*            sliceProgressLabel     { new QLabel      };
    QLabel*            renderProgress         { new QLabel      };
    QLabel*            renderProgressLabel    { new QLabel      };
    QLabel*            currentSliceLabel      { new QLabel      };
    QLabel*            currentSliceDisplay    { new QLabel      };
    QVBoxLayout*       currentSliceLayout     { new QVBoxLayout };
    QWidget*           currentSliceContainer  { new QWidget     };
    QGridLayout*       _layout                { new QGridLayout };
    PrintJob*          _printJob              { };

signals:

    void sliceStarting( );
    void sliceComplete( bool success );
    void renderStarting( );
    void renderComplete( bool success );

public slots:

    void setPrintJob( PrintJob* printJob );

protected slots:

private slots:

    void layerThicknessComboBox_currentIndexChanged( int index );
    void sliceButton_clicked( bool );
    void calibrateButton_clicked( bool );

    void slicerProcessErrorOccurred( QProcess::ProcessError error );
    void slicerProcessStarted( );
    void slicerProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );

    void svgRenderer_progress( int const currentLayer );
    void svgRenderer_done( int const totalLayers );

};

#endif // __PREPARETAB_H__
