#ifndef __APP_H__
#define __APP_H__

class Window;

enum class Theme {
    Dark,
    Light,
};

class AppSettings {

public:

#if defined _DEBUG
    QPoint mainWindowPosition       { 0, 800 };
    QPoint pngDisplayWindowPosition { 0,   0 };
#endif

    Theme  theme                    {        };
#if defined _DEBUG
    bool   frameless                { false  };
    bool   pretendPrinterIsPrepared { false  };
    bool   ignoreShepherdFailures   { false  };
    bool   pretendPrinterIsOnline   { false  };
    bool   ignoreUsb                { false  };
    QPoint projectorOffset          { 0, 0   };
#endif // defined _DEBUG

};

class App: public QApplication {

    Q_OBJECT

public:

    App( int& argc, char *argv[] );
    virtual ~App( ) override;

private:

    DebugManager* _debugManager;
    Window*       _window;

    void _parseCommandLine( );
    void _setTheme( );

};

extern AppSettings g_settings;

#endif // __APP_H__
