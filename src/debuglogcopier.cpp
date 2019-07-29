#include "pch.h"

#include "debuglogcopier.h"

#include "filecopier.h"
#include "usbmountmanager.h"
#include "window.h"

DebugLogCopier::DebugLogCopier( UsbMountManager* manager, QWidget* parent ): InitialShowEventMixin<DebugLogCopier, QMainWindow>( parent ), _usbMountManager( manager ) {
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    auto origFontSize = font( ).pointSizeF( );
    auto origFont     = ModifyFont( font( ),  origFontSize );
    auto boldFont     = ModifyFont( origFont, QFont::Bold  );
    setFont( ModifyFont( origFont, LargeFontSize ) );

    _message->setAlignment( Qt::AlignCenter );
    _message->setFont( boldFont );
    _message->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    _message->setTextFormat( Qt::RichText );
    _message->setWordWrap( true );

    _messageLayout = WrapWidgetsInVBox( { _message } );

    _currentFileNameLabel->setAlignment( Qt::AlignRight | Qt::AlignTop );
    _currentFileNameLabel->setFont( origFont );
    _currentFileNameLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    _currentFileNameLabel->setText( "Current file: " );

    _currentFileName->setAlignment( Qt::AlignRight | Qt::AlignTop );
    _currentFileName->setFont( boldFont );
    _currentFileName->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    _fileSizeLabel->setAlignment( Qt::AlignRight | Qt::AlignTop );
    _fileSizeLabel->setFont( origFont );
    _fileSizeLabel->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    _fileSizeLabel->setText( "File size: " );

    _fileSize->setAlignment( Qt::AlignRight | Qt::AlignTop );
    _fileSize->setFont( boldFont );
    _fileSize->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    _progressBar->setMinimum( 0 );

    _notifications->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    _notifications->setFont( origFont );
    _notifications->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    _copyStatusLayout->addLayout( WrapWidgetsInHBox( { nullptr, _currentFileNameLabel, _currentFileName, nullptr } ) );
    _copyStatusLayout->addLayout( WrapWidgetsInHBox( { nullptr, _fileSizeLabel,        _fileSize,        nullptr } ) );
    _copyStatusLayout->addLayout( WrapWidgetsInHBox( { nullptr, _progressBar,                            nullptr } ) );
    _copyStatusLayout->addLayout( WrapWidgetsInHBox( { nullptr, _notifications,                          nullptr } ) );
    _copyStatusLayout->addStretch( );

    _innerWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    _innerWidget->setLayout( _copyStatusLayout );

    _button->setText( "Abort" );
    (void) QObject::connect( _button, &QPushButton::clicked, this, &DebugLogCopier::abortButton_clicked );

    auto layout = new QVBoxLayout;
    layout->addStretch( );
    layout->addWidget( _innerWidget );
    layout->addStretch( );
    layout->addLayout( WrapWidgetsInHBox( { nullptr, _button, nullptr } ) );
    layout->addStretch( );

    auto copyFilesGroupBox = new QGroupBox;
    copyFilesGroupBox->setFixedSize( MainWindowSize );
    copyFilesGroupBox->setLayout( layout );
    copyFilesGroupBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    copyFilesGroupBox->setTitle( "Copying files" );

    setCentralWidget( copyFilesGroupBox );
}

DebugLogCopier::~DebugLogCopier( ) {
    if ( _fileCopier ) {
        _fileCopier->abort( );
        _fileCopier->deleteLater( );
        _fileCopier = nullptr;
    }
}

void DebugLogCopier::_initialShowEvent( QShowEvent* event ) {
    auto totalWidth = MainWindowSize.width( ) * 2 / 3;

    _currentFileName->setFixedWidth( totalWidth - _currentFileNameLabel->width( ) );
    _fileSizeLabel  ->setFixedWidth( _currentFileNameLabel->width( )              );
    _fileSize       ->setFixedWidth( _currentFileName     ->width( )              );
    _progressBar    ->setFixedWidth( totalWidth                                   );
    _button         ->setFixedSize ( _button->size( ) + ButtonPadding             );

    event->accept( );
}

void DebugLogCopier::copyTo( QString const& mountPointPath ) {
    _targetPath = mountPointPath % "/LightField logs " % QDateTime::currentDateTime( ).toString( Qt::ISODate ).replace( ':', '-' );
    debug( "+ DebugLogCopier::copyTo: target path '%s'\n", _targetPath.toUtf8( ).data( ) );

    this->show( );
    getMainWindow( )->hide( );

    _remountRw_start( );
}

void DebugLogCopier::_remountRw_start( ) {
    debug( "+ DebugLogCopier::_remountRw_start: asking UsbMountManager to remount USB stick read-write\n" );

    QObject::connect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &DebugLogCopier::remountRw_finished );
    _usbMountManager->remount( true );
}

void DebugLogCopier::remountRw_finished( bool const succeeded, bool const writable ) {
    QObject::disconnect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &DebugLogCopier::remountRw_finished );

    debug( "+ DebugLogCopier::remountRw_finished: remounting USB stick read-write %s\n", SucceededString( succeeded ) );
    if ( !succeeded ) {
        _showMessage( "Unable to acquire permission to write to USB stick." );
        return;
    }

    _fileCopier_start( );
}

void DebugLogCopier::_fileCopier_start( ) {
    debug( "+ DebugLogCopier::_fileCopier_start: copying log files to '%s'\n", _targetPath.toUtf8( ).data( ) );

    for ( QString srcFileName : DebugLogPaths ) {
        if ( QFileInfo::exists( srcFileName ) ) {
            _fileList.append( QPair<QString, QString> { srcFileName, _targetPath % Slash % srcFileName.mid( srcFileName.lastIndexOf( Slash ) + 1 ) } );
        }
    }
    if ( _fileList.isEmpty( ) ) {
        _showMessage( "No files to copy." );
        return;
    }

    if ( !QDir::root( ).mkpath( _targetPath ) ) {
        _showMessage( "Couldn't create destination directory named <span style=\"font-weight: bold;\">" % _targetPath.mid( _targetPath.lastIndexOf( Slash ) + 1 ) % "</span> on the USB stick." );
        return;
    }

    _fileCopier = new FileCopier;
    //_fileCopier->moveToThread( nullptr );
    (void) QObject::connect( _fileCopier, &FileCopier::fileStarted,  this, &DebugLogCopier::fileCopier_fileStarted,  Qt::QueuedConnection );
    (void) QObject::connect( _fileCopier, &FileCopier::fileProgress, this, &DebugLogCopier::fileCopier_fileProgress, Qt::QueuedConnection );
    (void) QObject::connect( _fileCopier, &FileCopier::fileFinished, this, &DebugLogCopier::fileCopier_fileFinished, Qt::QueuedConnection );
    (void) QObject::connect( _fileCopier, &FileCopier::notify,       this, &DebugLogCopier::fileCopier_notify,       Qt::QueuedConnection );
    (void) QObject::connect( _fileCopier, &FileCopier::failure,      this, &DebugLogCopier::fileCopier_failure,      Qt::QueuedConnection );
    (void) QObject::connect( _fileCopier, &FileCopier::finished,     this, &DebugLogCopier::fileCopier_finished,     Qt::QueuedConnection );
    _fileCopier->copy( _fileList );
}

void DebugLogCopier::fileCopier_fileStarted( int const index, qint64 const totalSize ) {
    debug(
        "+ DebugLogCopier::fileCopier_fileStarted: file #%d/%d\n"
        "  + src file: '%s'\n"
        "  + dst file: '%s'\n"
        "  + size:     %lld bytes\n"
        "",
        index + 1, _fileList.count( ),
        _fileList[index]. first.toUtf8( ).data( ),
        _fileList[index].second.toUtf8( ).data( ),
        totalSize
    );

    auto fileName = _fileList[index].first;
    if ( auto index = fileName.lastIndexOf( Slash ); -1 != index ) {
        fileName = fileName.mid( index + 1 );
    }
    _currentFileName->setText( fileName );

    char const* unit;
    double scaledSize;
    ScaleSize( totalSize, scaledSize, unit );
    _fileSize->setText( GroupDigits( QString::asprintf( "%.2f", scaledSize ), ',', '.' ) % Space % unit );

    _progressBar->setMaximum( totalSize );
    _progressBar->setValue( 0 );
}

void DebugLogCopier::fileCopier_fileProgress( int const index, qint64 const bytesCopied ) {
    debug( "+ DebugLogCopier::fileCopier_fileProgress: file #%d/%d: %lld bytes copied\n", index + 1, _fileList.count( ), bytesCopied );

    _progressBar->setValue( bytesCopied );
}

void DebugLogCopier::fileCopier_fileFinished( int const index, qint64 const bytesCopied ) {
    debug( "+ DebugLogCopier::fileCopier_fileFinished: file #%d/%d: %lld bytes copied\n", index + 1, _fileList.count( ), bytesCopied );

    _progressBar->setMaximum( bytesCopied );
    _progressBar->setValue( _progressBar->maximum( ) );
}

void DebugLogCopier::fileCopier_notify( int const index, QString const message ) {
    debug( "+ DebugLogCopier::fileCopier_notify: while copying file #%d/%d: '%s'\n", index + 1, _fileList.count( ), message.toUtf8( ).data( ) );

    auto text = _notifications->text( );
    if ( !text.isEmpty( ) ) {
        text += "<br />" % LineFeed;
    }
    text += message;
    _notifications->setText( text );
}

void DebugLogCopier::fileCopier_failure( int const index, QString const message ) {
    debug( "+ DebugLogCopier::fileCopier_failure: while copying file #%d/%d: '%s'\n", index + 1, _fileList.count( ), message.toUtf8( ).data( ) );

    auto const& dstFileName = _fileList[index].second;
    _showMessage( QString { "While copying file " } % dstFileName.mid( dstFileName.lastIndexOf( Slash ) ) % ":<br/>" % message );
}

void DebugLogCopier::fileCopier_finished( int const copiedFiles, int const skippedFiles ) {
    debug( "+ DebugLogCopier::fileCopier_finished: copied %d files, skipped %d files\n", copiedFiles, skippedFiles );
    _progressBar->hide( );
    fileCopier_notify( _fileList.count( ), QString::asprintf( "Copy finished!<br /><span style=\"font-weight: bold;\">%d</span> files copied<br /><span style=\"font-weight: bold;\">%d</span> files skipped", copiedFiles, skippedFiles ) );

    _remountRo_start( );
}

void DebugLogCopier::_remountRo_start( ) {
    debug( "+ DebugLogCopier::_remountRo_start: asking UsbMountManager to remount USB stick read-only\n" );

    QObject::connect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &DebugLogCopier::remountRo_finished );
    _usbMountManager->remount( false );
}

void DebugLogCopier::remountRo_finished( bool const succeeded, bool const writable ) {
    debug( "+ DebugLogCopier::remountRo_finished: remount USB stick read-only %s\n", SucceededString( succeeded ) );

    QObject::disconnect( _usbMountManager, &UsbMountManager::filesystemRemounted, this, &DebugLogCopier::remountRo_finished );
    _showOkButton( );
}

void DebugLogCopier::_showOkButton( ) {
    debug( "+ DebugLogCopier::_showOkButton\n" );

    _button->setText( "OK" );
    (void) QObject::disconnect( _button );
    (void) QObject::connect( _button, &QPushButton::clicked, this, &DebugLogCopier::okButton_clicked );
}

void DebugLogCopier::_showMessage( QString const& message ) {
    debug( "+ DebugLogCopier::_showMessage: '%s'\n", message.toUtf8( ).data( ) );

    _message->setText( message );
    _innerWidget->setLayout( _messageLayout );
    _showOkButton( );
}

void DebugLogCopier::abortButton_clicked( bool ) {
    debug( "+ DebugLogCopier::abortButton_clicked\n" );

    (void) QObject::disconnect( _button );
    _fileCopier->abort( );
}

void DebugLogCopier::okButton_clicked( bool ) {
    debug( "+ DebugLogCopier::okButton_clicked\n" );

    (void) QObject::disconnect( _button );

    getMainWindow( )->show( );
    this->hide( );

    emit finished( );
}
