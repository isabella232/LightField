#ifndef __INITIALSHOWEVENTMIXIN_H__
#define __INITIALSHOWEVENTMIXIN_H__

template<typename T, typename Base>
class InitialShowEventMixin: public Base {

public:

    template<typename ParentType>
    InitialShowEventMixin(ParentType* parent): Base(parent) {
        _initialShowEventFunc = std::bind( &InitialShowEventMixin<T, Base>::_initialShowEvent, this, std::placeholders::_1 );
    }

protected:

    std::function<void( QShowEvent* event )> _initialShowEventFunc;

    virtual void showEvent( QShowEvent* event ) override {
        if ( _initialShowEventFunc ) {
            _initialShowEventFunc( event );
            _initialShowEventFunc = nullptr;
        } else {
            event->ignore( );
        }
    }

    virtual void _initialShowEvent( QShowEvent* event ) = 0;

};

#endif // !__INITIALSHOWEVENTMIXIN_H__
