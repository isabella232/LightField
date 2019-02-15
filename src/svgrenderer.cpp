#include "pch.h"

#include "svgrenderer.h"

#include "processrunner.h"
#include "strings.h"

namespace {

    QDomDocument _CloneDocRoot( QDomDocument& doc ) {
        QDomDocument skeletonDoc( doc.doctype( ) );
        skeletonDoc.appendChild( doc.childNodes( ).item( 0 ).cloneNode( false ) );
        skeletonDoc.appendChild( doc.childNodes( ).item( 1 ).cloneNode( false ) );
        return skeletonDoc;
    }

}

SvgRenderer::SvgRenderer( ) {
    pr = new ProcessRunner;
    QObject::connect( pr, &ProcessRunner::succeeded, this, &SvgRenderer::programSucceeded );
    QObject::connect( pr, &ProcessRunner::failed,    this, &SvgRenderer::programFailed    );
}

SvgRenderer::~SvgRenderer( ) {
    /*empty*/
}

void SvgRenderer::startRender( QString const& svgFileName, QString const& outputDirectory_ ) {
    debug( "+ SvgRenderer::startRender\n" );
    outputDirectory = outputDirectory_;

    QFile file { svgFileName };
    if ( !file.open( QIODevice::ReadOnly ) ) {
        debug( "  + couldn't open file '%s'", svgFileName.toUtf8( ).data( ) );
        emit done( -1 );
        return;
    }
    if ( !doc.setContent( &file ) ) {
        debug( "  + couldn't load file\n" );
        file.close( );
        emit done( -1 );
        return;
    }
    file.close( );

    QDomDocument skeletonDoc = _CloneDocRoot( doc );

    QDomElement svgElement = doc.documentElement( );
    if ( !svgElement.hasAttributes( ) ) {
        debug( "  + SVG element has no attributes?\n" );
        emit done( -1 );
        return;
    }
    pxWidth  = svgElement.attribute( "width"  ).toDouble( ) * 20 + 0.5;
    pxHeight = svgElement.attribute( "height" ).toDouble( ) * 20 + 0.5;

    QDomNodeList childNodes = svgElement.childNodes( );
    int limit = childNodes.length( );
    digits = ceil( log10( static_cast<double>( limit ) ) );
    debug( "  + childNodes.length(): %d\n", childNodes.length( ) );

    currentLayer = 0;
    for ( int childIndex = 0; childIndex < limit; ++childIndex ) {
        QDomNode node = childNodes.item( childIndex );
        if ( !node.isElement( ) ) {
            debug( "  + skipping non-element node\n" );
            continue;
        }

        QDomElement* element = reinterpret_cast<QDomElement*>( &node );
        QString nodeName = element->nodeName( );
        if ( nodeName != "g" ) {
            debug( "  + skipping element because it's not <g>\n" );
            continue;
        }
        if ( !element->hasAttributes( ) ) {
            debug( "  + skipping element because it has no attributes\n" );
            continue;
        }

        QString idValue = element->attribute( "id" );
        if ( !idValue.startsWith( "layer" ) ) {
            debug( "  + skipping element because its id attribute does not start with 'layer'\n" );
            continue;
        }

        QDomDocument layerDoc = _CloneDocRoot( skeletonDoc );
        layerDoc.documentElement( ).appendChild( element->cloneNode( true ) );

        QFile data( QString( "%1/%2.svg" ).arg( outputDirectory ).arg( currentLayer++, 6, 10, QChar( '0' ) ) );
        if ( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
            QTextStream textStream( &data );
            layerDoc.save( textStream, 0 );
            data.close( );
        } else {
            debug( "+ SvgRenderer::startRender: save failed\n" );
            emit done( -1 );
            return;
        }
    }
    layerCount = currentLayer;

    currentLayer = 0;
    renderLayer( );
}

void SvgRenderer::renderLayer( ) {
    debug( "+ SvgRenderer::renderLayer: currentLayer: %d\n", currentLayer );
    emit nextLayer( currentLayer );
    pr->start(
        QString     { "gm" },
        QStringList {
            QString( "convert"     ),
            QString( "-antialias"  ),
            QString( "-density"    ), QString( "400" ),
            QString( "-size"       ), QString( "%1x%2" ).arg( pxWidth ).arg( pxHeight ),
            QString( "-background" ), QString( "#000000" ),
            QString( "%1/%2.svg"   ).arg( outputDirectory ).arg( currentLayer, 6, 10, QChar( '0' ) ),
            QString( "%1/%2.png"   ).arg( outputDirectory ).arg( currentLayer, 6, 10, QChar( '0' ) )
        }
    );
}

void SvgRenderer::programSucceeded( ) {
    ++currentLayer;
    if ( currentLayer == layerCount ) {
        debug( "+ SvgRenderer::programSucceeded: finished\n" );
        emit done( layerCount );
    } else {
        renderLayer( );
    }
}

void SvgRenderer::programFailed( QProcess::ProcessError const error ) {
    debug( "+ SvgRenderer::programFailed: error: %s [%d]\n", ToString( error ), static_cast<int>( error ) );
    emit done( -1 );
}
