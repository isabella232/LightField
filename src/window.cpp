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

    move_up_button = new QPushButton("Move &Up");
    QObject::connect(move_up_button, &QPushButton::clicked, this, &Window::on_move_up);

    move_down_button = new QPushButton("Move &Down");
    QObject::connect(move_down_button, &QPushButton::clicked, this, &Window::on_move_down);

    buttonHBox = new QHBoxLayout();
    buttonHBox->addStretch(0);
    buttonHBox->addWidget(move_up_button);
    buttonHBox->addWidget(move_down_button);
    buttonHBox->addStretch(0);
    //buttonHBox->setContentsMargins(0, 0, 0, 0);
    buttonGroupBox = new QGroupBox();
    buttonGroupBox->setLayout(buttonHBox);
    //buttonGroupBox->setContentsMargins(0, 0, 0, 0);

    canvas = new Canvas(format, this);
    canvas->setMinimumSize(600, 400);
    canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //canvas->setContentsMargins(0, 0, 0, 0);

    containerVBox = new QVBoxLayout();
    containerVBox->addWidget(canvas);
    containerVBox->addWidget(buttonGroupBox);
    containerVBox->setContentsMargins(0, 0, 0, 0);
    containerWidget = new QWidget();
    containerWidget->setLayout(containerVBox);
    containerWidget->setContentsMargins(0, 0, 0, 0);

    setCentralWidget(containerWidget);

    //watcher = new QFileSystemWatcher(this);
    shepherd = new Shepherd(this);
}

//void Window::on_open()
//{
//    QString filename = QFileDialog::getOpenFileName(
//                this, "Load .stl file", QString(), "*.stl");
//    if (!filename.isNull())
//    {
//        load_stl(filename);
//    }
//}

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

void Window::on_finished() {
    loader = nullptr;
}

void Window::on_loaded(const QString& filename)
{
    if (filename[0] == ':') {
        return;
    }

    current_file = filename;
    setWindowTitle( current_file );
}

void Window::on_move_up()
{
    shepherd->doMove( 1 );
}

void Window::on_move_down()
{
    shepherd->doMove( -1 );
}

bool Window::load_stl(const QString& filename, bool is_reload)
{
    if (loader) {
        return false;
    }
    loader = new Loader(this, filename, is_reload);

    canvas->set_status("Loading " + filename);

    connect(loader, &Loader::got_mesh,              canvas, &Canvas::load_mesh);
    connect(loader, &Loader::error_bad_stl,         this,   &Window::on_bad_stl);
    connect(loader, &Loader::error_empty_mesh,      this,   &Window::on_empty_mesh);
    connect(loader, &Loader::warning_confusing_stl, this,   &Window::on_confusing_stl);
    connect(loader, &Loader::error_missing_file,    this,   &Window::on_missing_file);

    connect(loader, &Loader::finished,              loader, &Loader::deleteLater);
    connect(loader, &Loader::finished,              canvas, &Canvas::clear_status);
    connect(loader, &Loader::finished,              this,   &Window::on_finished);

    connect(loader, &Loader::loaded_file,           this,   &Window::on_loaded);

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
