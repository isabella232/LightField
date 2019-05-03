#include "pch.h"
#include "constants.h"

QSize          const MainWindowSize           { 1024, 600 };
QSize          const PngDisplayWindowSize     { 1280, 800 };
QSize          const MainButtonSize           {  279,  93 };
QSize          const MaximalRightHandPaneSize {  722, 530 };

QString        const MediaRootPath            { "/media"                            };
QString        const StlModelLibraryPath      { "/var/lib/lightfield/model-library" };
QString        const JobWorkingDirectoryPath  { "/var/cache/lightfield/print-jobs"  };
QString        const SlicedSvgFileName        { "sliced.svg"                        };
QString        const SetpowerCommand          { "setpower"                          };

QChar          const Space                    { L' '      };
QChar          const Slash                    { L'/'      };
QChar          const DigitZero                { L'0'      };
QChar          const FigureSpace              { L'\u2007' };
QChar          const EmDash                   { L'\u2014' };
QChar          const BlackDiamond             { L'\u25C6' };

QChar          const FA_Check                 { L'\uF00C' };
QChar          const FA_Times                 { L'\uF00D' };
QChar          const FA_FastBackward          { L'\uF049' };
QChar          const FA_Backward              { L'\uF04A' };
QChar          const FA_Forward               { L'\uF04E' };
QChar          const FA_FastForward           { L'\uF050' };
