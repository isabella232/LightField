#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

QRegularExpression extern const EndsWithWhitespaceRegex;
QRegularExpression extern const NewLineRegex;

QSize              extern const MainButtonSize;
QSize              extern const MainWindowSize;
QSize              extern const MaximalRightHandPaneSize;
QSize              extern const PngDisplayWindowSize;
QSize              extern const QuarterRightHandPaneSize;

QString            extern const MediaRootPath;
QString            extern const StlModelLibraryPath;
QString            extern const JobWorkingDirectoryPath;
QString            extern const SlicedSvgFileName;
QString            extern const SetpowerCommand;
QString            extern const GpgKeyRingPath;
QString            extern const ShepherdPath;
QString            extern const UpdatesRootPath;

QChar              extern const LineFeed;
QChar              extern const CarriageReturn;
QChar              extern const Space;
QChar              extern const Slash;
QChar              extern const DigitZero;
QChar              extern const FigureSpace;
QChar              extern const EmDash;
QChar              extern const BlackDiamond;

QChar              extern const FA_Check;
QChar              extern const FA_Times;
QChar              extern const FA_FastBackward;
QChar              extern const FA_Backward;
QChar              extern const FA_Forward;
QChar              extern const FA_FastForward;

double                    const PrinterMaximumX          = 64.00;
double                    const PrinterMaximumY          = 40.00;
double                    const PrinterMaximumZ          = 50.00;
double                    const PrinterRaiseToMaxZHeight = 70.00;

double                    const AspectRatio5to3          =  5.0 /  3.0;
double                    const AspectRatio16to10        = 16.0 / 10.0;

#endif // __CONSTANTS_H__
