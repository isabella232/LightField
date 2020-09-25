#ifndef __TILINGTAB_H__
#define __TILINGTAB_H__

#include <QtCore>
#include <QtWidgets>
#include "tabbase.h"
#include "ordermanifestmanager.h"
#include "paramslider.h"
#include "printjob.h"
#include "spoiler.h"

class TilingExpoTimePopup: public QDialog {
    Q_OBJECT

public:
    TilingExpoTimePopup(QWidget* parent);

    inline void setMinExposureBase(int value) {
        _advBaseExpoCorse->setValue(value - (value % 1000));
        _advBaseExpoFine->setValue(value % 1000);
    }


    inline void setStepBase(double value) {
        _stepBase->setValueDouble(value);
    }

    inline void setMinExposureBody(int value) {
        _advBodyExpoCorse->setValue(value - (value % 1000));
        _advBodyExpoFine->setValue(value % 1000);
    }

    inline void setStepBody(double value) {
        _stepBody->setValueDouble(value);
    }

    inline int minExposureBase() {
        return _advBaseExpoCorse->getValue() + _advBaseExpoFine->getValue();
    }

    inline double stepBase() {
        return _stepBase->getValueDouble();
    }

    inline int minExposureBody() {
        return _advBodyExpoCorse->getValue() + _advBodyExpoFine->getValue();
    }

    inline double stepBody() {
        return _stepBody->getValueDouble();
    }

private:
    ParamSlider*            _stepBase                     { new ParamSlider ("Base layer exposure step", "sec", 0, 16, 1, 0, 4 )};
    ParamSlider*            _stepBody                     { new ParamSlider ("Body layer exposure step", "sec", 0, 16, 1, 0, 4 )};
    QPushButton*            _okButton                     { new QPushButton ("Ok" )                };
    QPushButton*            _cancelButton                 { new QPushButton ("Cancel" )            };

    ParamSlider*       _advBodyExpoCorse                   { new ParamSlider( "Body exposure time coarse", "s",
                                                                              1000, 30000, 1000, 1000, 1000 ) };
    ParamSlider*       _advBodyExpoFine                    { new ParamSlider( "Body exposure time fine", "ms",
                                                                              50, 950, 50, 0) };

    ParamSlider*       _advBaseExpoCorse                   { new ParamSlider( "Base exposure time coarse", "s",
                                                                              1000, 150000, 1000, 1000, 1000 ) };
    ParamSlider*       _advBaseExpoFine                    { new ParamSlider( "Base exposure time fine", "ms",
                                                                             50, 950, 50, 0) };

    QLabel*             _advBodyLbl                        { new QLabel };
    QLabel*             _advBaseLbl                        { new QLabel };

    void confirm( bool );
    void cancel( bool );
private slots:
    void updateExposureTimes();
};

class TilingTab: public TabBase {
    Q_OBJECT

public:
    TilingTab(QWidget* parent = nullptr);
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
    TilingExpoTimePopup     _expoTimePopup            { this };
    QPushButton*            _setupExpoTimeBt          { new QPushButton( "Edit Exposure..." ) };
    QPushButton*            _setupTiling              { new QPushButton      };

    void _showLayerImage ( );
    void _showWarningAndClose ( );
    int  _getMaxCount();
    void _renderText(QPainter* painter, QPoint pos, double expoBase, double expoBody);
    void _setEnabled(bool enabled);
    void _updateExposureTiming();
signals:
    ;

public slots:
    void setupTilingClicked ( bool );

    void setStepValue();

    void setupExpoTimeClicked(bool);

    void activeProfileChanged(QSharedPointer<PrintProfile> newProfile);

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;
    virtual void printJobChanged() override;
    ;
protected slots:
    ;

private slots:
    void confirmButton_clicked( bool );

    void printManager_printStarting();
    void printManager_printComplete(bool const success);
    void printManager_printAborted();
};

#endif // __TILINGTAB_H__
