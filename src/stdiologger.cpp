#include "pch.h"

#include "stdiologger.h"

StdioLogger::StdioLogger( char const* name, QObject* parent ): QObject { parent } {
    auto prefix { QString { "[%1] " }.arg( name ) };
    _lfPrefix = LineFeed % prefix;
    _prefix   = strdup( prefix.toUtf8( ).data( ) );
}

StdioLogger::~StdioLogger( ) {
    if ( _prefix ) {
        free( _prefix );
    }
}

void StdioLogger::clear( ) {
    _buffer.clear( );
}

void StdioLogger::flush( ) {
    if ( _buffer.isEmpty( ) ) {
        return;
    }

    if ( _buffer.endsWith( LineFeed ) ) {
        _buffer.chop( 1 );
    }

    _buffer.replace( NewLineRegex, _lfPrefix );
    debug( "%s%s\n", _prefix, _buffer.toUtf8( ).data( ) );

    clear( );
}

void StdioLogger::read( QString const& data ) {
    _buffer += data;
    if ( auto index { _buffer.lastIndexOf( LineFeed ) }; index > -1 ) {
        auto chunk { _buffer.left( index ) };

        chunk.replace( NewLineRegex, _lfPrefix );
        debug( "%s%s\n", _prefix, chunk.toUtf8( ).data( ) );

        _buffer.remove( 0, index + 1 );
    }
}
