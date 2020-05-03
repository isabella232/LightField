#ifndef __MOVEMENTSEQUENCER_H__
#define __MOVEMENTSEQUENCER_H__

class Shepherd;

enum class MoveType {
    Absolute,
    Relative,
};

class MovementInfo {

public:

    MovementInfo( MoveType const type_, double const distance_, double const speed_ ):
        type     ( type_ == MoveType::Absolute ? moveAbsolute : moveRelative ),
        distance ( distance_ ),
        speed    ( speed_    )
    {
        /*empty*/
    }

    MovementInfo( int const duration_ ):
        type     ( delay     ),
        duration ( duration_ )
    {
        /*empty*/
    }

    enum {
        moveAbsolute,
        moveRelative,
        delay
    } type;

    union {
        struct {
            double distance;
            double speed;
        };

        struct {
            int duration;
        };
    };

};

class MovementSequencer: public QObject {

    Q_OBJECT

public:

    MovementSequencer( Shepherd* shepherd, QObject* parent = nullptr );
    virtual ~MovementSequencer( ) override;

    MovementSequencer( MovementSequencer const& ) = delete;
    MovementSequencer( MovementSequencer&& )      = delete;

    MovementSequencer& operator=( MovementSequencer const& ) = delete;
    MovementSequencer& operator=( MovementSequencer&& )      = delete;

    bool isAborting( ) const {
        return _aborting;
    }

    bool isActive( ) const {
        return _active;
    }

    QList<MovementInfo> const& movements( ) const {
        return _movements;
    }

    void setMovements( QList<MovementInfo> const& movements ) {
        _movements.clear( );
        _movements.append( movements );
    }

    void clearMovements( ) {
        _movements.clear( );
    }

    void addMovement( MoveType const type, double const distance, double const speed ) {
        _movements.push_back( { type, distance, speed } );
    }

    void addDelay( int const duration ) {
        _movements.push_back( { duration } );
    }

    void execute( ) {
        if ( _active ) {
            debug( "+ MovementSequencer::execute: already active?!\n" );
            return;
        }

        _aborting = false;
        _active   = true;
        _startNextMovement( );
    }

    void abort( );

protected:

private:

    Shepherd*           _shepherd;
    QTimer*             _timer;
    QList<MovementInfo> _movements;

    std::atomic_bool    _aborting  { false };
    std::atomic_bool    _active    { false };

    void _startNextMovement( );

signals:
    ;

    void movementComplete( bool const success );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void shepherd_moveAbsoluteComplete( bool const success );
    void shepherd_moveRelativeComplete( bool const success );
    void timer_timeout( );

};

#endif // __MOVEMENTSEQUENCER_H__
