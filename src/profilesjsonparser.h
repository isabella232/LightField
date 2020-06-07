#ifndef PROFILESJSONPARSER_H
#define PROFILESJSONPARSER_H

#include <QtCore>
#include <QtWidgets>
#include "constants.h"
#include "printprofile.h"

#define DEBUG_LOGS

class ProfilesJsonParser
{
public:
    static QMap<QString, QSharedPointer<PrintProfile>> loadProfiles()
    {
        QMap<QString, QSharedPointer<PrintProfile>> profilesList;
        QFile jsonFile(PrintProfilesPath);

        debug("ProfilesJsonParser::loadProfiles: opening file\n");

        if (!jsonFile.exists()) {
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
        foreach (const auto &value, array) {
            QJsonObject obj = value.toObject();
            QSharedPointer<PrintProfile> printProfile { new PrintProfile };

            try {
                printProfile->setProfileName(obj["name"].toString("Unknown"));
                printProfile->setDefault(obj["default"].toBool(false));
                printProfile->setActive(obj["active"].toBool(false));
                printProfile->setBuildPlatformOffset(obj["buildPlatformOffset"].toInt());
                printProfile->setDisregardFirstLayerHeight(obj["disregardFirstLayerHeight"].toBool());
                printProfile->setBaseLayerCount(obj["baseLayerCount"].toInt(2));
                printProfile->setBaseLayerParameters(_parsePrintParameters(
                    obj["baseLayerParameters"]));
                printProfile->setBodyLayerParameters(_parsePrintParameters(
                    obj["bodyLayerParameters"]));
                profilesList.insert(printProfile->profileName(), printProfile);
            }
            catch (const std::exception &ex)
            {
                debug("+ ProfilesJsonParser::loadProfiles: error\n", ex.what());
            }
        }

        debug( "ProfilesJsonParser::loadProfiles: end parsing\n" );
        return profilesList;
    }

    static void saveProfiles(const QMap<QString, QSharedPointer<PrintProfile>>& profiles)
    {
        QFile jsonFile(PrintProfilesPath);
        QJsonDocument jsonDocument;
        QJsonArray jsonArray;

        debug( "ProfilesJsonParser::saveProfiles:\n" );

        foreach (const auto &profile, profiles.values()) {
            debug( "  + serializing profile %s\n", profile->profileName().toUtf8( ).data( ) );

            QJsonObject json {
                {"name", profile->profileName()},
                {"default", profile->isDefault()},
                {"active", profile->isActive()},
                {"buildPlatformOffset", profile->buildPlatformOffset()},
                {"disregardFirstLayerHeight", profile->disregardFirstLayerHeight()},
                {"heatingTemperature", profile->heatingTemperature()},
                {"baseLayerCount", profile->baseLayerCount()},
                {"baseLayerParameters", _serializePrintParameters(profile->baseLayerParameters())},
                {"bodyLayerParameters", _serializePrintParameters(profile->bodyLayerParameters())}
            };

            jsonArray.append(json);
        }

        jsonDocument.setArray(jsonArray);
        jsonFile.open(QFile::WriteOnly);
        jsonFile.write(jsonDocument.toJson());
    }

private:
    static PrintParameters _parsePrintParameters(const QJsonValueRef &value)
    {
        PrintParameters params;
        QJsonObject obj;

        if (!value.isObject())
            return params;

        obj = value.toObject();
        params.setLayerThickness(obj["layerThickness"].toInt(100));
        params.setLayerExposureTime(obj["layerExposureTime"].toInt());
        params.setPowerLevel(obj["powerLevel"].toInt());
        params.setPumpingEnabled(obj["pumpingEnabled"].toBool(false));
        params.setPumpUpDistance(obj["pumpUpDistance"].toInt(100));
        params.setPumpUpVelocity( obj["pumpUpVelocity"].toInt(50));
        params.setPumpUpPause(obj["pumpUpPause"].toInt());
        params.setPumpDownVelocity(obj["pumpDownVelocity"].toInt(50));
        params.setPumpDownPause(obj["pumpDownPause"].toInt());
        params.setNoPumpUpVelocity(obj["noPumpUpVelocity"].toInt(50));
        params.setPumpEveryNthLayer(obj["pumpEveryNthLayer"].toInt(1));
        return params;
    }

    static QJsonObject _serializePrintParameters(const PrintParameters& params)
    {
        return QJsonObject {
            {"layerThickness", params.layerThickness()},
            {"layerExposureTime", params.layerExposureTime()},
            {"powerLevel", params.powerLevel()},
            {"pumpingEnabled", params.isPumpingEnabled()},
            {"pumpUpDistance", params.pumpUpDistance()},
            {"pumpUpVelocity", params.pumpUpVelocity_Effective()},
            {"pumpUpPause", params.pumpUpPause()},
            {"pumpDownVelocity", params.pumpDownVelocity_Effective()},
            {"pumpDownPause", params.pumpDownPause()},
            {"noPumpUpVelocity", params.noPumpUpVelocity()},
            {"pumpEveryNthLayer", params.pumpEveryNthLayer()}
        };
    }

};

#endif // PROFILESJSONPARSER_H
