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

    MovementInfo( int const duration_ ): type ( delay ), duration ( duration_ ) { /*empty*/ }

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
using MovementCollection = QList<MovementInfo>;

class MovementSequencer: public QObject {

    Q_OBJECT

public:

    MovementSequencer( Shepherd* shepherd, QObject* parent = nullptr );
    virtual ~MovementSequencer( ) override;

    MovementSequencer( MovementSequencer const& value ) = delete;
    MovementSequencer( MovementSequencer&& value ) = delete;

    MovementSequencer& operator=( MovementSequencer const& value ) = delete;
    MovementSequencer& operator=( MovementSequencer&& value ) = delete;

    void addMovement( MoveType const type, double const distance, double const speed );
    void addDelay( int const duration );
    void execute( );

    MovementCollection const& movements( ) const {
        return _movements;
    }

    void setMovements( MovementCollection const& movements );

protected:

private:

    Shepherd*          _shepherd;
    QTimer*            _timer;
    MovementCollection _movements;

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
