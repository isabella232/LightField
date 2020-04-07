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
QString            extern const  JobWorkingDirectoryPath;
QString            extern const  MountmonCommand;
QString            extern const  ResetLumenArduinoPortCommand;
QString            extern const  SetProjectorPowerCommand;
QString            extern const  ShepherdPath;
QString            extern const  SlicedSvgFileName;
QString            extern const  StlModelLibraryPath;
QString            extern const  UpdatesRootPath;
QString            extern const  PrintProfilesPath;

QChar              extern const  LineFeed;
QChar              extern const  CarriageReturn;
QChar              extern const  Space;
QChar              extern const  Comma;
QChar              extern const  Slash;
QChar              extern const  DigitZero;
QChar              extern const  EmDash;
QChar              extern const  Bullet;

QChar              extern const  FA_Check;
QChar              extern const  FA_Times;
QChar              extern const  FA_FastBackward;
QChar              extern const  FA_Backward;
QChar              extern const  FA_Forward;
QChar              extern const  FA_FastForward;
int                extern const  KeyboardRepeatDelay;

int                       const  DebugLogPathCount          =    6;
char               extern const* DebugLogPaths[DebugLogPathCount];

double constexpr          const  PrinterMaximumX            =   64.00; // mm
double constexpr          const  PrinterMaximumY            =   40.00; // mm
double constexpr          const  PrinterMaximumZ            =   50.00; // mm
double constexpr          const  PrinterRaiseToMaximumZ     =   60.00; // mm
double constexpr          const  PrinterHighSpeedThresholdZ =   10.00; // mm

double constexpr          const  PrinterDefaultHighSpeed    =  200.00; // mm/min
double constexpr          const  PrinterDefaultLowSpeed     =   50.00; // mm/min

double constexpr          const  AspectRatio5to3            =    5.0 /  3.0;
double constexpr          const  AspectRatio16to10          =   16.0 / 10.0;

double constexpr          const  LargeFontSize              =   22.0; // pt
double constexpr          const  NormalFontSize             =   12.0; // pt



#   if defined DLP4710

double constexpr          const  ProjectorMaxPowerLevel     = 1023.0;

int    constexpr          const  ProjectorMinPercent        =    5;
int    constexpr          const  ProjectorMaxPercent        =   80;

#   else

double constexpr          const  ProjectorMaxPowerLevel     =  255.0;

int    constexpr          const  ProjectorMinPercent        =   20;
int    constexpr          const  ProjectorMaxPercent        =   80;

#   endif

#endif // __CONSTANTS_H__
