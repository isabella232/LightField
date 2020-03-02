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
            // Show message when not found a profile in path
            // TODO: Create file with default profile
            QMessageBox msgBox;
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setText("File print profile does not exist\n" );
            msgBox.open();
            msgBox.exec();

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

                debug( "  + get baseLayersPumpingParameters\n" );
                auto baseLayersPumpingParameters = obj["baseLayersPumpingParameters"];

                if(!baseLayersPumpingParameters.isUndefined() && !baseLayersPumpingParameters.isNull())
                {
                    PrintPumpingParameters params = _parsePrintPumpingParameters(baseLayersPumpingParameters.toObject());
                    printProfile->setBaseLayersPumpingParameters(params);
                    printProfile->setBaseLayersPumpingEnabled(true);
                }
                else
                    printProfile->setBaseLayersPumpingEnabled(false);

                debug( "  + get bodyLayersPumpingParameters\n" );
                auto bodyLayersPumpingParameters = obj["bodyLayersPumpingParameters"];

                if(!bodyLayersPumpingParameters.isUndefined() && !bodyLayersPumpingParameters.isNull())
                {
                    PrintPumpingParameters params = _parsePrintPumpingParameters(bodyLayersPumpingParameters.toObject());
                    printProfile->setBodyLayersPumpingParameters(params);
                    printProfile->setBodyLayersPumpingEnabled(true);
                }
                else
                    printProfile->setBodyLayersPumpingEnabled(false);

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

    static void saveProfiles(const QVector<PrintProfile*>* profiles)
    {
        debug( "ProfilesJsonParser::saveProfiles:\n" );
        QFile jsonFile(PrintProfilesPath);

        QJsonDocument jsonDocument=QJsonDocument();

        QJsonArray jsonArray;

        debug( "  + loop start: %d profiles\n", profiles->count( ) );
        for(int i=0; i<profiles->count(); ++i)
        {
            auto profile = (*profiles)[i];

            debug( "  + serializing profile %s\n", profile->profileName( ).toUtf8( ).data( ) );
            if(profile->profileName() == "temp")
                continue;

            QJsonObject json;

            json["name"] = profile->profileName();
            json["baseLayerCount"] = profile->baseLayerCount();

            if(profile->baseLayersPumpingEnabled())
            {
                QJsonObject baseParameters = _serializePrintPumpingParameters(profile->baseLayersPumpingParameters());
                json["baseLayersPumpingParameters"]= baseParameters;
            }

            if(profile->bodyLayersPumpingEnabled())
            {
                QJsonObject bodyParameters = _serializePrintPumpingParameters(profile->bodyLayersPumpingParameters());
                json["bodyLayersPumpingParameters"]= bodyParameters;
            }

            jsonArray.append(json);
        }

        jsonDocument.setArray(jsonArray);

        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(jsonDocument.toJson());
    }

private:
    static PrintPumpingParameters _parsePrintPumpingParameters(QJsonObject obj)
    {
        PrintPumpingParameters params;

        params.setPumpUpDistance( _parseIntValue(obj, QString("pumpUpDistance")) );
        params.setPumpUpTime( _parseDoubleValue(obj, QString("pumpUpTime")) );
        params.setPumpUpPause( _parseDoubleValue(obj, QString("pumpUpPause")) );
        params.setPumpDownPause( _parseDoubleValue(obj, QString("pumpDownPause")) );
        params.setNoPumpUpVelocity( _parseDoubleValue(obj, QString("noPumpUpVelocity")) );
        params.setPumpEveryNthLayer( _parseDoubleValue(obj, QString("pumpEveryNthLayer")) );
        params.setLayerThickness( _parseDoubleValue(obj, QString("layerThickness")) );
        params.setLayerExposureTime( _parseDoubleValue(obj, QString("layerExposureTime")) );
        params.setPowerLevel( _parseIntValue(obj, QString("powerLevel")) );

        return params;
    }

    static int _parseIntValue(QJsonObject obj, QString propertyName)
    {
        QJsonValue value;

        value = obj[propertyName];

        if(value.isDouble())
           return value.toInt();
        else
        {
            debug( "ProfilesJsonParser::_parseIntValue: Error while parsing field %s\n", propertyName.toUtf8( ).data( ) );
            throw new QException();
        }
    }

    static double _parseDoubleValue(QJsonObject obj, QString propertyName)
    {
        QJsonValue value;

        value = obj[propertyName];

        if(value.isDouble())
           return value.toDouble();
        else
        {
            debug( "ProfilesJsonParser::_parseDoubleValue: Error while parsing field %s\n", propertyName.toUtf8( ).data( ) );
            throw new QException();
        }
    }

    static QJsonObject _serializePrintPumpingParameters(PrintPumpingParameters params)
    {
        QJsonObject result;

        result["pumpUpDistance"]=params.pumpUpDistance();
        result["pumpUpTime"]=params.pumpUpTime();
        result["pumpUpPause"]=params.pumpUpPause();
        result["pumpDownPause"]=params.pumpDownPause();
        result["noPumpUpVelocity"]=params.noPumpUpVelocity();
        result["pumpEveryNthLayer"]=params.pumpEveryNthLayer();
        result["layerThickness"]=params.layerThickness();
        result["layerExposureTime"]=params.layerExposureTime();
        result["powerLevel"]=params.powerLevel();

        return result;
    }

};

#endif // PROFILESJSONPARSER_H
