
#include "tilingmanager.h"


TilingManager::TilingManager( OrderManifestManager* manifestMgr, PrintJob* printJob )
{
    _manifestMgr = manifestMgr;
    _printJob = printJob;
}

void TilingManager::processImages( int width, int height, double expoTime, double step, int space, int count )
{
    debug( "+ TilingManager::processImages\n");

    _width = width;
    _height = height;
    _expoTime = expoTime;
    _step = step;
    _space = space;
    _count = count;
    _wCount = count;
    _spacePx = ((double)space) / ProjectorPixelSize;

    QString jobWorkingDir = _printJob->jobWorkingDirectory;

    QString dirName = QString("tiled-%1-%2-%3-%4").arg( _expoTime ).arg( _step ).arg( _space ).arg( _count ) % GetFileBaseName( jobWorkingDir );
    _path = jobWorkingDir.mid( 0, jobWorkingDir.lastIndexOf( Slash ) ) % Slash % dirName;

    QDir dir ( JobWorkingDirectoryPath );
    dir.mkdir( dirName );

    tileImages( );

    QFile::link( _path , StlModelLibraryPath % Slash % dirName );

    _manifestMgr->setFileList( _fileNameList );
    _manifestMgr->setPath( JobWorkingDirectoryPath % Slash % dirName );

    _manifestMgr->setTiled( true );
    _manifestMgr->setTilingMinExpoTime( _expoTime );
    _manifestMgr->setTilingSpace( _space );
    _manifestMgr->setTilingStep( _step );
    _manifestMgr->setTilingCount( _count );


    _manifestMgr->save();
}

void TilingManager::tileImages ( )
{
    debug( "+ TilingManager::tileImages\n");

    QPixmap pixmap;
    pixmap.load(_printJob->jobWorkingDirectory % Slash % _manifestMgr->getFirstElement());

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

    //std::reverse(tileSlots.begin(), tileSlots.end());
    std::rotate(_tileSlots.begin(),
                _tileSlots.end()-1, // this will be the new first element
                _tileSlots.end());


    OrderManifestManager::Iterator iter = _manifestMgr->iterator();


    /* iterating over slices in manifest */
    while ( iter.hasNext() ) {
        QFileInfo entry ( _printJob->jobWorkingDirectory % Slash % *iter);
        ++iter;

        /* render tiles based on slice */
        renderTiles ( entry );
    }
}

void TilingManager::renderTiles ( QFileInfo info ) {
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
        y -= ( _space / ProjectorPixelSize ) / 5;

    debug( "+ TilingManager::renderTiles x %d, y %d \n", x, y);

    painter->drawPixmap( x, y, pixmap );
}
