#include "pch.h"

#include "buildinfo.h"

QString g_BuildTimestamp;

namespace {

    class ReformatBuildTimestamp {

    public:

        ReformatBuildTimestamp( ) {
            QMap<QString, QString> const monthMap {
                { "Jan", "01" },
                { "Feb", "02" },
                { "Mar", "03" },
                { "Apr", "04" },
                { "May", "05" },
                { "Jun", "06" },
                { "Jul", "07" },
                { "Aug", "08" },
                { "Sep", "09" },
                { "Oct", "10" },
                { "Nov", "11" },
                { "Dec", "12" },
            };

            QString const date { __DATE__ }; // looks like "Mmm dd yyyy"

            g_BuildTimestamp =
                date.right( 4 )                              % '-' % // year
                monthMap[date.left( 3 )]                     % '-' % // month
                ( date[4] == ' ' ? '0' : date[4] ) % date[5] % 'T' % // day
                __TIME__                                             // hour, minute, and second
                ;
        }

    };

    ReformatBuildTimestamp _buildTimestampReformatter;

}
