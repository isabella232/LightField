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

    Window* mainWindow( ) { return _window; }

private:

    DebugManager* _debugManager;
    Window*       _window;

    void _parseCommandLine( );
    void _setTheme( );

};

extern AppSettings g_settings;

#endif // __APP_H__
