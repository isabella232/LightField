#ifndef PROFILESJSONPARSER_H
#define PROFILESJSONPARSER_H

#include <QtCore>
#include <QtWidgets>
#include "constants.h"
#include "printprofile.h"
#include "window.h"

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
            QTimer::singleShot(1000, []{
                Window* window = App::mainWindow();
                QMessageBox msgBox (window);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setText("File print profile does not exist\n" );
                msgBox.exec();
            });

            debug( "ProfilesJsonParser::loadProfiles: File %s does not exist\n", PrintProfilesPath.toUtf8( ).data( ) );
            return profilesList;
        }

        jsonFile.open(QIODevice::ReadOnly);

        debug( "ProfilesJsonParser::loadProfiles: building document\n" );
        QJsonDocument jsonDocument=QJsonDocument().fromJson(jsonFile.readAll());

        if(jsonDocument.isNull()) {

            QTimer::singleShot(1000, []{
                Window* window = App::mainWindow();
                QMessageBox msgBox (window);
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.setText("File print profile is corrupted.  The file has been restored to its default state.\n" );
                msgBox.setWindowFlags(msgBox.windowFlags());
                msgBox.exec();
            });

            QSharedPointer<PrintProfile> printProfile { new PrintProfile };

            PrintParameters baseParams;

            baseParams.setLayerExposureTime(8000);
            baseParams.setPowerLevel(50);
            baseParams.setPumpingEnabled(false);
            baseParams.setPumpUpDistance(2);
            baseParams.setPumpUpVelocity(50);
            baseParams.setPumpUpPause(2000);
            baseParams.setPumpDownVelocity(50);
            baseParams.setPumpDownPause(2000);
            baseParams.setNoPumpUpVelocity(200);
            baseParams.setPumpEveryNthLayer(1);
            baseParams.setLayerThickness(100);
            baseParams.setTilingDefaultExposure(10000);
            baseParams.setTilingDefaultExposureStep(2000);

            PrintParameters bodyParams;

            bodyParams.setLayerExposureTime(8000);
            bodyParams.setPowerLevel(50);
            bodyParams.setPumpingEnabled(false);
            bodyParams.setPumpUpDistance(2);
            bodyParams.setPumpUpVelocity(50);
            bodyParams.setPumpUpPause(2000);
            bodyParams.setPumpDownVelocity(50);
            bodyParams.setPumpDownPause(2000);
            bodyParams.setNoPumpUpVelocity(200);
            bodyParams.setPumpEveryNthLayer(1);
            bodyParams.setLayerThickness(100);
            bodyParams.setTilingDefaultExposure(10000);
            bodyParams.setTilingDefaultExposureStep(2000);

            printProfile->setProfileName("default");
            printProfile->setDefault(true);
            printProfile->setActive(true);
            printProfile->setBuildPlatformOffset(300);
            printProfile->setDisregardFirstLayerHeight(false);
            printProfile->setAdvancedExposureControlsEnabled(false);
            printProfile->setBaseLayerParameters(baseParams);
            printProfile->setBodyLayerParameters(bodyParams);
            printProfile->setDigitalOffsetX(0);
            printProfile->setDigitalOffsetY(0);

            profilesList.insert(printProfile->profileName(), printProfile);
            saveProfiles(profilesList);
            return profilesList;
        }

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
                printProfile->setAdvancedExposureControlsEnabled(obj["advancedExposureEnabled"].toBool());
                printProfile->setBaseLayerParameters(_parsePrintParameters(
                    obj["baseLayerParameters"]));
                printProfile->setBodyLayerParameters(_parsePrintParameters(
                    obj["bodyLayerParameters"]));
                printProfile->setDigitalOffsetX(obj["shimX"].toInt(0));
                printProfile->setDigitalOffsetY(obj["shimY"].toInt(0));

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
                {"advancedExposureEnabled", profile->advancedExposureControlsEnabled()},
                {"baseLayerParameters", _serializePrintParameters(profile->baseLayerParameters())},
                {"bodyLayerParameters", _serializePrintParameters(profile->bodyLayerParameters())},
                {"shimX", profile->getDigitalOffsetX()},
                {"shimY", profile->getDigitalOffsetY()}
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
        params.setLayerExposureTime(obj["layerExposureTime"].toInt(1000));
        params.setPowerLevel(obj["powerLevel"].toInt(50));
        params.setPumpingEnabled(obj["pumpingEnabled"].toBool(false));
        params.setPumpUpDistance(obj["pumpUpDistance"].toDouble(1.0));
        params.setPumpUpVelocity( obj["pumpUpVelocity"].toInt(50));
        params.setPumpUpPause(obj["pumpUpPause"].toInt());
        params.setPumpDownVelocity(obj["pumpDownVelocity"].toInt(50));
        params.setPumpDownPause(obj["pumpDownPause"].toInt());
        params.setNoPumpUpVelocity(obj["noPumpUpVelocity"].toInt(50));
        params.setPumpEveryNthLayer(obj["pumpEveryNthLayer"].toInt(1));
        params.setLayerThickness(obj["layerThickness"].toInt(100));
        params.setTilingDefaultExposure(obj["tilingDefaultExposure"].toInt(10000));
        params.setTilingDefaultExposureStep(obj["tilingDefaultExposureStep"].toInt(2000));
        return params;
    }

    static QJsonObject _serializePrintParameters(const PrintParameters& params)
    {
        return QJsonObject {
            {"layerExposureTime", params.layerExposureTime()},
            {"powerLevel", params.powerLevel()},
            {"pumpingEnabled", params.isPumpingEnabled()},
            {"pumpUpDistance", params.pumpUpDistance()},
            {"pumpUpVelocity", params.pumpUpVelocity_Effective()},
            {"pumpUpPause", params.pumpUpPause()},
            {"pumpDownVelocity", params.pumpDownVelocity_Effective()},
            {"pumpDownPause", params.pumpDownPause()},
            {"noPumpUpVelocity", params.noPumpUpVelocity()},
            {"pumpEveryNthLayer", params.pumpEveryNthLayer()},
            {"layerThickness", params.layerThickness()},
            {"tilingDefaultExposure", params.tilingDefaultExposure()},
            {"tilingDefaultExposureStep", params.tilingDefaultExposureStep()}
        };
    }

};

#endif // PROFILESJSONPARSER_H
