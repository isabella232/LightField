#ifndef ORDERMANIFESTMANAGER_H
#define ORDERMANIFESTMANAGER_H

enum class ManifestParseResult {
    FILE_NOT_EXIST,
    FILE_CORRUPTED,
    POSITIVE_WITH_WARNINGS,
    POSITIVE
};

class ManifestKeys {
public:
    enum Value : uint8_t
    {
        SIZE,
        SORT_TYPE,
        ENTITIES
    };

    static const QString strings[3];

    ManifestKeys() = default;
    constexpr ManifestKeys(Value key) : value(key) { }

    constexpr const QString& toQString() {
        return strings[value];
    }
  private:
    Value value;
};


class ManifestSortTypes {
public:
    enum Value : uint8_t
    {
        NUMERIC,
        ALPHANUMERIC,
        CUSTOM
    };

    static const QString strings[3];

    ManifestSortTypes() = default;
    constexpr ManifestSortTypes(Value key) : value(key) { }

    ManifestSortTypes(QString key) {
        for(uint8_t i=0; i<strings->length(); ++i) {
            if(strings[i] == key) {
                value = (Value)i;

                return;
            }
        }

        value = CUSTOM;
    }

    constexpr const QString& toQString() {
        return strings[value];
    }
private:
    Value value;
};


class OrderManifestManager {

public:
    class Iterator {
    public:
        Iterator(QStringList list) : _list(list) { _ptr = 0; }

        void     operator++() { _ptr = ( _ptr < _list.size() ? _ptr++ : _ptr); }
        QString  operator*()  { return _list[_ptr]; }
        bool     hasNext()    { return _ptr + 1 >= _list.size(); }
    private:
            int         _ptr;
            QStringList _list;
    };

    OrderManifestManager()      { }
    ~OrderManifestManager()     { }

    void setPath ( QString path ) {
        _dirPath = path;
    }

    QString path() { return _dirPath; }

    ManifestParseResult parse(QStringList *errors=nullptr, QStringList *warningList = nullptr) {
        debug( "+ OrderManifestManager::getSlicesList: parsing manifest file in directory '%s'\n", _dirPath.toUtf8( ).data( ) );
        _fileNameList.clear();


        QFile jsonFile(_dirPath % ManifestFilename);
        if(!jsonFile.exists()) {

            debug( "+ OrderManifestManager::getSlicesList: manifest file not exist '%s'\n", _dirPath.toUtf8( ).data( ) );

            return ManifestParseResult::FILE_NOT_EXIST;
        }

        if(!jsonFile.open(QIODevice::ReadOnly)) {

            debug( "+ OrderManifestManager::getSlicesList: no access rights to manifest file '%s'\n", _dirPath.toUtf8( ).data( ) );

            if(errors)
                errors->push_back("No access rights to manifest file.");

            return ManifestParseResult::FILE_NOT_EXIST;
        }

        debug( "OrderManifestManager::getSlicesList: building document\n" );
        QJsonParseError parseError;
        QJsonDocument jsonDocument = QJsonDocument().fromJson(jsonFile.readAll(), &parseError);

        if(jsonDocument.isNull()) {

            debug( "+ OrderManifestManager::getSlicesList: error while parsing manifest file '%s'\n", parseError.errorString().toUtf8( ).data( ) );

            if(errors)
                errors->push_back(parseError.errorString());

            return ManifestParseResult::FILE_CORRUPTED;
        }

        QJsonObject           root = jsonDocument.object();

        _size = root.value(ManifestKeys(ManifestKeys::SIZE).toQString()).toInt();
        _type = ManifestSortTypes(root.value(ManifestKeys(ManifestKeys::SORT_TYPE).toQString()).toString());

        QJsonArray entities = root.value(ManifestKeys(ManifestKeys::ENTITIES).toQString()).toArray();

        int i=0;
        for(; i<entities.count(); ++i) {
            debug( "+ OrderManifestManager::parse: '%s'\n", entities[i].toString().toUtf8( ).data( ) );

            _fileNameList.push_back(entities[i].toString());
        }

        if(i != _size ) {
            if(warningList)
                warningList->push_back("Declared size is divergent with the number of entries.");

            _size = i;
            return ManifestParseResult::POSITIVE_WITH_WARNINGS;
        }

        return ManifestParseResult::POSITIVE;
    }


    Iterator iterator() {
        return Iterator(_fileNameList);
    }

private:
    QString             _dirPath;
    ManifestSortTypes   _type;
    int                 _size;
    QStringList         _fileNameList { };
};

#endif // ORDERMANIFESTMANAGER_H
