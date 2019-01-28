#ifndef __SHEPHERD_H__
#define __SHEPHERD_H__

class Shepherd {

    Shepherd( Shepherd const& ) = delete;
    Shepherd( Shepherd&& ) = delete;
    Shepherd& operator=( Shepherd const& ) = delete;
    Shepherd& operator=( Shepherd&& ) = delete;

public:

    Shepherd( );
    ~Shepherd( );

    bool Start( );
    bool Stop( );

private:

    pid_t _childPid      { -1 };
    int   _stdinPipe[2]  { -1, -1 };
    int   _stdoutPipe[2] { -1, -1 };
    int   _stderrPipe[2] { -1, -1 };

    bool _CreatePipes( );

};

#endif // __SHEPHERD_H__
