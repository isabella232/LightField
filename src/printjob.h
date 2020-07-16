#ifndef __PRINTJOB_H__
#define __PRINTJOB_H__


#include "constants.h"
#include "coordinate.h"
#include "printprofile.h"
#include "ordermanifestmanager.h"

class PrintJob
{
public:
    PrintJob(QSharedPointer<PrintProfile>& profile): _printProfile(profile)
    {

        _baseLayerCount = 2;
        _selectedBaseThickness = -1;
        _selectedBodyThickness = -1;
        _directoryMode = false;
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
       return getLayerThicknessAt(getBaseLayerCount());
    }

    int getBaseLayerCount() const
    {
       if(isTiled()) {
           return _baseManager->baseLayerCount();
       }

       return _baseLayerCount;
    }

    void setDisregardFirstLayerHeight(bool enabled) {
        this->_printProfile->setDisregardFirstLayerHeight(enabled);
    }

    void setBuildPlatformOffset(int offset) {
        this->_printProfile->setBuildPlatformOffset(offset);
    }

    void setBaseLayerCount(int value)
    {

        _baseLayerCount = value;
    }

    int getBodyLayerCount() const
    {

        Q_ASSERT(_bodyManager);

        if(isTiled()) {
            return _bodyManager->getSize() - _baseManager->baseLayerCount();
        }

        //assumption that directories has always 2 base layers
        if(_directoryMode) {
            return _bodyManager->getSize() - 2;
        }

        return bodyLayerEnd() - bodyLayerStart() + 1;
    }

    void setPrintProfile(QSharedPointer<PrintProfile>& printProfile)
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
        if(isTiled()) {
            return _bodyManager->baseLayerCount() > 0;
        }

        //assumption that directories has always 2 base layers
        if(_directoryMode)
            return true;

        return _baseLayerCount > 0;
    }

    /**
     * @brief totalLayerCount
     * @return overall layers count of current print (base + body)
     */
    int totalLayerCount() const
    {

        Q_ASSERT(_bodyManager);

        if(isTiled() || _directoryMode) {
            return _bodyManager->getSize();
        }

        if (hasBaseLayers()) {
            Q_ASSERT(_baseManager);
            return _baseLayerCount + (bodyLayerEnd()+1) - bodyLayerStart();
        }

        return _bodyManager->getSize();
    }

    /**
     * @brief baseThickness
     * @return overall base thickness - sum of all base layers
     */
    int baseThickness() const
    {

        return getBaseLayerCount() * getBaseLayerThickness();
    }

    /**
     * @brief getLayerThicknessAt
     * @param layerNo
     * @return returns thickness of requested layer
     */
    int getLayerThicknessAt(int layerNo) const
    {
        Q_ASSERT(_bodyManager);

        if(_bodyManager->tiled()) {
            return _bodyManager->layerThickNessAt(layerNo);
        } else if (_directoryMode) {
            return _selectedBodyThickness;
        } else {
            if (isBaseLayer(layerNo)) {
                return _selectedBaseThickness;
            }

            return _selectedBodyThickness;
        }
    }

    /**
     * @brief isBaseLayer
     * @param layer layer number
     * @return requested layer belongs to: base (true) or body (false)
     */
    bool isBaseLayer(int layer) const
    {

        if (!hasBaseLayers())
            return false;

        return layer < getBaseLayerCount();
    }

    /**
     * @brief getLayerDirectory
     * @param layer layer number
     * @return name of directory where file of requested layer is
     */
    QString getLayerDirectory(int layer) const
    {

        return isBaseLayer(layer) ? _baseManager->path() : _bodyManager->path();
    }

    /**
     * @brief getLayerFileName
     * @param layer layer number
     * @return layer file name
     */
    QString getLayerFileName(int layer) const
    {
        Q_ASSERT(_bodyManager);

        if(isTiled() || _directoryMode) {
            return _bodyManager->getElementAt(layer);
        }

        return isBaseLayer(layer)
            ? _baseManager->getElementAt(layer)
            : _bodyManager->getElementAt(bodyLayerStart() + layer - _baseLayerCount);
    }

    /**
     * @brief getLayerPath
     * @param layer layer number
     * @return absolute path to requested layer
     */
    QString getLayerPath(int layer) const
    {

        return QString("%1/%2").arg(getLayerDirectory(layer)).arg(getLayerFileName(layer));
    }

    /**
     * @brief getTimeForElementAt
     * @param layer layer number in context of current print
     * @return exposure time for requested layer
     */
    double getTimeForElementAt(int layer) const
    {

        Q_ASSERT(_bodyManager);

        if(isTiled()) {
            return _bodyManager->getTimeForElementAt(layer);
        }

        if(isBaseLayer(layer)) {
            return this->_printProfile->baseLayerParameters().layerExposureTime();
        } else {
            return this->_printProfile->bodyLayerParameters().layerExposureTime();
        }

    }

    QSharedPointer<OrderManifestManager>& getBaseManager()
    {

        return _baseManager;
    }

    QSharedPointer<OrderManifestManager>& getBodyManager()
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

    /* number of tiles per layer */
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

    QSharedPointer<PrintProfile>& printProfile() {
        return _printProfile;
    }

    void printJobData() {
        const auto& baseLayerParameters = this->baseLayerParameters();
        const auto& bodyLayerParameters = this->bodyLayerParameters();

        debug(
            "+ Window::startPrinting: print job %p:\n"
            "  + modelFileName:              '%s'\n"
            "  + modelHash:                  %s\n"
            "  + totalLayerCount:            %d\n"
            "  + base slices:\n"
            "    + sliceDirectory:           '%s'\n"
            "    + layerCount:               %d\n"
            "    + layerThickness:           %d\n"
            "    + startLayer:               %d\n"
            "    + endLayer:                 %d\n"
            "  + body slices:\n"
            "    + sliceDirectory:           '%s'\n"
            "    + layerCount:               %d\n"
            "    + layerThickness:           %d\n"
            "    + startLayer:               %d\n"
            "    + endLayer:                 %d\n"
            "  + layer parameters: (calculated parameters are marked with *)\n"

            "",

            this,
            getModelFilename().toUtf8().data(),
            getModelHash().toUtf8().data(),
            totalLayerCount(),

            getBaseManager()->path().toUtf8().data(),
            getBaseLayerCount(),
            getBaseLayerThickness(),
            baseLayerStart(),
            baseLayerEnd(),

            getBodyManager()->path().toUtf8().data(),
            getBodyLayerCount(),
            getBodyLayerThickness(),
            bodyLayerStart(),
            bodyLayerEnd()
        );

        debug(
            "    + baseLayerCount:           %d\n"
            "    + baseLayersPumpingEnabled: %s\n"
            "    + base layer parameters:\n"
            "      + pumpUpDistance:         %.2f mm\n"
            "      + pumpUpVelocity:        *%.2f mm/min\n"
            "      + pumpUpPause:            %d ms\n"
            "      + pumpDownDistance:      *%.2f mm\n"
            "      + pumpDownVelocity:      *%.2f mm/min\n"
            "      + pumpDownPause:          %d ms\n"
            "      + noPumpUpVelocity:       %.2f mm/min\n"
            "      + pumpEveryNthLayer:      %d\n"
            "      + layerExposureTime:      %.2f s\n"
            "      + powerLevel:             %.1f%%\n"
            "",

            getBaseLayerCount(),
            ToString( baseLayerParameters.isPumpingEnabled()),
            baseLayerParameters.pumpUpDistance(),
            baseLayerParameters.pumpUpVelocity_Effective(),
            baseLayerParameters.pumpUpPause(),
            baseLayerParameters.pumpDownDistance_Effective(),
            baseLayerParameters.pumpDownVelocity_Effective(),
            baseLayerParameters.pumpDownPause(),
            baseLayerParameters.noPumpUpVelocity(),
            baseLayerParameters.pumpEveryNthLayer(),
            baseLayerParameters.layerExposureTime(),
            baseLayerParameters.powerLevel()
        );


        debug(
            "    + bodyLayersPumpingEnabled: %s\n"
            "    + body layer parameters:\n"
            "      + pumpUpDistance:         %.2f mm\n"
            "      + pumpUpVelocity:        *%.2f mm/min\n"
            "      + pumpUpPause:            %d ms\n"
            "      + pumpDownDistance:      *%.2f mm\n"
            "      + pumpDownVelocity:      *%.2f mm/min\n"
            "      + pumpDownPause:          %d ms\n"
            "      + noPumpUpVelocity:       %.2f mm/min\n"
            "      + pumpEveryNthLayer:      %d\n"
            "      + layerExposureTime:      %.2f s\n"
            "      + powerLevel:             %.1f%%\n"
            "",


            ToString( bodyLayerParameters.isPumpingEnabled()),
            bodyLayerParameters.pumpUpDistance(),
            bodyLayerParameters.pumpUpVelocity_Effective(),
            bodyLayerParameters.pumpUpPause(),
            bodyLayerParameters.pumpDownDistance_Effective(),
            bodyLayerParameters.pumpDownVelocity_Effective(),
            bodyLayerParameters.pumpDownPause(),
            bodyLayerParameters.noPumpUpVelocity(),
            bodyLayerParameters.pumpEveryNthLayer(),
            bodyLayerParameters.layerExposureTime(),
            bodyLayerParameters.powerLevel()
        );
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
    QSharedPointer<PrintProfile>&         _printProfile;

    int baseLayerStart() const
    {

        return hasBaseLayers() ? 0 : -1;
    }

    int baseLayerEnd() const
    {

        return hasBaseLayers() ? getBaseLayerCount() - 1 : -1;
    }

    int bodyLayerStart() const
    {

        Q_ASSERT(_bodyManager);

        if(_selectedBodyThickness > 0)
            return (_selectedBaseThickness * getBaseLayerCount()) / _selectedBodyThickness;
        else
            return 0;
    }

    int bodyLayerEnd() const
    {

        Q_ASSERT(_bodyManager);
        return _bodyManager->getSize() - 1;                                                                      ;
    }
};

#endif // __PRINTJOB_H__
