#include <exception>
#include <QtCore>
#include <QtWidgets>
#include "constants.h"
#include "debug.h"
#include "ordermanifestmanager.h"

using namespace std;

const QString ManifestKeys::strings[13] = {
        "size",
        "sort_type",
        "tiling",
        "minExposure",
        "step",
        "space",
        "count",
        "entities",
        "fileName",
        "layerThickness",
        "exposureTime",
        "estimatedVolume",
        "baseLayerCnt"
};

const QString ManifestSortType::strings[3] = {
        "numeric",
        "alphanumeric",
        "custom"
};

ManifestParseResult OrderManifestManager::parse(QStringList *errors=nullptr, QStringList *warningList = nullptr) {
    debug( "+ OrderManifestManager::parse: parsing manifest file in directory '%s'\n", _dirPath.toUtf8( ).data( ) );
    _fileNameList.clear();
    _layerThickNess.clear();
    _tilingExpoTime.clear();

    QFile jsonFile( _dirPath % Slash % ManifestFilename );
    if(!jsonFile.exists()) {

        debug( "+ OrderManifestManager::parse: manifest file not exist '%s'\n", _dirPath.toUtf8( ).data( ) );

        return ManifestParseResult::FILE_NOT_EXIST;
    }

    if(!jsonFile.open(QIODevice::ReadOnly)) {

        debug( "+ OrderManifestManager::parse: no access rights to manifest file '%s'\n", _dirPath.toUtf8( ).data( ) );

        if(errors)
            errors->push_back("No access rights to manifest file.");

        return ManifestParseResult::FILE_NOT_EXIST;
    }

    debug( "+ OrderManifestManager::parse: building document\n" );
    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument().fromJson(jsonFile.readAll(), &parseError);

    if(jsonDocument.isNull()) {

        debug( "+ OrderManifestManager::parse: error while parsing manifest file '%s'\n", parseError.errorString().toUtf8( ).data( ) );

        if(errors)
            errors->push_back(parseError.errorString());

        return ManifestParseResult::FILE_CORRUPTED;
    }

    QJsonObject           root = jsonDocument.object();

    _size = root.value(ManifestKeys(ManifestKeys::SIZE).toQString()).toInt();
    _type = ManifestSortType(root.value(ManifestKeys(ManifestKeys::SORT_TYPE).toQString()).toString());

    debug( "+ OrderManifestManager::parse: checking tiling... \n" );
    try {
        if( root.contains( ManifestKeys(ManifestKeys::TILING).toQString( ) ) ) {
            QJsonObject tilingNested;
            debug( "+ OrderManifestManager::parse: checking tiled \n" );
            _tiled = true;
            tilingNested = root.value(ManifestKeys(ManifestKeys::TILING).toQString()).toObject();
            _tilingStep = tilingNested.value(ManifestKeys(ManifestKeys::STEP).toQString()).toDouble();
            _tilingMinExposure = tilingNested.value(ManifestKeys(ManifestKeys::MIN_EXPOSURE).toQString()).toDouble();
            _tilingSpace = tilingNested.value(ManifestKeys(ManifestKeys::SPACE).toQString()).toInt();
            _tilingCount = tilingNested.value(ManifestKeys(ManifestKeys::COUNT).toQString()).toInt();
            _baseLayerCount = tilingNested.value(ManifestKeys(ManifestKeys::BASE_LAYER_CNT).toQString()).toInt();

            QJsonArray expoTimes = tilingNested.value(ManifestKeys(ManifestKeys::EXPOSURE_TIME).toQString()).toArray();

            for(int i=0; i<expoTimes.count(); ++i) {
                _tilingExpoTime.push_back(expoTimes[i].toDouble());
            }

            QJsonArray layerThicknessArray = tilingNested.value(ManifestKeys(ManifestKeys::LAYER_THICKNESS).toQString()).toArray();

            for(int i=0; i<layerThicknessArray.count(); ++i)
                _layerThickNess.push_back(layerThicknessArray[i].toInt());
    }
    } catch (...) {
        return ManifestParseResult::FILE_CORRUPTED;
    }

    if(root.contains( ManifestKeys(ManifestKeys::VOLUME).toQString( ) ) ) {
      _estimatedVolume = root.value(ManifestKeys(ManifestKeys::VOLUME).toQString()).toDouble();
    }

    QJsonArray entities = root.value(ManifestKeys(ManifestKeys::ENTITIES).toQString()).toArray();

    int i=0;

    try {
        for(; i<entities.count(); ++i) {
            QJsonObject entity = entities[i].toObject();
            QString fileName = entity.value(ManifestKeys(ManifestKeys::FILE_NAME).toQString()).toString();

            _fileNameList.push_back(fileName);
        }
    } catch (...) {
        return ManifestParseResult::FILE_CORRUPTED;
    }

    if(i != _size) {
        if(warningList)
            warningList->push_back( "Declared size is divergent with the number of entries.");

        _size = i;
        return ManifestParseResult::POSITIVE_WITH_WARNINGS;
    }

    _initialized = true;
    return ManifestParseResult::POSITIVE;
}

bool OrderManifestManager::save() {
    debug("+ OrderManifestManager::save \n");
    QFile jsonFile(_dirPath % Slash % ManifestFilename);
    QJsonDocument jsonDocument=QJsonDocument();
    QJsonObject root;
    root.insert(ManifestKeys(ManifestKeys::SORT_TYPE).toQString(), QJsonValue {_type.toQString()});
    root.insert(ManifestKeys(ManifestKeys::SIZE).toQString(), QJsonValue {_size});

    if (_tiled) {
        QJsonObject tiling;

        tiling.insert(ManifestKeys(ManifestKeys::MIN_EXPOSURE).toQString(), QJsonValue {_tilingMinExposure});
        tiling.insert(ManifestKeys(ManifestKeys::STEP).toQString(), QJsonValue {_tilingStep});
        tiling.insert(ManifestKeys(ManifestKeys::SPACE).toQString(), QJsonValue {_tilingSpace});
        tiling.insert(ManifestKeys(ManifestKeys::COUNT).toQString(), QJsonValue {_tilingCount});
        tiling.insert(ManifestKeys(ManifestKeys::BASE_LAYER_CNT).toQString(), QJsonValue {_baseLayerCount});

        QJsonArray expoArray;
        for(int i=0; i<_tilingExpoTime.size(); ++i)
        {
            expoArray.append(_tilingExpoTime[i]);
        }

        tiling.insert( ManifestKeys(ManifestKeys::EXPOSURE_TIME).toQString(), expoArray );

        QJsonArray layerThickNessArray;
        for (int i=0; i<_tilingExpoTime.size(); ++i)
            layerThickNessArray.append(layerThickNessAt(i));

        tiling.insert( ManifestKeys(ManifestKeys::LAYER_THICKNESS).toQString(),  layerThickNessArray);

        root.insert( ManifestKeys(ManifestKeys::TILING).toQString(), tiling );

    }

    QJsonArray jsonArray;
    QString fileName;
    QImage calculationImage;
    int activeTreshold = QColor("white").value() / 2;
    int i = 0;
    int total = _fileNameList.count();

    foreach (fileName, _fileNameList) {
        QJsonObject entity;
        entity.insert(ManifestKeys(ManifestKeys::FILE_NAME).toQString(), QJsonValue { fileName });

        if(_tiled || _calculateArea) {
            unsigned int activePixels = 0;

            if (_calculateArea) {
                calculationImage.load(_dirPath % Slash % fileName);
                for (int size_y=0; size_y < calculationImage.height(); size_y++) {
                    for (int size_x=0; size_x < calculationImage.width(); size_x++) {
                        QColor tempColor = calculationImage.pixel(size_x, size_y);
                        if (tempColor.value() >= activeTreshold)
                            activePixels++;
                    }
                }

                _estimatedVolume += ProjectorPixelSize * ProjectorPixelSize *
                    activePixels * layerThickNessAt(i++) / 1000; // units: mm * mm * µm / 1000 = µL
            }
        }

        jsonArray.append(entity);

        emit statusUpdate(QString("Calculating volume of layer %1").arg(fileName));
        emit progressUpdate( ((double)i / (double)total) * 100 );
    }

    root.insert( ManifestKeys(ManifestKeys::VOLUME).toQString(), QJsonValue { _estimatedVolume } );
    root.insert( ManifestKeys(ManifestKeys::ENTITIES).toQString(), jsonArray );
    jsonDocument.setObject(root);

    bool result;
    result = jsonFile.open(QFile::WriteOnly);
    result = result && jsonFile.write(jsonDocument.toJson()) > 0;

    return result;
}

double OrderManifestManager::getTimeForElementAt(int position){
    if(!_tiled || _tilingExpoTime.count() < position)
        return 0;
    else
        return _tilingExpoTime[position];
}

/**
 * If position out of range returns -1.
 *
 * @brief OrderManifestManager::layerThickNessAt
 * @param position
 * @return
 */
int OrderManifestManager::layerThickNessAt(int position) {
    debug("+ OrderManifestManager::layerThickNessAt \n");
    if(_layerThickNess.count() > 0 && position < _layerThickNess.count())
    {
        return _layerThickNess[position];
    } else if (_layerThickNess.count() > 0 && _layerThickNess.count() <= position) {
        return -1;
    } else {
        if( position < _baseLayerCount ) {
            return _baseLayerThickNess;
        } else {
            return _bodyLayerThickNess;
        }
    }
}
