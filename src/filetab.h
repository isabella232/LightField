#ifndef __FILETAB_H__
#define __FILETAB_H__

#include "tabbase.h"
#include "coordinate.h"
#include "gesturelistview.h"

class Canvas;
class Loader;
class Mesh;
class ProcessRunner;
class UpgradeManager;

enum class ModelsLocation {
    Library,
    Usb
};

inline int operator+( ModelsLocation const value ) { return static_cast<int>( value ); }

char const* ToString( ModelsLocation const value );

class ModelSelectionInfo {

public:

    ModelSelectionInfo( ):
        fileName ( )
    {
        /*empty*/
    }

    ModelSelectionInfo( QString const& fileName ):
        fileName ( fileName )
    {
        /*empty*/
    }

    QString    fileName;
    size_t     vertexCount     { };
    Coordinate x               { };
    Coordinate y               { };
    Coordinate z               { };
    double     estimatedVolume { };

};

class FileTab: public InitialShowEventMixin<FileTab, TabBase> {

    Q_OBJECT

public:

    FileTab( QWidget* parent = nullptr );
    virtual ~FileTab( ) override;

    virtual TabIndex          tabIndex( )       const override { return TabIndex::File;   }

    ModelSelectionInfo const* modelSelection( ) const          { return &_modelSelection; }

protected:

    virtual void initialShowEvent( QShowEvent* event ) override;

private:

    ProcessRunner*      _processRunner           { };
    UpgradeManager*      _upgradeManager           { };

    QFileSystemModel*   _libraryFsModel          { new QFileSystemModel    };
    QFileSystemModel*   _usbFsModel              { new QFileSystemModel    };
    QFileSystemModel*   _currentFsModel          {                         };
    QPushButton*        _toggleLocationButton    { new QPushButton         };
    GestureListView*    _availableFilesListView  { new GestureListView     };
    QLabel*             _availableFilesLabel     { new QLabel              };
    QGridLayout*        _availableFilesLayout    { new QGridLayout         };
    QWidget*            _availableFilesContainer { new QWidget             };
    QLabel*             _dimensionsLabel         { new QLabel              };
    QLabel*             _errorLabel              { new QLabel              };
    QHBoxLayout*        _dimensionsLayout        {                         };
    QPushButton*        _selectButton            { new QPushButton         };
    Canvas*             _canvas                  {                         };
    QVBoxLayout*        _canvasLayout            { new QVBoxLayout         };
    Loader*             _loader                  {                         };
    QGridLayout*        _layout                  { new QGridLayout         };
    QFileSystemWatcher* _fsWatcher               { new QFileSystemWatcher  };
    QTimer*             _usbRetryTimer           { new QTimer              };

    QString             _dimensionsText;
    int                 _selectedRow             { -1 };
    QString             _slicerBuffer;
    QString             _usbPath;
    int                 _usbRetryCount           { -1 };
    QString             _userMediaPath;
    ModelSelectionInfo  _modelSelection;

    ModelsLocation      _modelsLocation          { ModelsLocation::Library };
    QPointF             _swipeLastPoint          { };

    void _loadModel( QString const& filename );

    void _checkUserMediaPath( );
    void _checkUsbPath( );

    void _startUsbRetry( );
    void _stopUsbRetry( );

    void _showLibrary( );
    void _showUsbStick( );

signals:

    void modelSelected( ModelSelectionInfo* modelSelection );

public slots:

    virtual void tab_uiStateChanged( TabIndex const sender, UiState const state ) override;

    void setUpgradeManager( UpgradeManager* upgradeManager );

protected slots:

private slots:

    void loader_gotMesh( Mesh* m );
    void loader_errorBadStl( );
    void loader_errorEmptyMesh( );
    void loader_errorMissingFile( );
    void loader_finished( );

    void libraryFsModel_directoryLoaded( QString const& name );
    void usbFsModel_directoryLoaded( QString const& name );

    void availableFilesListView_clicked( QModelIndex const& index );
    void availableFilesListView_swipeGesture( QGestureEvent* event, QSwipeGesture* gesture );
    void toggleLocationButton_clicked( bool );

    void selectButton_clicked( bool );

    void processRunner_succeeded( );
    void processRunner_failed( int const exitCode, QProcess::ProcessError const error );
    void processRunner_readyReadStandardOutput ( QString const& data );
    void processRunner_readyReadStandardError( QString const& data );

    void usbRetryTimer_timeout( );

    void fsWatcher_directoryChanged( QString const& path );

};

#endif // __FILETAB_H__
