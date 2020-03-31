#ifndef PROFILESJSONPARSER_H
#define PROFILESJSONPARSER_H

#include "printprofile.h"

#define DEBUG_LOGS

class ProfilesJsonParser
{
public:
    static QVector<PrintProfile*>* loadProfiles() {
        QVector<PrintProfile*>* profilesList = new QVector<PrintProfile*>();

        debug( "ProfilesJsonParser::loadProfiles: opening file\n" );
        QFile jsonFile(PrintProfilesPath);
        if(!jsonFile.exists())
        {
            debug( "ProfilesJsonParser::loadProfiles: File %s does not exist\n", PrintProfilesPath.toUtf8( ).data( ) );
            return profilesList;
        }
        jsonFile.open(QIODevice::ReadOnly);

        debug( "ProfilesJsonParser::loadProfiles: building document\n" );
        QJsonDocument jsonDocument=QJsonDocument().fromJson(jsonFile.readAll());

        debug( "ProfilesJsonParser::loadProfiles: get root array\n" );
        QJsonArray array = jsonDocument.array();

        debug( "ProfilesJsonParser::loadProfiles: iter over array: %d elements\n", array.count() );
        for(int i=0; i<array.count(); ++i) {
            PrintProfile* printProfile = new PrintProfile();
            try {

                QJsonObject obj = array[i].toObject();

                debug( "ProfilesJsonParser::loadProfiles: element %d\n", i );
                debug( "  + get name\n");
                auto name = obj["name"];

                if(name.isString()) {
                    printProfile->setProfileName(obj["name"].toString());
                }
                else
                {
                    debug( "ProfilesJsonParser::loadProfiles: Error while parsing name field" );
                    throw new QException();
                }

                debug( "  + get baseLayerCount\n" );
                auto blc = obj["baseLayerCount"];

                if(blc.isDouble())
                    printProfile->setBaseLayerCount(blc.toInt());
                else
                {
                    debug( "ProfilesJsonParser::loadProfiles: Error while parsing baseLayerCount field" );
                    throw new QException();
                }

                debug( "  + get baseLayerParameters\n" );
                auto baseLayerParameters = obj["baseLayerParameters"];

                if(!baseLayerParameters.isUndefined() && !baseLayerParameters.isNull())
                {
                    PrintParameters params = _parsePrintParameters(baseLayerParameters.toObject());
                    printProfile->setBaseLayerParameters(params);
                }

                debug( "  + get bodyLayerParameters\n" );
                auto bodyLayerParameters = obj["bodyLayerParameters"];

                if(!bodyLayerParameters.isUndefined() && !bodyLayerParameters.isNull())
                {
                    PrintParameters params = _parsePrintParameters(bodyLayerParameters.toObject());
                    printProfile->setBodyLayerParameters(params);
                }

                debug( "  + push back to list\n" );
                profilesList->push_back(printProfile);
            }
            catch (QException*)
            {
                delete printProfile;
            }
            catch (...)
            {
                debug( "ProfilesJsonParser::loadProfiles: Unknown error while parsing print profile\n" );
                delete printProfile;
            }
        }

        debug( "ProfilesJsonParser::loadProfiles: end parsing\n" );
        return profilesList;
    }

    static void saveProfiles( const QVector<PrintProfile*>* profiles ) {
        QFile         jsonFile( PrintProfilesPath );
        QJsonDocument jsonDocument;
        QJsonArray    jsonArray;

        debug( "ProfilesJsonParser::saveProfiles:\n" );
        debug( "  + loop start: %d profiles\n", profiles->count( ) );
        for ( int i = 0; i < profiles->count( ); ++i ) {
            auto profile = ( *profiles )[i];

            debug( "  + serializing profile %s\n", profile->profileName( ).toUtf8( ).data( ) );
            if ( profile->profileName( ) == "temp" ) {
                continue;
            }

            QJsonObject json;
            json["name"]                 = profile->profileName( );
            json["baseLayerCount"]       = profile->baseLayerCount( );
            json["baseLayerParameters"] = _serializePrintParameters( profile->baseLayerParameters( ) );
            json["bodyLayerParameters"] = _serializePrintParameters( profile->bodyLayerParameters( ) );

            jsonArray.append( json );
        }

        jsonDocument.setArray( jsonArray );

        jsonFile.open( QFile::WriteOnly );
        jsonFile.write( jsonDocument.toJson( ) );
    }

private:

    static PrintParameters _parsePrintParameters( QJsonObject obj ) {
        PrintParameters params;

        params.setLayerThickness   ( _parseDoubleValue( obj, "layerThickness"    ) );
        params.setLayerExposureTime( _parseDoubleValue( obj, "layerExposureTime" ) );
        params.setPowerLevel       ( _parseIntValue   ( obj, "powerLevel"        ) );
        params.setPumpingEnabled   ( _parseBoolValue  ( obj, "pumpingEnabled"    ) );
        params.setPumpUpDistance   ( _parseIntValue   ( obj, "pumpUpDistance"    ) );
        params.setPumpUpTime       ( _parseDoubleValue( obj, "pumpUpTime"        ) );
        params.setPumpUpPause      ( _parseDoubleValue( obj, "pumpUpPause"       ) );
        params.setPumpDownPause    ( _parseDoubleValue( obj, "pumpDownPause"     ) );
        params.setNoPumpUpVelocity ( _parseDoubleValue( obj, "noPumpUpVelocity"  ) );
        params.setPumpEveryNthLayer( _parseDoubleValue( obj, "pumpEveryNthLayer" ) );

        return params;
    }

    static bool _parseBoolValue( QJsonObject const& obj, QString const& propertyName ) {
        QJsonValue value { obj[propertyName] };
        if ( value.isBool( ) )
            return value.toBool( );
        else {
            debug( "ProfilesJsonParser::_parseBoolValue: Error while parsing field %s\n", propertyName.toUtf8( ).data( ) );
            throw new QException( );
        }
    }

    static int _parseIntValue( QJsonObject const& obj, QString const& propertyName ) {
        QJsonValue value { obj[propertyName] };
        if ( value.isDouble( ) )
            return value.toInt( );
        else {
            debug( "ProfilesJsonParser::_parseIntValue: Error while parsing field %s\n", propertyName.toUtf8( ).data( ) );
            throw new QException( );
        }
    }

    static double _parseDoubleValue( QJsonObject const& obj, QString const& propertyName ) {
        QJsonValue value { obj[propertyName] };
        if ( value.isDouble( ) )
            return value.toDouble( );
        else {
            debug( "ProfilesJsonParser::_parseDoubleValue: Error while parsing field %s\n", propertyName.toUtf8( ).data( ) );
            throw new QException( );
        }
    }

    static QJsonObject _serializePrintParameters( PrintParameters const& params ) {
        QJsonObject result;

        result["layerThickness"]    = params.layerThickness( );
        result["layerExposureTime"] = params.layerExposureTime( );
        result["powerLevel"]        = params.powerLevel( );
        result["pumpingEnabled"]    = params.pumpingEnabled( );
        result["pumpUpDistance"]    = params.pumpUpDistance( );
        result["pumpUpTime"]        = params.pumpUpTime( );
        result["pumpUpPause"]       = params.pumpUpPause( );
        result["pumpDownPause"]     = params.pumpDownPause( );
        result["noPumpUpVelocity"]  = params.noPumpUpVelocity( );
        result["pumpEveryNthLayer"] = params.pumpEveryNthLayer( );

        return result;
    }

};

#endif // PROFILESJSONPARSER_H
