#ifndef __SELECTTAB_H__
#define __SELECTTAB_H__

#include "coordinate.h"

class Canvas;
class Loader;
class Mesh;
class PrintJob;
class Shepherd;

class SelectTab: public QWidget {

    Q_OBJECT

public:

    SelectTab( QWidget* parent = nullptr );
    virtual ~SelectTab( ) override;

    Shepherd* shepherd( ) const { return _shepherd; }

protected:

private:

    Shepherd*         _shepherd                { };
    QFileSystemModel* _fileSystemModel         { new QFileSystemModel };
    QListView*        _availableFilesListView  { new QListView        };
    QLabel*           _availableFilesLabel     { new QLabel           };
    QGridLayout*      _availableFilesLayout    { new QGridLayout      };
    QWidget*          _availableFilesContainer { new QWidget          };
    QPushButton*      _selectButton            { new QPushButton      };
    Canvas*           _canvas                  {                      };
    Loader*           _loader                  {                      };
    QGridLayout*      _layout                  { new QGridLayout      };
    PrintJob*         _printJob                {                      };
    int               _selectedRow             { -1                   };
    QString           _fileName;

    bool _loadModel( QString const& filename );

signals:

    void modelSelected( bool const success, QString const& fileName );
    void modelDimensioned( size_t const vertexCount, Coordinate const sizeX, Coordinate const sizeY, Coordinate const sizeZ );

public slots:

    void setPrintJob( PrintJob* printJob );
    void setShepherd( Shepherd* shepherd );

protected slots:

private slots:

    void                 loader_gotMesh          ( Mesh* m );
    void                 loader_ErrorBadStl      ( );
    void                 loader_ErrorEmptyMesh   ( );
    void                 loader_ErrorMissingFile ( );
    void                 loader_Finished         ( );
    void                 loader_LoadedFile       ( QString const& filename );
    void        fileSystemModel_DirectoryLoaded  ( QString const& name );
    void        fileSystemModel_FileRenamed      ( QString const& path, QString const& oldName, QString const& newName );
    void        fileSystemModel_RootPathChanged  ( QString const& newPath );
    void availableFilesListView_clicked          ( QModelIndex const& index );
    void           selectButton_clicked          ( bool );

};

#endif // __SELECTTAB_H__
