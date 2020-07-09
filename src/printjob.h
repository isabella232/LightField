#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__


#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
#include "ordermanifestmanager.h"

class PrintJob
{
public:
    PrintJob(QSharedPointer<PrintProfile> profile)
    {

        _baseLayerCount = 2;
        _selectedBaseThickness = -1;
        _selectedBodyThickness = -1;
        _directoryMode = false;
        _printProfile = profile;
    }

    PrintParameters& baseLayerParameters()
    {
        return _printProfile->baseLayerParameters();
    }

    PrintParameters& bodyLayerParameters()
    {
        return _printProfile->bodyLayerParameters();
    }

    int buildPlatformOffset() const
    {

        return _printProfile->buildPlatformOffset();
    }

    bool disregardFirstLayerHeight() const
    {

        return _printProfile->disregardFirstLayerHeight();
    }

    int heatingTemperature() const
    {

        return _printProfile->heatingTemperature();
    }

    int getBaseLayerThickness() const
    {

       Q_ASSERT(_baseManager);
       return hasBaseLayers() ? getLayerThicknessAt(0) : 0;
    }

    int getBodyLayerThickness() const
    {

        Q_ASSERT(_bodyManager);
        return getLayerThicknessAt(_baseLayerCount);
    }

    int getBaseLayerCount() const
    {

        return _baseLayerCount;
    }

    void setBaseLayerCount(int value)
    {

        _baseLayerCount = value;
    }

    int getBodyLayerCount() const
    {

        Q_ASSERT(_bodyManager);
        return _bodyManager->getSize() - _baseLayerCount;
    }

    void setPrintProfile(QSharedPointer<PrintProfile> printProfile)
    {

        this->_printProfile = printProfile;
    }

    bool isTiled() const
    {

        if (_bodyManager)
            return _bodyManager->tiled();

        return false;
    }

    bool hasExposureControlsEnabled() const
    {
        return !isTiled();
    }

    int getBuildPlatformOffset() const
    {
        int result = buildPlatformOffset();

        if (!disregardFirstLayerHeight())
            result += getLayerThicknessAt(0);

        return result;
    }

    bool hasBaseLayers() const
    {

        return _baseLayerCount > 0;
    }

    int totalLayerCount() const
    {

        Q_ASSERT(_bodyManager);

        if (hasBaseLayers()) {
            Q_ASSERT(_baseManager);
            return _baseLayerCount + bodyLayerEnd() - bodyLayerStart();
        }

        return _bodyManager->getSize();
    }

    int baseThickness() const
    {

        return _baseLayerCount * getBaseLayerThickness();
    }

    int getLayerThicknessAt(int layerNo) const
    {
        Q_ASSERT(_bodyManager);

        if(_bodyManager->tiled()) {
            return _bodyManager->layerThickNessAt(layerNo);
        } else if (_directoryMode) {
            return _selectedBodyThickness;
        } else {
            if (hasBaseLayers()) {
                Q_ASSERT(_baseManager);
                return layerNo < _baseLayerCount
                    ? _selectedBaseThickness
                    : _selectedBodyThickness;
            }
            return _selectedBodyThickness;
        }
    }

    int baseLayerStart() const
    {

        return hasBaseLayers() ? 0 : -1;
    }

    int baseLayerEnd() const
    {

        return hasBaseLayers() ? _baseLayerCount - 1 : -1;
    }

    int bodyLayerStart() const
    {   

        Q_ASSERT(_bodyManager);
        if(getBodyLayerThickness() > 0)
            return hasBaseLayers() ? baseThickness() / getBodyLayerThickness() : 0;
        else
            return 0;
    }

    int bodyLayerEnd() const
    {

        Q_ASSERT(_bodyManager);
        return _bodyManager->getSize() - 1;
    }

    bool isBaseLayer(int layer) const
    {

        if (!hasBaseLayers())
            return false;

        return layer < _baseLayerCount;
    }

    QString getLayerDirectory(int layer) const
    {

        return isBaseLayer(layer) ? _baseManager->path() : _bodyManager->path();
    }

    QString getLayerFileName(int layer) const
    {

        return isBaseLayer(layer)
            ? _baseManager->getElementAt(layer)
            : _bodyManager->getElementAt(bodyLayerStart() + layer - _baseLayerCount);
    }

    QString getLayerPath(int layer) const
    {

        return QString("%1/%2").arg(getLayerDirectory(layer)).arg(getLayerFileName(layer));
    }

    double getTimeForElementAt(int layer) const
    {

        Q_ASSERT(_bodyManager);
        Q_ASSERT(isTiled());
        return _bodyManager->getTimeForElementAt(layer);
    }

    QSharedPointer<OrderManifestManager> getBaseManager()
    {

        return _baseManager;
    }

    QSharedPointer<OrderManifestManager> getBodyManager()
    {

        return _bodyManager;
    }

    void setBodyManager(QSharedPointer<OrderManifestManager> manager)
    {

        _bodyManager.swap(manager);

        if(_directoryMode || _bodyManager->tiled()) {
            _baseManager = _bodyManager;
        }
    }

    void setBaseManager(QSharedPointer<OrderManifestManager> manager)
    {

        _baseManager.swap(manager);
    }

    int tilingCount() const
    {

        Q_ASSERT(_bodyManager);
        return isTiled() ? _bodyManager->tilingCount() : 1;
    }

    bool getDirectoryMode() const
    {

        return _directoryMode;
    }

    void setDirectoryMode(bool value)
    {

        _directoryMode = value;
    }

    QString getDirectoryPath() const
    {

        return _directoryPath;
    }

    void setDirectoryPath(QString value)
    {

        _directoryPath = value;
    }

    QString getModelFilename() const
    {

        return _modelFilename;
    }

    void setModelFilename(QString value)
    {

        _modelFilename = value;
    }

    QString getModelHash() const
    {

        return _modelHash;
    }

    void setModelHash(QString value)
    {

        _modelHash = value;
    }

    int getSelectedBaseLayerThickness() const
    {

        return _selectedBaseThickness;
    }

    void setSelectedBaseLayerThickness(int value)
    {

        _selectedBaseThickness = value;
    }

    int getSelectedBodyLayerThickness() const
    {

        return _selectedBodyThickness;
    }

    void setSelectedBodyLayerThickness(int value)
    {

        _selectedBodyThickness = value;
    }

    double getEstimatedVolume() const
    {

        Q_ASSERT(_bodyManager);
        return _bodyManager->manifestVolume();
    }

    int getAdvancedExposureControlsEnabled() const {
        return _printProfile->advancedExposureControlsEnabled();
    }

    // unit: boolean (true/false)
    void setAdvancedExposureControlsEnabled(bool value) {
        _printProfile->setAdvancedExposureControlsEnabled(value);
    }

protected:
    int _baseLayerCount;
    int _selectedBaseThickness;
    int _selectedBodyThickness;
    QString _modelFilename;
    QString _directoryPath;
    QString _modelHash;
    bool _directoryMode;
    QSharedPointer<OrderManifestManager> _bodyManager {};
    QSharedPointer<OrderManifestManager> _baseManager {};
    QSharedPointer<PrintProfile>         _printProfile {};
};

#endif // __PRINTJOB_H__
