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
    ParamSlider*            _minExposureBase              { new ParamSlider ("Base layer minimum exposure", "sec", 1, 40, 2, 1, 4 ) };
    ParamSlider*            _stepBase                     { new ParamSlider ("Base layer exposure step", "sec", 1, 16, 2, 1, 4 )};
    ParamSlider*            _minExposureBody              { new ParamSlider ("Body layer minimum exposure", "sec", 1, 40, 2, 1, 4 ) };
    ParamSlider*            _stepBody                     { new ParamSlider ("Body layer exposure step", "sec", 1, 16, 2, 1, 4 )};
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

private:
    QLabel*                 _currentLayerImage        { new QLabel  };
    ParamSlider*            _space                    { new ParamSlider ("Space", "mm", 1, 10, 1, 1)};
    ParamSlider*            _count                    { new ParamSlider ("Count", "", 1, 8, 1, 1)};
    QPushButton*            _confirm                  { new QPushButton ("Confirm") };
    QLabel*                 _minExposureBaseLabel     { new QLabel("Minimum exposure time") };
    QLabel*                 _stepBaseLabel            { new QLabel("Step") };
    QLabel*                 _minExposureBodyLabel     { new QLabel("Minimum exposure time") };
    QLabel*                 _stepBodyLabel            { new QLabel("Step") };

    QLabel*                 _minExposureBaseValue     { new QLabel("10s") };
    QLabel*                 _stepBaseValue            { new QLabel("2s") };
    QLabel*                 _minExposureBodyValue     { new QLabel("10s") };
    QLabel*                 _stepBodyValue            { new QLabel("2s") };

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
    QPushButton*            _setupExpoTimeBt          { new QPushButton( "Exposure time" ) };
    QPushButton*            _setupTiling              { new QPushButton      };

    void _showLayerImage ( );
    void _showWarningAndClose ( );
    int  _getMaxCount();
    void _renderText(QPainter* painter, int tileWidth, QPoint pos, double expoBase, double expoBody);
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
    ;
};

#endif // __TILINGTAB_H__
