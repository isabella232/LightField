#ifndef __LIGHTFIELDSTYLE_H__
#define __LIGHTFIELDSTYLE_H__

class LightFieldStyle: public QProxyStyle {

    Q_OBJECT

public:

    LightFieldStyle( QString const& key );
    LightFieldStyle( QStyle* style = nullptr );
    virtual ~LightFieldStyle( ) override;

    virtual int pixelMetric( QStyle::PixelMetric metric, QStyleOption const* option = nullptr, QWidget const* widget = nullptr ) const override;

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // __LIGHTFIELDSTYLE_H__
