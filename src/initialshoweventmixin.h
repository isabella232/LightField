#ifndef __INITIALSHOWEVENTMIXIN_H__
#define __INITIALSHOWEVENTMIXIN_H__

template<typename T, typename Base>
class InitialShowEventMixin: public Base {

public:

    template<typename ParentType>
    InitialShowEventMixin( ParentType* parent ): Base( parent ) {
        initialShowEventFunc = std::bind( &InitialShowEventMixin<T, Base>::initialShowEvent, this, std::placeholders::_1 );
    }

protected:

    std::function<void( QShowEvent* event )> initialShowEventFunc;

    virtual void showEvent( QShowEvent* event ) override {
        if ( initialShowEventFunc ) {
            initialShowEventFunc( event );
            initialShowEventFunc = nullptr;
        } else {
            event->ignore( );
        }
    }

    virtual void initialShowEvent( QShowEvent* event ) = 0;

};

#endif // !__INITIALSHOWEVENTMIXIN_H__
