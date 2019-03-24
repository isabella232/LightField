#ifndef __APP_H__
#define __APP_H__

class Window;

enum class Theme {
    Dark,
    Light,
};

class AppSettings {

public:

    QPoint pngDisplayWindowPosition { 0,   0 };
    QPoint mainWindowPosition       { 0, 800 };

    Theme  theme                    {        };
    bool   frameless                { false  };
    bool   pretendPrinterIsPrepared { false  };
    bool   ignoreShepherdFailures   { false  };

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
