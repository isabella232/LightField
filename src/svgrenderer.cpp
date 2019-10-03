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

    int const NumberOfCpus = get_nprocs( );

}

SvgRenderer::SvgRenderer( ) {
}

SvgRenderer::~SvgRenderer( ) {
    /*empty*/
}

void SvgRenderer::startRender( QString const& svgFileName, QString const& outputDirectory ) {
    TimingLogger::startTiming( TimingId::RenderingPngs );
    debug( "+ SvgRenderer::startRender\n" );
    _outputDirectory = outputDirectory;

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
    _pxWidth  = mmWidth  * 20.0 + 0.5;
    _pxHeight = mmHeight * 20.0 + 0.5;
    debug( "  + dimensions: mm %.2f×%.2f; px %d×%d\n", mmWidth, mmHeight, _pxWidth, _pxHeight );

    QDomNodeList childNodes = svgElement.childNodes( );
    int limit = childNodes.length( );
    debug( "  + childNodes.length(): %d\n", childNodes.length( ) );

    _currentLayer = 0;
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
        auto docElt = layerDoc.documentElement( );
        docElt.appendChild( element->cloneNode( true ) );
        docElt.setAttribute( "width",  QString( "%1" ).arg( mmWidth,  0, 'f', 2 ) );
        docElt.setAttribute( "height", QString( "%1" ).arg( mmHeight, 0, 'f', 2 ) );

        QFile data( QString( "%1/%2.svg" ).arg( _outputDirectory ).arg( _currentLayer++, 6, 10, DigitZero ) );
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

    _totalLayers = _currentLayer;
    _runningLayers .fill( -1,      NumberOfCpus );
    _processRunners.fill( nullptr, NumberOfCpus );
    emit layerCount( _totalLayers );

    _renderLayer( );
}

void SvgRenderer::_renderLayer( ) {
    debug( "+ SvgRenderer::_renderLayer:\n" );

    for ( int slot = 0; ( slot < NumberOfCpus ) && ( _currentLayer < _totalLayers ); ++slot ) {
        debug(
            "  + slot:              %d\n"
            "  + _currentLayer:     %d\n"
            "  + _completedLayers:  %d\n"
            "  + _totalLayers:      %d\n"
            "  + _processesRunning: %d\n"
            "",
            slot,
            _currentLayer,
            _completedLayers,
            _totalLayers,
            _processesRunning
        );
        if ( _processRunners[slot] ) {
            debug( "  + slot is busy, skipping\n" );
            continue;
        }

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
                "  + _processesRunning: %d\n"
                "",
                slot,
                _runningLayers[slot],
                _completedLayers,
                _totalLayers,
                _processesRunning
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
                "  + slot:     %d\n"
                "  + layer:     %d\n"
                "  + exit code: %d\n"
                "  + error:     %s [%d]\n"
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

        debug( "  + new instance in slot %d for layer %d: %p\n", slot, processRunner, _currentLayer );
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

    for ( int slot = 0; slot < NumberOfCpus; ++slot ) {
        _cleanUpOneProcessRunner( slot );
    }
}
