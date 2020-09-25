#ifndef TILINGMANAGER_H
#define TILINGMANAGER_H

#include <QtCore>
#include <QtWidgets>
#include "ordermanifestmanager.h"
#include "printjob.h"

class TilingManager: public QObject
{
    Q_OBJECT

public:
    TilingManager();
    ~TilingManager() = default;

    OrderManifestManager* processImages(int width, int height, double baseExpoTime,
        double baseStep, double bodyExpoTime, double bodyStep, int space, int count);
    inline QString getPath () { return _path; }

signals:
  void statusUpdate(const QString &messgae);
  void progressUpdate(int percentage);

protected:
  void tileImages ();
  void renderTiles0Step(QFileInfo info, int sequence);
  void renderTiles (QFileInfo info, int sequence);
  void putImageAt (QPixmap pixmap, QPainter* painter, int i, int j);
private:
        QString               _path;
        int                   _width;
        int                   _height;
        double                _baseExpoTime;
        double                _baseStep;
        double                _bodyExpoTime;
        double                _bodyStep;
        int                   _space;
        int                   _spacePx;
        int                   _count;

        int                   _counter;
        int                   _wCount;
        int                   _hCount;
        QStringList           _fileNameList;
        QList<double>         _expoTimeList;
        QList<int>            _layerThicknessList;
        std::vector<int>      _tileSlots;
};



#endif // TILINGMANAGER_H
