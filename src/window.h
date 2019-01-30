#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
//#include <QFileSystemWatcher>
#include <QCollator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>

class Canvas;
class Loader;
class Shepherd;

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget* parent=0);

private:
    bool load_stl(const QString& filename, bool is_reload=false);
    void sorted_insert(QStringList& list, const QCollator& collator, const QString& value);
    void build_folder_file_list();
    void setFullScreen(bool const fullScreen = true);

    QString current_file;
    QString lookup_folder;
    QStringList lookup_folder_files;

    //QFileSystemWatcher* watcher;
    Canvas* canvas;
    Shepherd* shepherd;
    Loader* loader;

    QPushButton* move_up_button;
    QPushButton* move_down_button;
    QWidget* containerWidget;
    QVBoxLayout* containerVBox;
    QHBoxLayout* buttonHBox;
    QGroupBox* buttonGroupBox;

signals:

public slots:

private slots:
    void on_bad_stl();
    void on_empty_mesh();
    void on_confusing_stl();
    void on_missing_file();

    void on_finished();
    void on_loaded(const QString& filename);
    void on_move_up();
    void on_move_down();
};

#endif // WINDOW_H
