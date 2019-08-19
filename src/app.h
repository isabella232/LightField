#ifndef __APP_H__
#define __APP_H__

class Window;

enum class Theme {
    Dark,
    Light,
};

class AppSettings {

public:

    QPoint mainWindowPosition       { 0, 800 };
    QPoint projectorWindowPosition  { 0,   0 };
    QPoint projectorOffset          { 0,   0 };

    Theme  theme                    {        };
    bool   frameless                { false  };

    int    buildPlatformOffset      {    300 }; // µm

#if defined _DEBUG
    bool   pretendPrinterIsPrepared { false  };
    bool   ignoreShepherdFailures   { false  };
    bool   pretendPrinterIsOnline   { false  };
    bool   ignoreUsb                { false  };
#endif // defined _DEBUG

};

class App: public QApplication {

    Q_OBJECT

public:

    App( int& argc, char *argv[] );
    virtual ~App( ) override;

private:

    DebugManager* _debugManager;

    void _parseCommandLine( );
    bool _isAlreadyRunning( );
    void _recordProcessId( );
    void _setTheme( );

public slots:

    void terminate( );

public /*static*/:

    static Window* mainWindow( ) { return _window; }

private /*static*/:

    static Window* _window;

};

extern AppSettings g_settings;

#endif // __APP_H__
