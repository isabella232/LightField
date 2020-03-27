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
    ParamSlider*            _minExposure              { new ParamSlider ("Minimum exposure", "sec", 2, 10, 2, 2) };
    ParamSlider*            _step                     { new ParamSlider ("Step", "", 2, 8, 2, 2)};
    ParamSlider*            _space                    { new ParamSlider ("Space", "", 1, 4, 1, 1, 4)};
    QPushButton*            _confirm                  { new QPushButton ("Confirm") };
    PrintJob*               _printJob                 { };
    OrderManifestManager*   _manifestManager          { };

    void _showLayerImage ( );
    int _checkIfTilable ( );
    void _showWarningAndClose ( );
signals:
    ;

public slots:
    void setupTilingClicked ( OrderManifestManager* manifestManager, PrintJob* printJob ) {
        debug( "+ TilingTab::setupTilingClicked\n" );

        this->_printJob = printJob;
        this->_manifestManager = manifestManager;


        if(!_checkIfTilable()) {
                _showWarningAndClose();
                return;
        }

        this->_confirm->setEnabled( true );
        this->_step->setEnabled( true );
        this->_space->setEnabled( true );
        this->_minExposure->setEnabled( true );

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
