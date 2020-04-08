#include "pch.h"
#include "constants.h"

QRegularExpression        const  EndsWithWhitespaceRegex       { "\\s+$"      };
QRegularExpression        const  SliceDirectoryNameRegex       { "-([0-9]+)$" };
QRegularExpression        const  NewLineRegex                  { "\\r?\\n"    };

QSize                     const  SmallMainWindowSize           {    800,  480 };
QSize                     const  SmallMainButtonSize           {    218,   74 };
QSize                     const  SmallMaximalRightHandPaneSize {    564,  424 };

QSize                            MainWindowSize                {   1024,  600 };
QSize                            MainButtonSize                {    279,   93 };
QSize                            MaximalRightHandPaneSize      {    722,  530 };

QSize                     const  ButtonPadding                 {     20,    4 };

#if defined DLP4710

QSize constexpr           const  ProjectorWindowSize           {   1920, 1080 };

#else // ! defined DLP4710

QSize constexpr           const  ProjectorWindowSize           {   1280,  800 };

#endif // defined DLP4710

QString                   const  AptSourcesFilePath            { "/etc/apt/sources.list.d/volumetric-lightfield.list"     };
QString                   const  JobWorkingDirectoryPath       { "/var/cache/lightfield/print-jobs"                       };
QString                   const  MountmonCommand               { "mountmon"                                               };
QString                   const  ResetLumenArduinoPortCommand  { "/usr/share/lightfield/libexec/reset-lumen-arduino-port" };
QString                   const  SetProjectorPowerCommand      { "set-projector-power"                                    };
QString                   const  ShepherdPath                  { "/usr/share/lightfield/libexec/stdio-shepherd"           };
QString                   const  SlicedSvgFileName             { "sliced.svg"                                             };
QString                   const  StlModelLibraryPath           { "/var/lib/lightfield/model-library"                      };
QString                   const  UpdatesRootPath               { "/var/lib/lightfield/software-updates"                   };
QString                   const  ManifestFilename              { "manifest.json"                                          };

//QString                   const  PrintProfilesPath             { "/var/lib/lightfield/print-profiles.json"                };
QString                   const  PrintProfilesPath             { "/home/lumen/Volumetric/LightField/print-profiles/print-profiles.json"};

//QString                   const  PrintProfilesSchemaPath       { "/var/lib/lightfield/print-profiles-schema.json"         };

QChar                     const  LineFeed                      { L'\u000A' };
QChar                     const  CarriageReturn                { L'\u000D' };
QChar                     const  Space                         { L'\u0020' };
QChar                     const  Comma                         { L'\u002C' };
QChar                     const  Slash                         { L'\u002F' };
QChar                     const  DigitZero                     { L'\u0030' };
QChar                     const  EmDash                        { L'\u2014' };
QChar                     const  Bullet                        { L'\u2022' };

QChar                     const  FA_Check                      { L'\uF00C' };
QChar                     const  FA_Times                      { L'\uF00D' };
QChar                     const  FA_FastBackward               { L'\uF049' };
QChar                     const  FA_Backward                   { L'\uF04A' };
QChar                     const  FA_Forward                    { L'\uF04E' };
QChar                     const  FA_FastForward                { L'\uF050' };

double                    const  PrinterMaximumX               { ProjectorWindowSize.width( )  * ProjectorPixelSize }; // mm
double                    const  PrinterMaximumY               { ProjectorWindowSize.height( ) * ProjectorPixelSize }; // mm

char                      const* DebugLogPaths[DebugLogPathCount]
{
    "/var/log/lightfield/debug.log",
    "/var/log/lightfield/debug.1.log",
    "/var/log/lightfield/debug.2.log",
    "/var/log/lightfield/debug.3.log",
    "/var/log/lightfield/debug.4.log",
    "/var/log/lightfield/debug.5.log",
};
