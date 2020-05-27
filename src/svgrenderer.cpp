#include "pch.h"

#include <sys/sysinfo.h>
#include <QtCore>
#include <QtXml>

#include "svgrenderer.h"
#include "timinglogger.h"
#include "ordermanifestmanager.h"


namespace
{
    QDomDocument _CloneDocRoot(QDomDocument& doc)
    {
        QDomDocument skeletonDoc(doc.doctype());
        skeletonDoc.appendChild(doc.childNodes().item(0).cloneNode(false));
        skeletonDoc.appendChild(doc.childNodes().item(1).cloneNode(false));
        return skeletonDoc;
    }

}

void SvgRenderer::render(const QString& svgFileName, const QString& outputDirectory,
    QSharedPointer<PrintJob>printJob, QSharedPointer<OrderManifestManager> orderManager)
{
    Q_ASSERT(svgFileName.length() > 0);
    Q_ASSERT(outputDirectory.length() > 0);
    Q_ASSERT(printJob != nullptr);

    debug("+ SvgRenderer::render\n");
    debug("  + svgFileName: %s\n", svgFileName.toUtf8().data());
    debug("  + outputDirectory: %s\n", outputDirectory.toUtf8().data());

    TimingLogger::startTiming( TimingId::RenderingPngs );

    _threadPool.setMaxThreadCount(1);
    _outputDirectory = outputDirectory;

    QFile file { svgFileName };
    if ( !file.open( QIODevice::ReadOnly ) ) {
        debug( "  + couldn't open file '%s'\n", svgFileName.toUtf8( ).data( ) );
        throw std::runtime_error("Couldn't open file");
    }
    if ( !_doc.setContent( &file ) ) {
        debug( "  + couldn't load file\n" );
        file.close( );
        throw std::runtime_error("Couldn't load file");
    }
    file.close( );

    QDomDocument skeletonDoc = _CloneDocRoot( _doc );

    QDomElement svgElement = _doc.documentElement( );
    if ( !svgElement.hasAttributes( ) ) {
        debug( "  + SVG element has no attributes?\n" );
        throw std::runtime_error("SVG element has no attributes");
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
        QString nodeName = element->nodeName();
        if (nodeName != "g")
            continue;

        if (!element->hasAttributes())
            continue;

        QString idValue = element->attribute( "id" );
        if ( !idValue.startsWith( "layer" ) ) {
            continue;
        }

        QString layerFilename { QString( "%1/%2.svg" )
            .arg(_outputDirectory)
            .arg(layer, 6, 10, DigitZero) };

        if (!orderManager.isNull())
            orderManager->addFile(QString("%1.png").arg(layer, 6, 10, DigitZero));

        QDomDocument layerDoc = _CloneDocRoot( skeletonDoc );
        auto docElt = layerDoc.documentElement( );
        docElt.appendChild( element->cloneNode( true ) );
        docElt.setAttribute( "width",  QString( "%1" ).arg( mmWidth,  0, 'f', 2 ) );
        docElt.setAttribute( "height", QString( "%1" ).arg( mmHeight, 0, 'f', 2 ) );

        QFile data( layerFilename );
        if ( data.open( QFile::WriteOnly | QFile::Truncate ) ) {
            QTextStream textStream( &data );
            layerDoc.save( textStream, 0 );
            data.close( );
        } else {
            debug( "+ SvgRenderer::startRender: save failed\n" );
            throw std::runtime_error("SVG save failed");
        }

        layer++;
    }

    orderManager->setPath(_outputDirectory);

    _totalLayers = layer;
    emit layerCount( _totalLayers );

    for (int i = 0; i < _totalLayers; i++) {
        auto in { QString("%1/%2.svg").arg(_outputDirectory).arg(i, 6, 10, DigitZero) };
        auto out { QString("%1/%2.png").arg(_outputDirectory).arg( i, 6, 10, DigitZero) };
        LayerRenderTask *task { new LayerRenderTask { *this, i, in, out } };
        _threadPool.start(task);
    }

    _threadPool.waitForDone();
    orderManager->save();
}
