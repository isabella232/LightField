#ifndef __PNGDISPLAYER_H__
#define __PNGDISPLAYER_H__

class PngDisplayer: public QMainWindow {

    Q_OBJECT

public:

    PngDisplayer( QWidget* parent = nullptr );

    virtual ~PngDisplayer( ) override;

protected:

private:

    QLabel* _label { new QLabel };
    QPixmap _png;

signals:

public slots:

    void clear( );
    bool loadImageFile( QString const& fileName );
    void setPixmap( QPixmap const& pixmap );

protected slots:

private slots:

};

#endif // __PNGDISPLAYER_H__
