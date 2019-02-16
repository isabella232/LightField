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

    int   startY { 800          };
    Theme theme  { Theme::Light };

};

class App: public QApplication {

    Q_OBJECT

public:

    App( int& argc, char *argv[] );
    virtual ~App( ) override;

private:

    Window* window;

};

extern AppSettings g_settings;

#endif // __APP_H__
