#ifndef __TILINGTAB_H__
#define __TILINGTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"
#include "printjob.h"
#include "ordermanifestmanager.h"
#include "paramslider.h"

class TilingExpoTimePopup: public QDialog {
    Q_OBJECT

public:
    TilingExpoTimePopup( );

    inline void setMinExposureBase(double value) { _minExposureBase->setValueDouble(value); }
    inline void setStepBase(double value)        { _stepBase->setValueDouble(value);        }
    inline void setMinExposureBody(double value) { _minExposureBody->setValueDouble(value); }
    inline void setStepBody(double value)        { _stepBody->setValueDouble(value);        }

    inline double minExposureBase() { return _minExposureBase->getValueDouble(); }
    inline double stepBase()        { return _stepBase->getValueDouble();        }
    inline double minExposureBody() { return _minExposureBody->getValueDouble(); }
    inline double stepBody()        { return _stepBody->getValueDouble();        }

private:
    ParamSlider*            _minExposureBase              { new ParamSlider ("Base layer minimum exposure", "sec", 1, 40, 1, 1, 4 ) };
    ParamSlider*            _stepBase                     { new ParamSlider ("Base layer exposure step", "sec", 1, 16, 1, 0, 4 )};
    ParamSlider*            _minExposureBody              { new ParamSlider ("Body layer minimum exposure", "sec", 1, 40, 1, 1, 4 ) };
    ParamSlider*            _stepBody                     { new ParamSlider ("Body layer exposure step", "sec", 1, 16, 1, 0, 4 )};
    QPushButton*            _okButton                     { new QPushButton ("Ok" )                };
    QPushButton*            _cancelButton                 { new QPushButton ("Cancel" )            };

    void confirm( bool );
    void cancel( bool );
};

class TilingTab: public TabBase {
    Q_OBJECT

public:
    TilingTab( QWidget* parent = nullptr );
    virtual ~TilingTab( ) { }

    virtual TabIndex tabIndex( ) const override { return TabIndex::Tiling; }

protected:
    virtual void _connectPrintManager() override;

private:
    QLabel*                 _currentLayerImage        { new QLabel  };
    ParamSlider*            _space                    { new ParamSlider ("Tile Spacing", "mm", 1, 10, 1, 1)};
    ParamSlider*            _count                    { new ParamSlider ("Count", "", 1, 8, 1, 1)};
    QPushButton*            _confirm                  { new QPushButton ("Create Tiles") };
    QLabel*                 _minExposureBaseLabel     { new QLabel("10s Minimum Layer Exposure") };
    QLabel*                 _stepBaseLabel            { new QLabel("2s Exposure Step") };
    QLabel*                 _minExposureBodyLabel     { new QLabel("10s Minimum Layer Exposure") };
    QLabel*                 _stepBodyLabel            { new QLabel("2s Exposure Step") };
    QLabel*                 _fileNameLabel            { new QLabel };

    int                     _pixmapWidth              { 0 };
    int                     _pixmapHeight             { 0 };
    int                     _areaWidth                { 0 };
    int                     _areaHeight               { 0 };
    double                  _wRatio                   { 1.0L };
    double                  _hRatio                   { 1.0L };
    double                  _minExposureBase          { 10 };
    double                  _stepBase                 { 2 };
    double                  _minExposureBody          { 10 };
    double                  _stepBody                 { 2 };
    QPixmap*                _pixmap                   { nullptr };
    TilingExpoTimePopup     _expoTimePopup            { };
    QPushButton*            _setupExpoTimeBt          { new QPushButton( "Edit Exposure..." ) };
    QPushButton*            _setupTiling              { new QPushButton      };

    void _showLayerImage ( );
    void _showWarningAndClose ( );
    int  _getMaxCount();
    void _renderText(QPainter* painter, QPoint pos, double expoBase, double expoBody);
    void _setEnabled(bool enabled);
signals:
    ;

public slots:
    void setupTilingClicked ( bool );

    void setStepValue();

    void setupExpoTimeClicked(bool);
    ;

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

protected slots:
    ;

private slots:
    void confirmButton_clicked( bool );

    void printManager_printStarting();
    void printManager_printComplete(bool const success);
    void printManager_printAborted();
};

#endif // __TILINGTAB_H__
