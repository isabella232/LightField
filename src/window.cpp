#include "window.h"

#include "canvas.h"
#include "loader.h"
#include "shepherd.h"

Window::Window(QWidget *parent): QMainWindow(parent)
{
    setWindowTitle("fstl");
    setAcceptDrops(false);
    setStatusBar(nullptr);
    setFixedSize(800, 480);

    QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(2, 1);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

    canvas = new Canvas(format, this);
    canvas->setMinimumSize(600, 400);
    canvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    centralWidgetLayout = new QGridLayout( );
    centralWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
    centralWidgetLayout->addWidget( canvas );

    centralWidget = new QWidget( );
    centralWidget->setContentsMargins(0, 0, 0, 0);
    centralWidget->setLayout( centralWidgetLayout );
    centralWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    setCentralWidget(centralWidget);

    //watcher = new QFileSystemWatcher(this);

    shepherd = new Shepherd(this);
    QObject::connect( shepherd, &Shepherd::shepherd_Started,              this, &Window::shepherd_Started              );
    QObject::connect( shepherd, &Shepherd::shepherd_Finished,             this, &Window::shepherd_Finished             );
    QObject::connect( shepherd, &Shepherd::shepherd_ProcessError,         this, &Window::shepherd_ProcessError         );

    QObject::connect( shepherd, &Shepherd::printer_Online,                this, &Window::printer_Online                );
    QObject::connect( shepherd, &Shepherd::printer_Offline,               this, &Window::printer_Offline               );
    QObject::connect( shepherd, &Shepherd::printer_Position,              this, &Window::printer_Position              );
    QObject::connect( shepherd, &Shepherd::printer_Temperature,           this, &Window::printer_Temperature           );

    QObject::connect( shepherd, &Shepherd::printProcess_ShowImage,        this, &Window::printProcess_ShowImage        );
    QObject::connect( shepherd, &Shepherd::printProcess_HideImage,        this, &Window::printProcess_HideImage        );
    QObject::connect( shepherd, &Shepherd::printProcess_StartedPrinting,  this, &Window::printProcess_StartedPrinting  );
    QObject::connect( shepherd, &Shepherd::printProcess_FinishedPrinting, this, &Window::printProcess_FinishedPrinting );
}

void Window::shepherd_Started( ) {
    fprintf( stderr, "+ Window::shepherd_Started\n" );
}

void Window::shepherd_Finished( ) {
    fprintf( stderr, "+ Window::shepherd_Finished\n" );
}

void Window::shepherd_ProcessError( ) {
    fprintf( stderr, "+ Window::shepherd_ProcessError\n" );
}

void Window::printer_Online( ) {
    fprintf( stderr, "+ Window::printer_Online\n" );
}

void Window::printer_Offline( ) {
    fprintf( stderr, "+ Window::printer_Offline\n" );
}

void Window::printer_Position( float position ) {
    fprintf( stderr, "+ Window::printer_Position: position: %f\n", position );
}

void Window::printer_Temperature( char const* temperatureInfo ) {
    fprintf( stderr, "+ Window::printer_Temperature: temperatureInfo: '%s'\n", temperatureInfo );
}

void Window::printProcess_ShowImage( char const* fileName, char const* brightness, char const* index, char const* total ) {
    fprintf( stderr, "+ Window::printProcess_ShowImage: fileName '%s', brightness %s, index %s, total %s\n", fileName, brightness, index, total );
}

void Window::printProcess_HideImage( ) {
    fprintf( stderr, "+ Window::printProcess_HideImage\n" );
}

void Window::printProcess_StartedPrinting( ) {
    fprintf( stderr, "+ Window::printProcess_StartedPrinting\n" );
}

void Window::printProcess_FinishedPrinting( ) {
    fprintf( stderr, "+ Window::printProcess_FinishedPrinting\n" );
}

void Window::on_bad_stl()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "This <code>.stl</code> file is invalid or corrupted.<br>"
                          "Please export it from the original source, verify, and retry.");
}

void Window::on_empty_mesh()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "This file is syntactically correct<br>but contains no triangles.");
}

void Window::on_confusing_stl()
{
    QMessageBox::warning(this, "Warning",
                         "<b>Warning:</b><br>"
                         "This <code>.stl</code> file begins with <code>solid </code>but appears to be a binary file.<br>"
                         "<code>fstl</code> loaded it, but other programs may be confused by this file.");
}

void Window::on_missing_file()
{
    QMessageBox::critical(this, "Error",
                          "<b>Error:</b><br>"
                          "The target file is missing.<br>");
}

//void Window::set_watched(const QString& filename)
//{
//    const auto files = watcher->files();
//    if (files.size())
//    {
//        watcher->removePaths(watcher->files());
//    }
//    watcher->addPath(filename);
//
//    QSettings settings;
//    auto recent = settings.value(RECENT_FILE_KEY).toStringList();
//    const auto f = QFileInfo(filename).absoluteFilePath();
//    recent.removeAll(f);
//    recent.prepend(f);
//    while (recent.size() > MAX_RECENT_FILES)
//    {
//        recent.pop_back();
//    }
//    settings.setValue(RECENT_FILE_KEY, recent);
//}
//
//void Window::on_projection(QAction* proj)
//{
//    if (proj == perspective_action)
//    {
//        canvas->view_perspective();
//    }
//    else
//    {
//        canvas->view_orthographic();
//    }
//}
//
//void Window::on_drawMode(QAction* mode)
//{
//    if (mode == shaded_action)
//    {
//        canvas->draw_shaded();
//    }
//    else
//    {
//        canvas->draw_wireframe();
//    }
//}

void Window::on_loaded(const QString& filename)
{
    current_file = filename;
}

void Window::on_move_up()
{
    shepherd->doMove( 1 );
}

void Window::on_move_down()
{
    shepherd->doMove( -1 );
}

void Window::closeEvent( QCloseEvent* event ) {
    shepherd->terminate( );
    event->accept( );
}

bool Window::load_stl(const QString& filename, bool is_reload)
{
    if (loader) {
        return false;
    }

    canvas->set_status("Loading " + filename);

    loader = new Loader(this, filename, is_reload);
    connect(loader, &Loader::got_mesh,              canvas, &Canvas::load_mesh);
    connect(loader, &Loader::error_bad_stl,         this,   &Window::on_bad_stl);
    connect(loader, &Loader::error_empty_mesh,      this,   &Window::on_empty_mesh);
    connect(loader, &Loader::warning_confusing_stl, this,   &Window::on_confusing_stl);
    connect(loader, &Loader::error_missing_file,    this,   &Window::on_missing_file);
    connect(loader, &Loader::finished,              loader, &Loader::deleteLater);
    connect(loader, &Loader::finished,              canvas, &Canvas::clear_status);

    if (filename[0] != ':') {
        connect(loader, &Loader::loaded_file,       this,   &Window::setWindowTitle);
        connect(loader, &Loader::loaded_file,       this,   &Window::on_loaded);
    }

    loader->start();
    return true;
}

void Window::sorted_insert(QStringList& list, const QCollator& collator, const QString& value)
{
    int start = 0;
    int end = list.size() - 1;
    int index = 0;
    while (start <= end){
        int mid = (start+end)/2;
        if (list[mid] == value) {
            return;
        }
        int compare = collator.compare(value, list[mid]);
        if (compare < 0) {
            end = mid-1;
            index = mid;
        } else {
            start = mid+1;
            index = start;
        }
    }

    list.insert(index, value);
}

void Window::build_folder_file_list()
{
    QString current_folder_path = QFileInfo(current_file).absoluteDir().absolutePath();
    if (!lookup_folder_files.isEmpty())
    {
        if (current_folder_path == lookup_folder) {
            return;
        }

        lookup_folder_files.clear();
    }
    lookup_folder = current_folder_path;

    QCollator collator;
    collator.setNumericMode(true);

    QDirIterator dirIterator(lookup_folder, QStringList() << "*.stl", QDir::Files | QDir::Readable | QDir::Hidden);
    while (dirIterator.hasNext()) {
        dirIterator.next();

        QString name = dirIterator.fileName();
        sorted_insert(lookup_folder_files, collator, name);
    }
}

void Window::setFullScreen(bool const fullScreen)
{
    if (fullScreen) {
        setWindowState(windowState() |  Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}
