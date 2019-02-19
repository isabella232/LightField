#ifndef __APP_H__
#define __APP_H__

class Window;

enum class Theme {
    unknown,
    Light,
    Dark,
};

class AppSettings {

public:

    QPoint pngDisplayWindowPosition { 0,   0 };
    QPoint mainWindowPosition       { 0, 800 };

    Theme  theme                    { Theme::Light };
    bool   frameless                { false        };

};

class App: public QApplication {

    Q_OBJECT

public:

    App( int& argc, char *argv[] );
    virtual ~App( ) override;

private:

    Window* window;

    void parseCommandLine( );

};

extern AppSettings g_settings;

#endif // __APP_H__
