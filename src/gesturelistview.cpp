#include "pch.h"

#include "gesturelistview.h"

bool GestureListView::event( QEvent *event ) {
    if ( event->type( ) == QEvent::Gesture ) {
        return gestureEvent( static_cast<QGestureEvent*>( event ) );
    }

    return QListView::event( event );
}

bool GestureListView::gestureEvent( QGestureEvent* event ) {
    debug( "+ GestureListView::gestureEvent\n" );

    if ( QGesture *swipe = event->gesture( Qt::SwipeGesture ) ) {
        debug( "  + emitting swipeGesture\n" );
        emit swipeGesture( event, static_cast<QSwipeGesture*>( swipe ) );
    }

    return true;
}
