#include "pch.h"
#include "constants.h"

QRegularExpression        const  EndsWithWhitespaceRegex       { "\\s+$"   };
QRegularExpression        const  NewLineRegex                  { "\\r?\\n" };
                                                             
QSize                     const  SmallMainWindowSize           {  800, 480 };
QSize                     const  SmallMainButtonSize           {  218,  74 };
QSize                     const  SmallMaximalRightHandPaneSize {  564, 424 };

QSize                            MainWindowSize                { 1024, 600 };
QSize                            MainButtonSize                {  279,  93 };
QSize                            MaximalRightHandPaneSize      {  722, 530 };

QSize                     const  ButtonPadding                 {   20,   4 };
QSize                     const  ProjectorWindowSize           { 1280, 800 };
                                                             
QString                   const  AptSourcesFilePath            { "/etc/apt/sources.list.d/volumetric-lightfield.list"     };
QString                   const  GpgKeyRingPath                { "/home/lumen/Volumetric/LightField/gpg/pubring.gpg"      };
QString                   const  JobWorkingDirectoryPath       { "/var/cache/lightfield/print-jobs"                       };
QString                   const  MountmonCommand               { "mountmon"                                               };
QString                   const  ResetLumenArduinoPortCommand  { "/usr/share/lightfield/libexec/reset-lumen-arduino-port" };
QString                   const  SetProjectorPowerCommand      { "set-projector-power"                                    };
QString                   const  ShepherdPath                  { "/usr/share/lightfield/libexec/stdio-shepherd"           };
QString                   const  SlicedSvgFileName             { "sliced.svg"                                             };
QString                   const  StlModelLibraryPath           { "/var/lib/lightfield/model-library"                      };
QString                   const  UpdatesRootPath               { "/var/lib/lightfield/software-updates"                   };
                                                             
QChar                     const  LineFeed                      { L'\u000A' };
QChar                     const  CarriageReturn                { L'\u000D' };
QChar                     const  Space                         { L'\u0020' };
QChar                     const  FullStop                      { L'\u002E' };
QChar                     const  Slash                         { L'\u002F' };
QChar                     const  DigitZero                     { L'\u0030' };
QChar                     const  FigureSpace                   { L'\u2007' };
QChar                     const  EmDash                        { L'\u2014' };
QChar                     const  BlackDiamond                  { L'\u25C6' };
                                                             
QChar                     const  FA_Check                      { L'\uF00C' };
QChar                     const  FA_Times                      { L'\uF00D' };
QChar                     const  FA_FastBackward               { L'\uF049' };
QChar                     const  FA_Backward                   { L'\uF04A' };
QChar                     const  FA_Forward                    { L'\uF04E' };
QChar                     const  FA_FastForward                { L'\uF050' };

char                      const* DebugLogPaths[DebugLogPathCount]
{
    "/var/log/lightfield/debug.log",
    "/var/log/lightfield/debug.1.log",
    "/var/log/lightfield/debug.2.log",
    "/var/log/lightfield/debug.3.log",
    "/var/log/lightfield/debug.4.log",
    "/var/log/lightfield/debug.5.log",
};
