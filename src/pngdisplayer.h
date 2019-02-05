#ifndef __PNGDISPLAYER_H__
#define __PNGDISPLAYER_H__

#include <QLabel>
#include <QMainWindow>
#include <QPixmap>

class PngDisplayer: public QMainWindow {

    Q_OBJECT

public:

    PngDisplayer( QWidget* parent = nullptr );
    ~PngDisplayer( );

    bool load( QString const& fileName );

protected:

private:

    QLabel* _label;
    QPixmap* _png { new QPixmap };

signals:

public slots:

protected slots:

private slots:

};

#endif // __PNGDISPLAYER_H__
