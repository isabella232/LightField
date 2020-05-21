
#include "tilingmanager.h"


TilingManager::TilingManager( PrintJob* printJob )
{
    _printJob = printJob;
}

void TilingManager::processImages(
        int width,
        int height,
        double baseExpoTime,
        double baseStep,
        double bodyExpoTime,
        double bodyStep,
        int space,
        int count )
{
    debug( "+ TilingManager::processImages\n");

    _width = width;
    _height = height;
    _baseExpoTime = baseExpoTime;
    _baseStep = baseStep;
    _bodyExpoTime = bodyExpoTime;
    _bodyStep = bodyStep;
    _space = space;
    _count = count;
    _wCount = count;
    _spacePx = static_cast<int>(static_cast<double>(space) / ProjectorPixelSize);

    QString dirName = QString("tiled-%1-%2-%3-%4-%5-%6-").arg( _baseExpoTime ).arg( _baseStep )
            .arg( _bodyExpoTime ).arg( _bodyStep ).arg( _count ).arg( space ) % _printJob->modelHash;
    _path = JobWorkingDirectoryPath % Slash % dirName;

    QDir dir ( JobWorkingDirectoryPath );
    dir.mkdir( dirName );

    tileImages( );

    QFile::link( _path , StlModelLibraryPath % Slash % dirName );

    OrderManifestManager manifestMgr;

    manifestMgr.setFileList( _fileNameList );
    manifestMgr.setExpoTimeList( _expoTimeList );
    manifestMgr.setBaseLayerThickness( _printJob->baseSlices.layerThickness );
    manifestMgr.setBodyLayerThickness( _printJob->bodySlices.layerThickness );
    manifestMgr.setBaseLayerCount( _printJob->baseSlices.layerCount );
    manifestMgr.setPath( JobWorkingDirectoryPath % Slash % dirName );

    manifestMgr.setTiled( true );
    manifestMgr.setTilingSpace( _space );
    manifestMgr.setTilingCount( _count );
    manifestMgr.setVolume( _count * _printJob->estimatedVolume );

    manifestMgr.save();
}

void TilingManager::tileImages ( )
{
    debug( "+ TilingManager::tileImages\n");

    QPixmap pixmap;
    pixmap.load(_printJob->getLayerDirectory(0) % Slash % _printJob->getLayerFileName(0) );

    //For now only 1 row
    //_hCount =  floor( _height / (pixmap.height() + pixmap.height() * _space ) );
    _hCount = 1;

    debug( "+ TilingManager::renderTiles _width %d, _height %d, pixmap.width %d, pixmap.height %d, _space %f \n",
                                         _width,    _height,    pixmap.width(),  pixmap.height(),  _space);

    _counter = 0;

    int deltax = ( ProjectorWindowSize.width() - ( _wCount*pixmap.width() ) - ( _wCount -1 ) * ( _space / ProjectorPixelSize ) ) / 2 - TilingMargin;

    for(int i=0; i<_wCount; ++i) {
        int x=TilingMargin + ( pixmap.width() * i ) + ( ( _space / ProjectorPixelSize ) * i );

        _tileSlots.push_back(x + deltax);
    }

    std::rotate(_tileSlots.begin(),
                _tileSlots.end()-1, // this will be the new first element
                _tileSlots.end());



    /* iterating over slices in manifest */
    int total = _printJob->totalLayerCount();

    for (int i = 0; i < total; i++) {
        QFileInfo entry ( _printJob->getLayerDirectory(i) % Slash % _printJob->getLayerFileName(i) );

        /* render tiles based on slice */
        emit statusUpdate(QString("Tiling layer %1").arg(i));
        emit progressUpdate((double)i / (double)total * 100);
        renderTiles ( entry, i );
    }

}

void TilingManager::renderTiles ( QFileInfo info, int sequence ) {
    int overalCount = _wCount * _hCount; // overal count of tiles

    /* interating over each exposure time */
    for ( int e = 1; e <= overalCount; ++e)
    {
        /* pixmap of single tile */
        QPixmap pixmap ( _width, _height );
        QPainter painter ( &pixmap );
        painter.fillRect(0,0, _width, _height, QBrush("#000000"));

        int innerCount=0;
        /* iterating over rows and columns */
        for( int r=0; (r<_wCount) && (innerCount<e); ++r )
        {
            for( int c=0; (c<_hCount) && (innerCount<e); ++c )
            {
                debug( "+ TilingManager::renderTiles overalCount %d, e %d, innerCount %d, r %d, c %d \n",
                                                    overalCount,    e,    innerCount,    r,    c);
                QPixmap sprite;

                debug( "+ TilingManager::renderTiles path %s\n", info.filePath().toUtf8().data() );
                sprite.load( info.filePath() );
                putImageAt ( sprite, &painter, r, c );

                ++innerCount;
            }
        }

        QString filename = QString( "%1/%2.png" ).arg( _path ).arg( _counter, 6, 10, DigitZero );
        debug( "+ TilingManager::tileImages save %s\n", filename.toUtf8().data());

        QFile file ( filename );
        file.open(QIODevice::WriteOnly);
        pixmap.save( &file, "PNG" );

        if( sequence < _printJob->baseSlices.layerCount ) {
            _expoTimeList.push_back( e == 1 ? _baseExpoTime : _baseStep );
        } else {
            _expoTimeList.push_back( e == 1 ? _bodyExpoTime : _bodyStep );
        }

        _fileNameList.push_back( GetFileBaseName( filename ) );

        _counter++;
    }
}

void TilingManager::putImageAt ( QPixmap pixmap, QPainter* painter, int i, int j ) {

    int x = _tileSlots[i];
    // For now only 1 row
    // int y = ( pixmap.height( ) * _space) + ( pixmap.height( ) * j )  + ( pixmap.height( ) * _space * j );

    int y = ( ProjectorWindowSize.height() - pixmap.height() ) / 2;

    if(i == 0)
        y -= ( 3 / ProjectorPixelSize );

    debug( "+ TilingManager::renderTiles x %d, y %d \n", x, y);

    painter->drawPixmap( x, y, pixmap );
}
