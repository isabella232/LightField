#ifndef __TILINGTAB_H__
#define __TILINGTAB_H__

#include "tabbase.h"
#include "printjob.h"
#include "ordermanifestmanager.h"
#include "paramslider.h"

class TilingTab: public TabBase {
    Q_OBJECT

public:
    TilingTab( QWidget* parent = nullptr );
    virtual ~TilingTab( ) { };

    virtual TabIndex tabIndex( ) const override { return TabIndex::Tiling; }
protected:

private:
    QLabel*                 _currentLayerImage        { new QLabel  };
    ParamSlider*            _minExposure              { new ParamSlider ("Minimum exposure", "sec", 1, 40, 2, 1, 4 ) };
    ParamSlider*            _step                     { new ParamSlider ("Exposure step", "sec", 1, 16, 2, 1, 4 )};
    ParamSlider*            _space                    { new ParamSlider ("Space", "mm", 1, 10, 1, 1)};
    ParamSlider*            _count                    { new ParamSlider ("Count", "", 1, 8, 1, 1)};
    QPushButton*            _confirm                  { new QPushButton ("Confirm") };
    PrintJob*               _printJob                 { };
    OrderManifestManager*   _manifestManager          { };
    int                     _pixmapWidth              { 0 };
    int                     _pixmapHeight             { 0 };
    int                     _areaWidth                { 0 };
    int                     _areaHeight               { 0 };
    double                  _wRatio                   { 1.0L };
    double                  _hRatio                   { 1.0L };
    QPixmap*                _pixmap                   { nullptr };

    void _showLayerImage ( );
    void _showWarningAndClose ( );
    int  _getMaxCount();
signals:
    ;

public slots:
    void setupTilingClicked ( OrderManifestManager* manifestManager, PrintJob* printJob ) {
        debug( "+ TilingTab::setupTilingClicked\n" );

        this->_printJob = printJob;
        this->_manifestManager = manifestManager;

        this->_areaWidth = _currentLayerImage->width( );
        this->_areaHeight = _currentLayerImage->height( );
        this->_wRatio = ((double)_areaWidth) /  ProjectorWindowSize.width();
        this->_hRatio = ((double)_areaHeight) /  ProjectorWindowSize.height();

        QPixmap pixmap ( _printJob->jobWorkingDirectory % Slash % _manifestManager->getFirstElement() );

        if( this->_pixmap )
            delete this->_pixmap;

        this->_pixmap = new QPixmap ( pixmap.scaled( pixmap.width( ) * _wRatio, pixmap.height( ) * _hRatio) );

        this->_pixmapWidth = this->_pixmap->width( );
        this->_pixmapHeight = this->_pixmap->height( );

        if(_getMaxCount() < 1) {
                _showWarningAndClose();
                return;
        }

        this->_confirm->setEnabled( true );
        this->_step->setEnabled( true );
        this->_space->setEnabled( true );
        this->_minExposure->setEnabled( true );
        this->_count->setEnabled( true );

        _showLayerImage();
    }

    void setStepValue();

    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

protected slots:
    ;

private slots:
    void confirmButton_clicked( bool );
    ;
};

#endif // __TILINGTAB_H__
