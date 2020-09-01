#include <QtCore>
#include <QSerialPort>
#include <QTimer>
#include "firmwarecontroller.h"
#include "debug.h"
#include "app.h"
#include "constants.h"

static QRegularExpression FirmwareVersionMatcher {"^echo:.*?Author:\\s*(.+?)(?:\\s|;|$)",
    QRegularExpression::CaseInsensitiveOption};
static QRegularExpression PositionReportMatcher {
    "^X:(-?\\d+\\.\\d\\d) Y:(-?\\d+\\.\\d\\d) Z:(-?\\d+\\.\\d\\d) E:(-?\\d+\\.\\d\\d) Count X:(-?\\d+) Y:(-?\\d+) Z:(-?\\d+)",
    QRegularExpression::CaseInsensitiveOption};
static QRegularExpression TemperatureReport1Matcher {
    "^T:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) B:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) @:(-?\\d+) B@:(-?\\d+)",
    QRegularExpression::CaseInsensitiveOption };
static QRegularExpression TemperatureReport2Matcher {
    "^T:(-?\\d+\\.\\d\\d)\\s*/(-?\\d+\\.\\d\\d) @:(-?\\d+)",
    QRegularExpression::CaseInsensitiveOption };

FirmwareController::FirmwareController(QObject *parent, QString const & portPath, int baudrate):
    QObject(parent),
    _portPath(portPath),
    _baudrate(baudrate)
{

    QObject::connect(&_serial, &QSerialPort::readyRead, this, &FirmwareController::serialDataReady);
    QObject::connect(this, &FirmwareController::printerOnline, this,
        &FirmwareController::printerInitialize);
#if defined _DEBUG
    QObject::connect(&_tempReportTimer, &QTimer::timeout, this, &FirmwareController::mockTempAutoreport);
#endif // defined _DEBUG
}

void FirmwareController::init()
{
    _serial.setPortName(_portPath);
    _serial.setBaudRate(_baudrate);
#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        debug("+ FirmwareController::init: mocking reset\n");
        resetFirmware();
        return;
    }

#endif // defined _DEBUG
    Q_ASSERT(_serial.open(QIODevice::ReadWrite));
    debug("+ FirmwareController::init: serial port open - baud %d, path %s\n", _serial.baudRate(),
        _serial.portName().toUtf8().data());
    debug("+ FirmwareController::init: serial port open - resetting firmware\n");
    resetFirmware();
}

void FirmwareController::close()
{
    _online = false;
    emit printerOffline();

#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        return;
    }
#endif // defined _DEBUG
    _serial.close();
}

void FirmwareController::moveRelative(double dist, double speed)
{
    QStringList args = {QString::number(dist, 'f', 2), QString::number(speed, 'f', 2)};
    sendComplexCommand(FirmwareComplexCommandType::MOVE_RELATIVE, args);
    debug("+ FirmwareController::moveRelative: command queued - distance: %.2f, speed: %.2f\n",
        dist, speed);
}

void FirmwareController::moveAbsolute(double position, double speed)
{
    QStringList args = {QString::number(position, 'f', 2), QString::number(speed, 'f', 2)};
    sendComplexCommand(FirmwareComplexCommandType::MOVE_ABSOLUTE, args);
    debug("+ FirmwareController::moveAbsolute: command queued - position: %.2f, speed: %.2f\n",
        position, speed);
}

void FirmwareController::moveHome()
{
    sendComplexCommand(FirmwareComplexCommandType::MOVE_HOME, QStringList());
    debug("+ FirmwareController::moveHome: command queued");
}

void FirmwareController::setTemperature(int temp)
{
    QStringList args = {QString::number(temp)};
    sendComplexCommand(FirmwareComplexCommandType::SET_BED_TEMPERATURE, args);
    debug("+ FirmwareController::setTemperature: command queued - temperature: %d\n", temp);
}

void FirmwareController::sendComplexCommand(FirmwareComplexCommandType type, QStringList args)
{
    QRegExp arg_rx("%\\d{1,2}(?!\\d)");
    int arg_pos = 0;

    _cplxCmdQueue.enqueue(FirmwareComplexCommand(type, args));

    foreach (const FirmwareCommandType &command, complexFirmwareCommands[type]) {
        int arg_cnt = firmwareCommands[command].count(arg_rx);
        QStringList sub_args = args.mid(arg_pos, arg_cnt);
        arg_pos += arg_cnt;

        if (sendCommand(command, sub_args) != 0) {
            debug("+ FirmwareController::sendComplexCommand: command discarded\n");
            resetFirmware();
            return;
        }
    }
}

int FirmwareController::sendCommand(FirmwareCommandType type, QStringList args)
{
    const FirmwareCommand cmd = FirmwareCommand(type, args);

#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        int resp_delay = 100;
        double shift = 1;
        double speed = 1;

        _cmdQueue.enqueue(cmd);

        if (type == FirmwareCommandType::LINEAR_MOVE) {
            double position = args[0].toDouble();
            speed = args[1].toDouble();

            if (_cplxCmdQueue.head().getType() == FirmwareComplexCommandType::MOVE_ABSOLUTE) {
                shift = abs(_zPosition - position);
                _zPosition = position;
            } else {
                shift = _zPosition + position;
                _zPosition = shift;
            }

        } else if (type == FirmwareCommandType::AUTO_HOME) {
            shift = _zPosition;
            speed = PrinterDefaultHighSpeed;
            _zPosition = 0;

        } else if (type == FirmwareCommandType::SET_BED_TEMPERATURE) {
            _targetTemp = args[0].toDouble();

        } else if (type == FirmwareCommandType::TEMPERATURE_AUTO_REPORT) {
            _tempReportInterval = args[0].toInt();

            if (_tempReportInterval == 0)
                _tempReportTimer.stop();
            else
                _tempReportTimer.start(_tempReportInterval * 1000);
        }

        if ((type == FirmwareCommandType::LINEAR_MOVE) || (type == FirmwareCommandType::AUTO_HOME))
            resp_delay = static_cast<int>((shift / speed) * 60000);

        QTimer::singleShot(resp_delay, this, SLOT(serialDataReady()));
        return 0;
    }

#endif // defined _DEBUG
    if (_serial.isOpen()) {
        if (_cmdQueue.isEmpty()) {
            _serial.write(cmd.getCommand().toUtf8());
            firmware_debug(cmd.getCommand().toUtf8().data());
        }

        _cmdQueue.enqueue(cmd);
        debug("+ FirmwareController::sendCommand: command queued: %s\n", cmd.getCommand().toUtf8().data());
    } else {
        debug("+ FirmwareController::sendCommand: discarding command: %s - serial port closed\n",
            cmd.getCommand().toUtf8().data());

        resetFirmware();
        return -1;
    }

    return 0;
}

void FirmwareController::resetFirmware()
{
    if (_online) {
        _online = false;
        debug("+ FirmwareController::resetFirmware: printer offline\n");
        emit printerOffline();
    }

    foreach (const FirmwareComplexCommand &cmd, _cplxCmdQueue) {
        switch (cmd.getType()) {
        case (FirmwareComplexCommandType::MOVE_RELATIVE):
            emit printerMoveRelativeCompleted(false);
            break;
        case (FirmwareComplexCommandType::MOVE_ABSOLUTE):
            emit printerMoveAbsoluteCompleted(false);
            break;
        case (FirmwareComplexCommandType::MOVE_HOME):
            emit printerHomeCompleted(false);
            break;
        case (FirmwareComplexCommandType::INITIALIZE):
            emit printerInitCompleted(false);
            break;
        case (FirmwareComplexCommandType::SET_BED_TEMPERATURE):
            emit printerSetTemperatureCompleted(false);
            break;
        }

        emit printerComplexCommandCompleted(cmd.getType(), false);
    }
    _cplxCmdQueue.clear();

    foreach (const FirmwareCommand &cmd, _cmdQueue)
        emit printerCommandCompleted(cmd.getType(), false);
    _cmdQueue.clear();
    debug("+ FirmwareController::resetFirmware: resetting firmware - DTR inactive\n");
#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        QTimer::singleShot(200, this, SLOT(serialDTRTimerTimeout()));
        return;
    }
#endif // defined _DEBUG

    _serial.setDataTerminalReady(false);
    QTimer::singleShot(200, this, SLOT(serialDTRTimerTimeout()));
}

void FirmwareController::serialDTRTimerTimeout()
{
    debug("+ FirmwareController::serialDTRTimerTimeout: firmware reset completed - DTR active\n");
#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        emit serialResetCompleted();
        
        serialDataReady();
        return;
    }
#endif // defined _DEBUG

    _serial.setDataTerminalReady(true);
    emit serialResetCompleted();
}

void FirmwareController::printerInitialize()
{
    sendComplexCommand(FirmwareComplexCommandType::INITIALIZE, QStringList("5"));
    debug("+ FirmwareController::printerInitialize: command queued\n");
}

void FirmwareController::serialDataReady()
{
    QString line;
    QByteArray raw_data;

#if defined _DEBUG
    if (g_settings.pretendPrinterIsOnline) {
        if (!_online) {
            line = QString("start\n");
            goto skip_serial;
        }
        
        if (_reportTemp) {
            line = QString(" T:25.00 /0.00 B:%1 /%2 @:0 B@:127\n");
            line = line.arg(QString::number(_targetTemp > 0 ? _targetTemp : 25.0, 'f', 2));
            line = line.arg(QString::number(_targetTemp, 'f', 2));
            _reportTemp = false;
            goto skip_serial;
        }

        if (_reportPosition) {
            line = QString("X:%1 Y:0.00 Z:0.00 E:0.00 Count X:0 Y:0 Z:0\n");
            line = line.arg(QString::number(_zPosition, 'f', 2));
            _reportPosition = false;
            goto skip_serial;
        }

        Q_ASSERT(!_cmdQueue.empty());
        FirmwareCommand &next_cmd = _cmdQueue.head();

        switch (next_cmd.getType()) {
        case (FirmwareCommandType::LINEAR_MOVE):
        case (FirmwareCommandType::AUTO_HOME):
        case (FirmwareCommandType::ABSOLUTE_POSITIONING):
        case (FirmwareCommandType::RELATIVE_POSITIONING):
        case (FirmwareCommandType::DISABLE_STEPPERS):
        case (FirmwareCommandType::SET_FAN_SPEED):
        case (FirmwareCommandType::SET_BED_TEMPERATURE):
        case (FirmwareCommandType::TEMPERATURE_AUTO_REPORT):
        case (FirmwareCommandType::FINISH_MOVES):
            line = QString("ok\n");
            break;
        case (FirmwareCommandType::REPORT_TEMPERATURES):
            line = QString("ok T:25.00 /0.00 B:%1 /%2 @:0 B@:0\n");
            line = line.arg(QString::number(_targetTemp > 0 ? _targetTemp : 25.0, 'f', 2));
            line = line.arg(QString::number(_targetTemp, 'f', 2));
            break;
        case (FirmwareCommandType::GET_CURRENT_POSITION):
            line = QString("ok\n");
            _reportPosition = true;
            QTimer::singleShot(10, this, SLOT(serialDataReady()));
            break;
        }

        goto skip_serial;
    }
#endif // defined _DEBUG

    while (_serial.canReadLine()) {

        raw_data = _serial.readLine();
        debug("+ FirmwareController::serialDataReady: raw_data - %s", raw_data.constData());
        line = QString::fromUtf8(raw_data);
skip_serial:
        debug("+ FirmwareController::serialDataReady: firmware response: %s", line.toUtf8().data());
        firmware_debug(line.toUtf8().data());

        if (!_online) {
            if (line.contains("start") || line.contains("Grbl") || line.contains("ok")) {
                debug("+ FirmwareController::serialDataReady: printer online\n");
                _online = true;
                emit printerOnline();
                return;
            }
        }

        if (line.startsWith("ok")) {
            Q_ASSERT(!_cmdQueue.empty());
            FirmwareCommand &next_cmd = _cmdQueue.head();
            FirmwareCommandType cmd_type = next_cmd.getType();

            if (cmd_type == FirmwareCommandType::REPORT_TEMPERATURES) {
                line = line.mid(3);
                parseTemperatureReport(line);
            }

            debug("+ FirmwareController::serialDataReady: command %s completed: %s",
                next_cmd.getCommandName(), next_cmd.getCommand().toUtf8().data());

            FirmwareComplexCommand &next_cplx_cmd = _cplxCmdQueue.head();
            FirmwareCommandType expected_command = next_cplx_cmd.popNextSubcommand();
            if (expected_command != next_cmd.getType()) {
                debug("+ FirmwareController::serialDataReady: command mismatch during %s - expected %d, got %d\n",
                    next_cplx_cmd.getCommandName(), expected_command, next_cmd.getType());
                resetFirmware();
                return;
            }

            if (next_cplx_cmd.isFinished()) {
                debug("+ FirmwareController::serialDataReady: complex command %s completed\n",
                    next_cplx_cmd.getCommandName());
                switch (next_cplx_cmd.getType()) {
                case (FirmwareComplexCommandType::MOVE_RELATIVE):
                    emit printerMoveRelativeCompleted(true);
                    break;
                case (FirmwareComplexCommandType::MOVE_ABSOLUTE):
                    emit printerMoveAbsoluteCompleted(true);
                    break;
                case (FirmwareComplexCommandType::MOVE_HOME):
                    emit printerHomeCompleted(true);
                    break;
                case (FirmwareComplexCommandType::INITIALIZE):
                    emit printerInitCompleted(true);
                    break;
                case (FirmwareComplexCommandType::SET_BED_TEMPERATURE):
                    emit printerSetTemperatureCompleted(true);
                    break;
                }

                emit printerComplexCommandCompleted(next_cplx_cmd.getType(), true);
                _cplxCmdQueue.dequeue();
            }

            emit printerCommandCompleted(next_cmd.getType(), true);
            _cmdQueue.dequeue();
#if defined _DEBUG
            if (g_settings.pretendPrinterIsOnline)
                return;
#endif // defined _DEBUG
            if (!_cmdQueue.isEmpty()) {
                const FirmwareCommand &next_cmd = _cmdQueue.head();
                _serial.write(next_cmd.getCommand().toUtf8());
                firmware_debug(next_cmd.getCommand().toUtf8().data());
            }

        } else if (line.contains("T:")) {
            line = line.mid(line.indexOf('T'));
            parseTemperatureReport(line);
        } else if (line.startsWith("X:")) {
            parsePosition(line);
        } else if (line.startsWith("Error")) {
            debug("+ FirmwareController::serialDataReady: firmware error: %s\n",
                line.toUtf8().data());
            resetFirmware();
            return;
        } else if (line.startsWith("echo:")) {
            debug("+ FirmwareController::serialDataReady: firmware debug print: %s\n",
                line.toUtf8().data());
        } else {
            QRegularExpressionMatch match = FirmwareVersionMatcher.match(line);

            if (match.hasMatch()) {
                QString firmwareVersion = match.captured(1);
                debug("+ FirmwareController::serialDataReady: firmware version: %s\n",
                    firmwareVersion);
                emit printerFirmwareVersionReport(firmwareVersion);
                return;
            }

            debug("+ FirmwareController::serialDataReady: unhandled firmware data output: %s\n",
                line.toUtf8().data());
        }
    }
}

void FirmwareController::parseTemperatureReport(QString const &line)
{
    QRegularExpressionMatch match = TemperatureReport1Matcher.match(line);

    if (match.hasMatch()) {
        double bed_temp = match.captured(3).toDouble();
        double bed_target_temp = match.captured(4).toDouble();
        int bed_pwm = match.captured(6).toInt();
        debug("+ FirmwareController::parseTemperatureReport: bed temperature: %.2f, target temperature: %.2f\n",
              bed_temp, bed_target_temp);
        emit printerTemperatureReport(bed_temp, bed_target_temp, bed_pwm);
    } else {
        QRegularExpressionMatch match2 = TemperatureReport2Matcher.match(line);

        if (match2.hasMatch()) {
            double bed_temp = match.captured(1).toDouble();
            double bed_target_temp = match.captured(2).toDouble();
            int bed_pwm = match.captured(3).toInt();
            debug("+ FirmwareController::parseTemperatureReport: bed temperature: %.2f, target temperature: %.2f\n",
                  bed_temp, bed_target_temp);
            emit printerTemperatureReport(bed_temp, bed_target_temp, bed_pwm);
        } else {
            debug("+ FirmwareController::parseTemperatureReport: temperature data missing in report: %s\n",
                line.toUtf8().data());
        }
    }
}

void FirmwareController::parsePosition(const QString &line)
{
    QRegularExpressionMatch match = PositionReportMatcher.match(line);

    if (match.hasMatch()) {
        double px = match.captured(1).toDouble();
        int cx = match.captured(5).toInt();
        emit printerPositionReport(px, cx);
        debug("+ FirmwareController::parsePosition: position px: %.2f, cx: %d\n",
              px, cx);
    } else
        debug("+ FirmwareController::parsePosition: position data missing in report: %s\n", line.toUtf8().data());
}

#if defined _DEBUG
void FirmwareController::mockTempAutoreport()
{
    _reportTemp = true;
    serialDataReady();
}
#endif // defined _DEBUG
