#ifndef __PREPARETAB_H__
#define __PREPARETAB_H__

#include "tabbase.h"

class Hasher;
class SvgRenderer;

class PrepareTab: public InitialShowEventMixin<PrepareTab, TabBase> {

    Q_OBJECT

public:

    PrepareTab( QWidget* parent = nullptr );
    virtual ~PrepareTab( ) override;

    bool isPrepareButtonEnabled( ) const { return _prepareButton->isEnabled( ); }
    bool isSliceButtonEnabled( )   const { return _sliceButton->isEnabled( );   }

protected:

private:

    QProcess*     _slicerProcess               { };
    SvgRenderer*  _svgRenderer                 { };
    Hasher*       _hasher                      { };
    int           _visibleLayer                { };
    int           _renderedLayers              { };

    QLabel*       _layerThicknessLabel         { new QLabel       };
    QRadioButton* _layerThickness50Button      { new QRadioButton };
    QRadioButton* _layerThickness100Button     { new QRadioButton };
    QVBoxLayout*  _layerThicknessButtonsLayout { new QVBoxLayout  };

    QLabel*       _sliceStatusLabel            { new QLabel       };
    QLabel*       _sliceStatus                 { new QLabel       };
    QLabel*       _imageGeneratorStatusLabel   { new QLabel       };
    QLabel*       _imageGeneratorStatus        { new QLabel       };

    QGroupBox*    _prepareGroup                { new QGroupBox    };
    QLabel*       _prepareMessage              { new QLabel       };
    QProgressBar* _prepareProgress             { new QProgressBar };
    QPushButton*  _prepareButton               { new QPushButton  };
    QVBoxLayout*  _prepareLayout               { new QVBoxLayout  };

    QVBoxLayout*  _optionsLayout               { new QVBoxLayout  };
    QWidget*      _optionsContainer            { new QWidget      };
    QPushButton*  _sliceButton                 { new QPushButton  };

    QGroupBox*    _currentSliceGroup           { new QGroupBox    };
    QLabel*       _currentSliceImage           { new QLabel       };
    QVBoxLayout*  _currentSliceLayout          { new QVBoxLayout  };

    QPushButton*  _navigateFirst               { new QPushButton  };
    QPushButton*  _navigatePrevious            { new QPushButton  };
    QLabel*       _navigateCurrentLabel        { new QLabel       };
    QPushButton*  _navigateNext                { new QPushButton  };
    QPushButton*  _navigateLast                { new QPushButton  };
    QHBoxLayout*  _navigationLayout            {                  };

    QGridLayout*  _layout                      { new QGridLayout  };

    virtual void _initialShowEvent( QShowEvent* showEvent ) override;

    bool _checkPreSlicedFiles( );
    bool _checkJobDirectory( );
    void _setNavigationButtonsEnabled( bool const enabled );
    void _showLayerImage( int const layer );

signals:

    void slicingNeeded( bool const needed );
    void sliceStarted( );
    void sliceComplete( bool const success );
    void renderStarted( );
    void renderComplete( bool const success );

    void preparePrinterStarted( );
    void preparePrinterComplete( bool const success );

public slots:

    void setPrepareButtonEnabled( bool const enabled );
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

    void prepareButton_clicked( bool );
    void shepherd_homeComplete( bool const success );
    void adjustBuildPlatform_complete( bool );
    void shepherd_resinLoadMoveToComplete( bool const success );

};

#endif // __PREPARETAB_H__
