#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include "ordermanifestmanager.h"
#include "printjob.h"


class TilingManager {

public:
  TilingManager( OrderManifestManager* manifestMgr, PrintJob* printJob );
  ~TilingManager() = default;

  void processImages( int width, int height, int expoTime, int step, double space );
  inline QString getPath ( ) { return _path; };

protected:
  void tileImages ( );
  void renderTiles ( QFileInfo info );
  void putImageAt ( QPixmap pixmap, QPainter* painter, int i, int j );
private:
        OrderManifestManager* _manifestMgr;
        PrintJob*             _printJob;
        QString               _path;
        int                   _width;
        int                   _height;
        int                   _expoTime;
        int                   _step;
        double                _space;

        int                   _counter;
        int                   _wCount;
        int                   _hCount;
        QStringList           _fileNameList;
};



#endif // TILINGMANAGER_H
