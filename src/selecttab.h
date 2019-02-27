#ifndef __SELECTTAB_H__
#define __SELECTTAB_H__

#include "coordinate.h"
#include "gesturelistview.h"

class Canvas;
class Loader;
class Mesh;
class PrintJob;
class ProcessRunner;
class Shepherd;

class SelectTab: public QWidget {

    Q_OBJECT

public:

    SelectTab( QWidget* parent = nullptr );
    virtual ~SelectTab( ) override;

    Shepherd* shepherd( ) const { return _shepherd; }

protected:

private:

    enum class ModelsLocation {
        Library,
        Usb
    };

    Shepherd*           _shepherd                { };
    PrintJob*           _printJob                { };
    ProcessRunner*      _processRunner           { };

    QFileSystemModel*   _libraryFsModel          { new QFileSystemModel    };
    QFileSystemModel*   _usbFsModel              { new QFileSystemModel    };
    QFileSystemModel*   _currentFsModel          {                         };
    GestureListView*    _availableFilesListView  { new GestureListView     };
    QLabel*             _availableFilesLabel     { new QLabel              };
    QPushButton*        _toggleLocationButton    { new QPushButton         };
    QGridLayout*        _availableFilesLayout    { new QGridLayout         };
    QWidget*            _availableFilesContainer { new QWidget             };
    QLabel*             _dimensionsLabel         { new QLabel              };
    QLabel*             _dimensionsErrorLabel    { new QLabel              };
    QHBoxLayout*        _dimensionsLayout        { new QHBoxLayout         };
    QPushButton*        _selectButton            { new QPushButton         };
    Canvas*             _canvas                  {                         };
    QVBoxLayout*        _canvasLayout            { new QVBoxLayout         };
    Loader*             _loader                  {                         };
    QGridLayout*        _layout                  { new QGridLayout         };
    int                 _selectedRow             { -1                      };
    QString             _fileName;
    QString             _slicerBuffer;
    QString             _usbPath;
    QFileSystemWatcher* _fsWatcher               { new QFileSystemWatcher  };
    QTimer*             _usbRetryTimer           { new QTimer              };
    int                 _usbRetryCount           { -1                      };
    QString             _userMediaPath;

    ModelsLocation      _modelsLocation          { ModelsLocation::Library };
    QPointF             _swipeLastPoint          {                         };

    bool _loadModel( QString const& filename );

    void _showLibrary( );
    void _showUsbStick( );

signals:

    void modelSelected( bool const success, QString const& fileName );
    void modelDimensioned( size_t const vertexCount, Coordinate const sizeX, Coordinate const sizeY, Coordinate const sizeZ );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

    void loader_gotMesh( Mesh* m );
    void loader_ErrorBadStl( );
    void loader_ErrorEmptyMesh( );
    void loader_ErrorMissingFile( );
    void loader_Finished( );
    void loader_LoadedFile( QString const& filename );
    void libraryFsModel_directoryLoaded( QString const& name );
    void usbFsModel_directoryLoaded( QString const& name );
    void _lookForUsbStick( QString const& path );
    void availableFilesListView_clicked( QModelIndex const& index );
    void availableFilesListView_swipeGesture( QGestureEvent* event, QSwipeGesture* gesture );
    void toggleLocationButton_clicked( bool );
    void selectButton_clicked( bool );
    void processRunner_succeeded( );
    void processRunner_failed( QProcess::ProcessError const error );
    void processRunner_readyReadStandardOutput ( QString const& data );
    void processRunner_readyReadStandardError( QString const& data );
    void usbRetryTimer_timeout( );

};

#endif // __SELECTTAB_H__
