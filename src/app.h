#ifndef APP_H
#define APP_H

#include <QApplication>

class Window;

class App : public QApplication
{
    Q_OBJECT

public:
    explicit App(int& argc, char *argv[]);
	~App();

private:
    Window* const window;
};

#endif // APP_H
