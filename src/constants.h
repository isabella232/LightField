#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

QRegularExpression extern const  EndsWithWhitespaceRegex;
QRegularExpression extern const  NewLineRegex;

QSize              extern const  SmallMainWindowSize;
QSize              extern const  SmallMainButtonSize;
QSize              extern const  SmallMaximalRightHandPaneSize;

QSize              extern        MainWindowSize;
QSize              extern        MainButtonSize;
QSize              extern        MaximalRightHandPaneSize;

QSize              extern const  ButtonPadding;
QSize              extern const  ProjectorWindowSize;
                                                             
QString            extern const  AptSourcesFilePath;
QString            extern const  GpgKeyRingPath;
QString            extern const  JobWorkingDirectoryPath;
QString            extern const  MountmonCommand;
QString            extern const  ResetLumenArduinoPortCommand;
QString            extern const  SetProjectorPowerCommand;
QString            extern const  ShepherdPath;
QString            extern const  SlicedSvgFileName;
QString            extern const  StlModelLibraryPath;
QString            extern const  UpdatesRootPath;

QChar              extern const  LineFeed;
QChar              extern const  CarriageReturn;
QChar              extern const  Space;
QChar              extern const  Slash;
QChar              extern const  DigitZero;
QChar              extern const  EmDash;
QChar              extern const  Bullet;
QChar              extern const  BlackDiamond;

QChar              extern const  FA_Check;
QChar              extern const  FA_Times;
QChar              extern const  FA_FastBackward;
QChar              extern const  FA_Backward;
QChar              extern const  FA_Forward;
QChar              extern const  FA_FastForward;

int                       const  DebugLogPathCount        =   6;
char               extern const* DebugLogPaths[DebugLogPathCount];

double                    const  PrinterMaximumX          =  64.00; // mm
double                    const  PrinterMaximumY          =  40.00; // mm
double                    const  PrinterMaximumZ          =  50.00; // mm
double                    const  PrinterRaiseToMaximumZ   =  60.00; // mm
double                    const  PrinterHighSpeedLowerToZ =  10.00; // mm

double                    const  PrinterDefaultHighSpeed  = 200.00; // mm/min
double                    const  PrinterDefaultLowSpeed   =  50.00; // mm/min

double                    const  AspectRatio5to3          =   5.0 /  3.0;
double                    const  AspectRatio16to10        =  16.0 / 10.0;

double                    const  LargeFontSize            =  22.0; // pt
double                    const  NormalFontSize           =  12.0; // pt

#endif // __CONSTANTS_H__
