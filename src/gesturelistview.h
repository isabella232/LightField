#ifndef __GESTURELISTVIEW_H__
#define __GESTURELISTVIEW_H__

#include <QtCore>
#include <QtWidgets>

class GestureListView: public QListView {

    Q_OBJECT

public:

    GestureListView( QWidget* parent = nullptr ): QListView( parent ) {
        /*empty*/
    }

    GestureListView( QListViewPrivate& priv, QWidget* parent = nullptr ): QListView( priv, parent ) {
        /*empty*/
    }

    virtual ~GestureListView( ) override {
        /*empty*/
    }

protected:

    virtual bool event( QEvent* event ) override;
    bool gestureEvent( QGestureEvent* event );

private:

signals:

    void swipeGesture( QGestureEvent* event, QSwipeGesture* gesture );

public slots:

protected slots:

private slots:

};

#endif // __GESTURELISTVIEW_H__
