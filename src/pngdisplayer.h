#ifndef __PNGDISPLAYER_H__
#define __PNGDISPLAYER_H__

class PngDisplayer: public QMainWindow {

    Q_OBJECT

public:

    PngDisplayer( QWidget* parent = nullptr );
    virtual ~PngDisplayer( ) override;

    bool load( QString const& fileName );
    void clear( );

protected:

private:

    QLabel* _label { new QLabel  };
    QPixmap _png;

signals:

public slots:

protected slots:

private slots:

};

#endif // __PNGDISPLAYER_H__
