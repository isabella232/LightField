#include <QDebug>

#include "app.h"
#include "window.h"

App::App(int& argc, char *argv[]) :
    QApplication(argc, argv), window(new Window())
{
    QCoreApplication::setOrganizationName("Volumetric");
    QCoreApplication::setOrganizationDomain("https://www.volumetricbio.com/");
    QCoreApplication::setApplicationName("fstl");

    qDebug( ).setVerbosity( 7 );
    window->load_stl( ":gl/BoundingBox.stl" );
    window->show();
}

App::~App()
{
	delete window;
}
