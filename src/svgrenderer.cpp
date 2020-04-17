#include "pch.h"

#include <sys/sysinfo.h>

#include "svgrenderer.h"

#include "processrunner.h"
#include "timinglogger.h"

namespace {

    QDomDocument _CloneDocRoot( QDomDocument& doc ) {
        QDomDocument skeletonDoc( doc.doctype( ) );
        skeletonDoc.appendChild( doc.childNodes( ).item( 0 ).cloneNode( false ) );
        skeletonDoc.appendChild( doc.childNodes( ).item( 1 ).cloneNode( false ) );
        return skeletonDoc;
    }

}

SvgRenderer::SvgRenderer( ) {
    /*empty*/
}

SvgRenderer::~SvgRenderer( ) {
    /*empty*/
}

void SvgRenderer::loadSlices( OrderManifestManager* manifestManager ) {
    int  layerNumber     { -1 };

    debug( "+ SvgRenderer::loadSlices \n");
    TimingLogger::startTiming( TimingId::LoadingPngFolder );
    OrderManifestManager::Iterator iter = manifestManager->iterator();
    while ( iter.hasNext() ) {
        debug( "+ SvgRenderer::loadSlices next iter \n");
        QString fileName = *iter;
        ++iter;

        debug( "+ SvgRenderer::loadSlices fileName: %s \n", fileName.toUtf8().data());
        layerNumber++;
        emit layerComplete( layerNumber );
    }
    TimingLogger::stopTiming( TimingId::LoadingPngFolder );

    debug( "  + Done\n" );
    emit layerCount( manifestManager->getSize() );
    emit done( true );
}

void SvgRenderer::startRender( QString const& svgFileName, QString const& outputDirectory, OrderManifestManager* manifestManager ) {
    debug( "+ SvgRenderer::startRender\n" );
    TimingLogger::startTiming( TimingId::RenderingPngs );

    _outputDirectory = outputDirectory;
    _layerList.clear();

    QFile file { svgFileName };
    if ( !file.open( QIODevice::ReadOnly ) ) {
        debug( "  + couldn't open file '%s'", svgFileName.toUtf8( ).data( ) );
        emit done( false );
        return;
    }
    if ( !_doc.setContent( &file ) ) {
        debug( "  + couldn't load file\n" );
        file.close( );
        emit done( false );
        return;
    }
    file.close( );

    QDomDocument skeletonDoc = _CloneDocRoot( _doc );

    QDomElement svgElement = _doc.documentElement( );
    if ( !svgElement.hasAttributes( ) ) {
        debug( "  + SVG element has no attributes?\n" );
        emit done( false );
        return;
    }

    auto mmWidth  = svgElement.attribute( "width" ).toDouble( );
    auto mmHeight = svgElement.attribute( "height" ).toDouble( );
    _pxWidth  = mmWidth  / ProjectorPixelSize + 0.5;
    _pxHeight = mmHeight / ProjectorPixelSize + 0.5;
    debug( "  + dimensions: mm %.2f×%.2f; px %d×%d\n", mmWidth, mmHeight, _pxWidth, _pxHeight );

    QDomNodeList childNodes = svgElement.childNodes( );
    int limit = childNodes.length( );
    debug( "  + childNodes.length(): %d\n", childNodes.length( ) );

    int layer = 0;
    for ( int childIndex = 0; childIndex < limit; ++childIndex ) {
        QDomNode node = childNodes.item( childIndex );
        if ( !node.isElement( ) ) {
            continue;
        }

        QDomElement* element = reinterpret_cast<QDomElement*>( &node );
        QString nodeName = element->nodeName( );
        if ( nodeName != "g" ) {
            continue;
        }
        if ( !element->hasAttributes( ) ) {
            continue;
        }

        QString idValue = element->attribute( "id" );
        if ( !idValue.startsWith( "layer" ) ) {
            continue;
        }

        QDomDocument layerDoc = _CloneDocRoot( skeletonDoc );
        auto docElt = layerDoc.documentElement( );
        docElt.appendChild( element->cloneNode( true ) );
        docElt.setAttribute( "width",  QString( "%1" ).arg( mmWidth,  0, 'f', 2 ) );
        docElt.setAttribute( "height", QString( "%1" ).arg( mmHeight, 0, 'f', 2 ) );

        QFile data( QString( "%1/%2.svg" ).arg( _outputDirectory ).arg( layer++, 6, 10, DigitZero ) );
        if ( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
            QTextStream textStream( &data );
            layerDoc.save( textStream, 0 );
            data.close( );
        } else {
            debug( "+ SvgRenderer::startRender: save failed\n" );
            emit done( false );
            return;
        }
    }

    manifestManager->restart();
    manifestManager->setPath( _outputDirectory );

    QStringList manifestFileList { };
    for( int i=0; i<layer; ++i) {
        manifestFileList.push_back( QString( "%1.png" ).arg( i, 6, 10, DigitZero ) );
    }

    manifestManager->setFileList( manifestFileList );
    manifestManager->save();

    _totalLayers = layer;
    _runningLayers .fill( -1,      get_nprocs( ) );
    _processRunners.fill( nullptr, get_nprocs( ) );
    emit layerCount( _totalLayers );

    _renderLayer( );
}

void SvgRenderer::_renderLayer( ) {
    debug(
        "+ SvgRenderer::_renderLayer:\n"
        "  + _currentLayer:     %d\n"
        "  + _totalLayers:      %d\n"
        "  + _runningProcesses: %d\n"
        "",
        _currentLayer,
        _totalLayers
    );

    for ( int slot = 0; ( slot < get_nprocs( ) ) && ( _currentLayer < _totalLayers ); ++slot ) {
        if ( _processRunners[slot] ) {
            debug( "  + slot:              %d [busy]\n", slot );
            continue;
        }
        debug(
            "  + slot:              %d [free]\n"
            "  + _totalLayers:      %d\n"
            "  + _completedLayers:  %d\n"
            "",
            slot,
            _totalLayers,
            _completedLayers
        );

        auto processRunner = new ProcessRunner;
        processRunner->setProcessChannelMode( QProcess::ForwardedChannels );

        QObject::connect( processRunner, &ProcessRunner::succeeded, [ this, slot ] ( ) {
            ++_completedLayers;
            --_processesRunning;
            debug(
                "+ SvgRenderer::_renderLayer: ProcessRunner::succeeded:\n"
                "  + slot:              %d\n"
                "  + layer:             %d\n"
                "  + _completedLayers:  %d\n"
                "  + _totalLayers:      %d\n"
                "",
                slot,
                _runningLayers[slot],
                _completedLayers,
                _totalLayers
            );

            emit layerComplete( _runningLayers[slot] );

            _cleanUpOneProcessRunner( slot );

            if ( _completedLayers == _totalLayers ) {
                TimingLogger::stopTiming( TimingId::RenderingPngs );
                debug( "+ SvgRenderer::_renderLayer: ProcessRunner::succeeded: finished\n" );
                emit done( true );
            } else {
                _renderLayer( );
            }
        } );

        QObject::connect( processRunner, &ProcessRunner::failed, [ this, slot ] ( int const exitCode, QProcess::ProcessError const error ) {
            debug(
                "+ SvgRenderer::_renderLayer: ProcessRunner::failed:\n"
                "  + slot:              %d\n"
                "  + layer:             %d\n"
                "  + exit code:         %d\n"
                "  + error:             %s [%d]\n"
                "",
                slot,
                _runningLayers[slot],
                exitCode,
                ToString( error ), static_cast<int>( error )
            );

            _processesRunning = 0;
            _cleanUpProcessRunners( );

            TimingLogger::stopTiming( TimingId::RenderingPngs );

            emit done( false );
        } );

        debug( "  + new instance in slot %d for layer %d: %p\n", slot, _currentLayer, processRunner );
        _processRunners[slot] = processRunner;
        _runningLayers[slot]  = _currentLayer;

        auto layer = _currentLayer++;
        ++_processesRunning;

        processRunner->start(
            { "gm" },
            {
                "convert",
                "-antialias",
                "-density",    "400",
                "-background", "#000000",
                "-size",       QString( "%1x%2" ).arg( _pxWidth ).arg( _pxHeight ),
                QString( "%1/%2.svg" ).arg( _outputDirectory ).arg( layer, 6, 10, DigitZero ),
                QString( "%1/%2.png" ).arg( _outputDirectory ).arg( layer, 6, 10, DigitZero )
            }
        );
    }
}

void SvgRenderer::_cleanUpOneProcessRunner( int const slot ) {
    auto processRunner = _processRunners[slot];
    if ( !processRunner ) {
        debug( "+ SvgRenderer::_cleanUpOneProcessRunner: slot %d: no instance, returning\n", slot );
        return;
    }

    debug( "+ SvgRenderer::_cleanUpOneProcessRunner: slot %d: layer %d, instance %p\n", slot, _runningLayers[slot], _processRunners[slot] );
    _processRunners[slot] = nullptr;
    _runningLayers[slot]  = -1;

    QObject::disconnect( processRunner );
    if ( processRunner->state( ) != QProcess::NotRunning ) {
        debug( "  + Process is in state %s, terminating\n", ToString( processRunner->state( ) ) );
        processRunner->terminate( );
    }
    processRunner->deleteLater( );
}

void SvgRenderer::_cleanUpProcessRunners( ) {
    debug( "+ SvgRenderer::_cleanUpProcessRunners\n" );

    for ( int slot = 0; slot < get_nprocs( ); ++slot ) {
        _cleanUpOneProcessRunner( slot );
    }
}
