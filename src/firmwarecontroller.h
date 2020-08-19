#ifndef FIRMWARECONTROLLER_H
#define FIRMWARECONTROLLER_H

#include <QtCore>
#include <QSerialPort>
#include <QList>
#include <QMap>
#include <QTimer>
#include "debug.h"
#include "constants.h"

enum class FirmwareCommandType {
    LINEAR_MOVE,
    AUTO_HOME,
    ABSOLUTE_POSITIONING,
    RELATIVE_POSITIONING,
    DISABLE_STEPPERS,
    REPORT_TEMPERATURES,
    SET_FAN_SPEED,
    GET_CURRENT_POSITION,
    SET_BED_TEMPERATURE,
    TEMPERATURE_AUTO_REPORT,
    FINISH_MOVES
};

enum class FirmwareComplexCommandType {
    MOVE_RELATIVE,
    MOVE_ABSOLUTE,
    MOVE_HOME,
    INITIALIZE,
    SET_BED_TEMPERATURE
};

static const QMap<FirmwareCommandType, QString> firmwareCommands = {
    {FirmwareCommandType::LINEAR_MOVE, "G0 X%1 F%2\n"},
    {FirmwareCommandType::AUTO_HOME, "G28 X\n"},
    {FirmwareCommandType::ABSOLUTE_POSITIONING, "G90\n"},
    {FirmwareCommandType::RELATIVE_POSITIONING, "G91\n"},
    {FirmwareCommandType::DISABLE_STEPPERS, "M18\n"},
    {FirmwareCommandType::REPORT_TEMPERATURES, "M105\n"},
    {FirmwareCommandType::SET_FAN_SPEED, "M106\n"},
    {FirmwareCommandType::GET_CURRENT_POSITION, "M114\n"},
    {FirmwareCommandType::SET_BED_TEMPERATURE, "M140 S%1\n"},
    {FirmwareCommandType::TEMPERATURE_AUTO_REPORT, "M155 S%1\n"},
    {FirmwareCommandType::FINISH_MOVES, "M400\n"}
};

static const QMap<FirmwareComplexCommandType, QList<FirmwareCommandType>> complexFirmwareCommands = {
    {FirmwareComplexCommandType::MOVE_RELATIVE, {
        FirmwareCommandType::RELATIVE_POSITIONING,
        FirmwareCommandType::LINEAR_MOVE,
        FirmwareCommandType::FINISH_MOVES,
        FirmwareCommandType::GET_CURRENT_POSITION
    }},
    {FirmwareComplexCommandType::MOVE_ABSOLUTE, {
        FirmwareCommandType::ABSOLUTE_POSITIONING,
        FirmwareCommandType::LINEAR_MOVE,
        FirmwareCommandType::FINISH_MOVES,
        FirmwareCommandType::GET_CURRENT_POSITION
    }},
    {FirmwareComplexCommandType::MOVE_HOME, {
        FirmwareCommandType::AUTO_HOME,
        FirmwareCommandType::FINISH_MOVES,
        FirmwareCommandType::GET_CURRENT_POSITION
    }},
    {FirmwareComplexCommandType::INITIALIZE, {
        FirmwareCommandType::DISABLE_STEPPERS,
        FirmwareCommandType::SET_FAN_SPEED,
        FirmwareCommandType::TEMPERATURE_AUTO_REPORT
    }},
    {FirmwareComplexCommandType::SET_BED_TEMPERATURE, {
        FirmwareCommandType::SET_BED_TEMPERATURE
    }}
};

class FirmwareCommand {
public:
   FirmwareCommand(FirmwareCommandType type, QStringList const & args): _args(args), _type(type) {
        _command = firmwareCommands[type];

        foreach (const QString &i, args)
            _command = _command.arg(i);
    }
    FirmwareCommand(const FirmwareCommand &other) = default;
    ~FirmwareCommand() = default;

    const QString& getCommand() const {
        return _command;
    }

    const char* getCommandName() const {
        switch (getType()) {
        case (FirmwareCommandType::LINEAR_MOVE):
            return "LINEAR_MOVE";
        case (FirmwareCommandType::AUTO_HOME):
            return "AUTO_HOME";
        case (FirmwareCommandType::ABSOLUTE_POSITIONING):
            return "ABSOLUTE_POSITIONING";
        case (FirmwareCommandType::RELATIVE_POSITIONING):
            return "RELATIVE_POSITIONING";
        case (FirmwareCommandType::DISABLE_STEPPERS):
            return "DISABLE_STEPPERS";
        case (FirmwareCommandType::REPORT_TEMPERATURES):
            return "REPORT_TEMPERATURES";
        case (FirmwareCommandType::SET_FAN_SPEED):
            return "SET_FAN_SPEED";
        case (FirmwareCommandType::GET_CURRENT_POSITION):
            return "GET_CURRENT_POSITION";
        case (FirmwareCommandType::SET_BED_TEMPERATURE):
            return "SET_BED_TEMPERATURE";
        case (FirmwareCommandType::TEMPERATURE_AUTO_REPORT):
            return "TEMPERATURE_AUTO_REPORT";
        case (FirmwareCommandType::FINISH_MOVES):
            return "FINISH_MOVES";
        }

        return "UNKNOWN";
    }

    const QStringList& getArgs() const {
        return _args;
    }

    FirmwareCommandType getType() const {
        return _type;
    }

    int getLength() const {
        return _command.length();
    }

    bool isSent() const {
        return _sent;
    }

    void setSent() {
        _sent = true;
        debug("+ FirmwareCommand::setSent: command sent: %s", _command.toUtf8().data());
    }

protected:
    QString _command;
    QStringList _args;
    FirmwareCommandType _type;
    bool _sent { false };
};

class FirmwareComplexCommand {
public:
    FirmwareComplexCommand(FirmwareComplexCommandType const &type, QStringList args):
        _type(type), _args(args) {

        _commands = complexFirmwareCommands[type];
    }
    FirmwareComplexCommand(const FirmwareComplexCommand &other) = default;
    ~FirmwareComplexCommand() = default;

    const char* getCommandName() const {
        switch (getType()) {
        case (FirmwareComplexCommandType::MOVE_HOME):
            return "MOVE_HOME";
        case (FirmwareComplexCommandType::INITIALIZE):
            return "INITIALIZE";
        case (FirmwareComplexCommandType::MOVE_ABSOLUTE):
            return "MOVE_ABSOLUTE";
        case (FirmwareComplexCommandType::MOVE_RELATIVE):
            return "MOVE_RELATIVE";
        case (FirmwareComplexCommandType::SET_BED_TEMPERATURE):
            return "SET_BED_TEMPERATURE";
        }

        return "UNKNOWN";
    }

    FirmwareComplexCommandType getType() const {
        return _type;
    }

    FirmwareCommandType popNextSubcommand() {
        return _commands.takeFirst();
    }

    bool isFinished() const {
        return _commands.isEmpty();
    }

protected:
    FirmwareComplexCommandType _type;
    QList<FirmwareCommandType> _commands;
    QStringList _args;
};

class FirmwareController: public QObject {
    Q_OBJECT
public:
    FirmwareController(QObject *parent, QString const& portPath, int baudrate);

    void moveRelative(double dist, double speed);
    void moveAbsolute(double position, double speed);
    void moveHome();
    void setTemperature(int temp);
    void init();
    void close();

protected:
    DebugManager firmwareDebugManager {DebugType::FIRMWARE, FirmwareLogPaths};
    QString _portPath;
    QSerialPort _serial;
    QQueue<FirmwareCommand> _cmdQueue;
    QQueue<FirmwareComplexCommand> _cplxCmdQueue;
    int _baudrate;
    int _writeCount { 0 };
#if defined _DEBUG

    QTimer _tempReportTimer;
    double _zPosition { 0.0 };
    double _targetTemp { 0.0 };
    int _tempReportInterval { 0 };
    bool _reportTemp { false };
    bool _reportPosition { false };

    void mockTempAutoreport();
#endif // defined _DEBUG
    bool _online { false };

    void sendComplexCommand(FirmwareComplexCommandType type, QStringList args);
    int sendCommand(FirmwareCommandType type, QStringList args);

    void resetFirmware();
    void parseTemperatureReport(QString const &line);
    void parsePosition(QString const &line);
    FirmwareCommand & getFirstUnsent();

signals:
    void printerOnline();
    void printerOffline();
    void printerPositionReport(double px, int cx);
    void printerTemperatureReport(double bedCurrentTemperature, double bedTargetTemperature,
        int bedPwm);
    void printerFirmwareVersionReport(QString const& version);
    void printerMoveRelativeCompleted(bool successful);
    void printerMoveAbsoluteCompleted(bool successful);
    void printerHomeCompleted(bool successful);
    void printerInitCompleted(bool successful);
    void printerSetTemperatureCompleted(bool succesful);

    void printerCommandCompleted(FirmwareCommandType cmd, bool successful);
    void printerComplexCommandCompleted(FirmwareComplexCommandType cmd, bool successful);
    void serialResetCompleted();

protected slots:
    void serialBytesWritten(int bytes);
    void serialDataReady();
    void serialDTRTimerTimeout();
    void printerInitialize();
};

#endif // FIRMWARECONTROLLER_H
