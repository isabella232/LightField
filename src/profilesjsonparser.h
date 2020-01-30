#ifndef PROFILESJSONPARSER_H
#define PROFILESJSONPARSER_H

#include "printprofile.h"
#include <iostream>

#define DEBUG_LOGS

class ProfilesJsonParser
{
public:
    static QVector<PrintProfile*>* loadProfiles() {
        QVector<PrintProfile*>* profilesList = new QVector<PrintProfile*>();

        log( "opening file ..." );
        QFile jsonFile(PrintProfilesPath);
        if(!jsonFile.exists())
        {
            std::cerr << "File " << PrintProfilesPath.toStdString() << " does not exists" << std::endl;
            return profilesList;
        }
        jsonFile.open(QIODevice::ReadOnly);

        log( "building document ..." );
        QJsonDocument jsonDocument=QJsonDocument().fromJson(jsonFile.readAll());

        log( "get root array ..." );
        QJsonArray array = jsonDocument.array();

        log( "iter over array ..." );
        for(int i=0; i<array.count(); ++i) {
                try {
                    PrintProfile* printProfile = new PrintProfile();

                    QJsonObject obj = array.takeAt(i).toObject();

                    log( "get name ..." );
                    auto name = obj["name"];

                    if(name.isString()) {
                        printProfile->setProfileName(obj["name"].toString());
                    }
                    else
                    {
                        std::cerr<<"Error while parsing name field";
                        throw new QException();
                    }

                    log( "get baseLayerCount ..." );
                    auto blc = obj["baseLayerCount"];

                    if(blc.isDouble())
                        printProfile->setBaseLayerCount(blc.toInt());
                    else
                    {
                        std::cerr<<"Error while parsing baseLayerCount field";
                        throw new QException();
                    }

                    log( "get baseLayersPumpingParameters ..." );
                    auto baseLayersPumpingParameters = obj["baseLayersPumpingParameters"];

                    if(!baseLayersPumpingParameters.isNull())
                    {
                        PrintPumpingParameters params = _parsePrintPumpingParameters(baseLayersPumpingParameters.toObject());
                        printProfile->setBaseLayersPumpingParameters(params);
                    }
                    else
                    {
                        std::cerr<<"Error while parsing baseLayersPumpingParameters field";
                        throw new QException();
                    }

                    log( "get bodyLayersPumpingParameters ..." );
                    auto bodyLayersPumpingParameters = obj["bodyLayersPumpingParameters"];

                    if(!bodyLayersPumpingParameters.isNull())
                    {
                        PrintPumpingParameters params = _parsePrintPumpingParameters(bodyLayersPumpingParameters.toObject());
                        printProfile->setBodyLayersPumpingParameters(params);
                    }
                    else
                    {
                        std::cerr<<"Error while parsing bodyLayersPumpingParameters field";
                        throw new QException();
                    }

                    log( "push back to list ..." );
                    profilesList->push_back(printProfile);
                }
                catch (QException e) { }
                catch (...)
                {
                    std::cerr<<"Unknow error while parsing print profile";
                }
            }

        log( "end parsing ..." );
        return profilesList;
    }

    static void saveProfiles(const QVector<PrintProfile*>* profiles)
    {
        QFile jsonFile(PrintProfilesPath);
        QJsonDocument jsonDocument=QJsonDocument();

        QJsonArray jsonArray;


        for(int i=0; i<profiles->count(); ++i)
        {
            auto profile = (*profiles)[i];
            QJsonObject json;

            json["name"] = profile->profileName();
            json["baseLayerCount"] = profile->baseLayerCount();

            QJsonObject baseParameters = _serializePrintPumpingParameters(profile->baseLayersPumpingParameters());
            json["baseLayersPumpingParameters"]= baseParameters;

            QJsonObject bodyParameters = _serializePrintPumpingParameters(profile->bodyLayersPumpingParameters());
            json["bodyLayersPumpingParameters"]= bodyParameters;

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
           std::cerr<<"Error while parsing " << propertyName.toStdString() << " field";
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
           std::cerr<<"Error while parsing " << propertyName.toStdString() << " field";
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

    static void log(std::string message)
    {
#ifdef DEBUG_LOGS
        std::cout << message << std::endl;
#endif
    }
};

#endif // PROFILESJSONPARSER_H
