#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#include <QtCore>

QRegularExpression extern const  EndsWithWhitespaceRegex;
QRegularExpression extern const  SliceDirectoryNameRegex;
QRegularExpression extern const  TiledDirectoryNameRegex;
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
QString            extern const  ManifestFilename;

QChar              extern const  LineFeed;
QChar              extern const  CarriageReturn;
QChar              extern const  Space;
QChar              extern const  Comma;
QChar              extern const  HyphenMinus;
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
int                extern const  KeyboardRepeatDelayStart;
int                extern const  KeyboardRepeatDelay;

int                extern const  DefaultBodyLayerThickness;
int                extern const  DefaultBaseLayerThickness;

int    constexpr          const  DebugLogPathCount          =    6;
char               extern const* DebugLogPaths[DebugLogPathCount];
char               extern const* DebugLogPath;
char               extern const* TestLogPath;

#   if defined XDLP471020UM

double constexpr          const  ProjectorPixelSize         =    0.0200; // mm

#   elif defined DLP4710

double constexpr          const  ProjectorPixelSize         =    0.0355; // mm

#   else // ! defined DLP4710

double constexpr          const  ProjectorPixelSize         =    0.0500; // mm

#   endif // defined DLP4710

double             extern const  PrinterMaximumX;
double             extern const  PrinterMaximumY;
double constexpr          const  PrinterMaximumZ            =   60.00;   // mm
double constexpr          const  PrinterRaiseToMaximumZ     =   60.00;   // mm
double constexpr          const  PrinterHighSpeedThresholdZ =   10.00;   // mm
int    constexpr          const  TilingMargin               =   25;      // px

double constexpr          const  PrinterDefaultHighSpeed    =  200.00;   // mm/min
double constexpr          const  PrinterDefaultLowSpeed     =   50.00;   // mm/min

double constexpr          const  LargeFontSize              =   22.0;   // pt
double constexpr          const  NormalFontSize             =   12.0;   // pt

double constexpr          const  AspectRatio5to3            =    5.0 /  3.0;
double constexpr          const  AspectRatio16to10          =   16.0 / 10.0;



#   if defined DLP4710 || XDLP471020UM

double constexpr          const  ProjectorMaxPowerLevel     = 1023.0;

int    constexpr          const  ProjectorMinPercent        =   5;
int    constexpr          const  ProjectorMaxPercent        =   100;

#   else // ! defined DLP4710

double constexpr          const  ProjectorMaxPowerLevel     =  255.0;

int    constexpr          const  ProjectorMinPercent        =   20;
int    constexpr          const  ProjectorMaxPercent        =   80;

#   endif // defined DLP4710

#endif // __CONSTANTS_H__
