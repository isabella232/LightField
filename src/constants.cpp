#include "pch.h"
#include "constants.h"

QSize          const MainButtonSize           {  218,  74 };
QSize          const MainWindowSize           {  800, 480 };
QSize          const MaximalRightHandPaneSize {  564, 424 };
QSize          const PngDisplayWindowSize     { 1280, 800 };
QSize          const QuarterRightHandPaneSize {  257, 187 };

QString        const MediaRootPath            { "/media"                                           };
QString        const StlModelLibraryPath      { "/var/lib/lightfield/model-library"                };
QString        const JobWorkingDirectoryPath  { "/var/cache/lightfield/print-jobs"                 };
QString        const SlicedSvgFileName        { "sliced.svg"                                       };
QString        const SetpowerCommand          { "/usr/lib/lightfield/setpower"                     };
QString        const GpgKeyRingPath           { "/code/work/Volumetric/LightField/gpg/pubring.kbx" };
QString        const ShepherdPath             { "/usr/lib/lightfield/stdio-shepherd"               };
QString        const UpdatesRootPath          { "/var/cache/lightfield/software-updates"           };

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
