#include "ordermanifestmanager.h"

const QString ManifestKeys::strings[3] = {
        "size",
        "sort_type",
        "entities"
};

const QString ManifestSortType::strings[3] = {
        "numeric",
        "alphanumeric",
        "custom"
};

ManifestParseResult OrderManifestManager::parse(QStringList *errors=nullptr, QStringList *warningList = nullptr) {
    debug( "+ OrderManifestManager::parse: parsing manifest file in directory '%s'\n", _dirPath.toUtf8( ).data( ) );
    _fileNameList.clear();


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

    debug( "OrderManifestManager::parse: building document\n" );
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

    QJsonArray entities = root.value(ManifestKeys(ManifestKeys::ENTITIES).toQString()).toArray();

    int i=0;
    for(; i<entities.count(); ++i) {
        _fileNameList.push_back(entities[i].toString());
    }

    if(i != _size ) {
        if(warningList)
            warningList->push_back( "Declared size is divergent with the number of entries.");

        _size = i;
        return ManifestParseResult::POSITIVE_WITH_WARNINGS;
    }

    return ManifestParseResult::POSITIVE;
}

bool OrderManifestManager::save() {
    debug( "+ OrderManifestManager::save \n" );
    QFile jsonFile( _dirPath % Slash % ManifestFilename );
    QJsonDocument jsonDocument=QJsonDocument( );
    QJsonObject   root;
    root.insert( ManifestKeys(ManifestKeys::SORT_TYPE).toQString(), QJsonValue { _type.toQString() } );
    root.insert( ManifestKeys(ManifestKeys::SIZE).toQString(),      QJsonValue { _size } );

    QJsonArray jsonArray;

    for(int i=0; i<_fileNameList.size(); ++i)
    {
        jsonArray.append(_fileNameList[i]);
    }

    root.insert( ManifestKeys(ManifestKeys::ENTITIES).toQString(), jsonArray );
    jsonDocument.setObject(root);

    jsonFile.open(QFile::WriteOnly);
    jsonFile.write(jsonDocument.toJson());
}
