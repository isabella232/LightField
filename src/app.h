#ifndef APP_H
#define APP_H

#include <QApplication>

class Window;

class App : public QApplication
{
    Q_OBJECT

public:
    App(int& argc, char *argv[]);
	virtual ~App() override;

private:
    Window* window;
};

#endif // APP_H
