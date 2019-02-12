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

#if defined _DEBUG
    bool debuggingPosition { false        };
#endif // defined _DEBUG

    bool  fullScreen       { true         };
    Theme theme            { Theme::Light };

};

class App: public QApplication {

    Q_OBJECT

public:

    App( int& argc, char *argv[] );
    virtual ~App( ) override;

private:

    Window* window;

};

#endif // __APP_H__
