#include "pch.h"

#include <QtCore>
#include <QtWidgets>

//#include <sys/sysinfo.h>

#include "preparetab.h"

#include "hasher.h"
#include "printjob.h"
#include "printmanager.h"
#include "printprofile.h"
#include "shepherd.h"
#include "slicesorderpopup.h"
#include "thicknesswindow.h"
#include "slicertask.h"
#include "timinglogger.h"
#include "usbmountmanager.h"
#include "window.h"

PrepareTab::PrepareTab(QWidget* parent ): InitialShowEventMixin<PrepareTab, TabBase>(parent) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto font12pt    = ModifyFont( origFont, 14.0 );
    auto font22pt    = ModifyFont( origFont, LargeFontSize );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome" );

    _threadPool.setMaxThreadCount(1);

    _layerThicknessLabel->setEnabled( false );
    _layerThicknessLabel->setText( "Layer Height Resolution:" );

    _layerThickness100Button->setEnabled( false );
    _layerThickness100Button->setChecked( true );
    _layerThickness100Button->setFont( font12pt );
    _layerThickness100Button->setText( "Standard Res (100 µm)" );
    QObject::connect( _layerThickness100Button, &QPushButton::clicked, this, &PrepareTab::layerThickness100Button_clicked );

    _layerThickness50Button->setEnabled( false );
    _layerThickness50Button->setChecked( false );
    _layerThickness50Button->setText( "Medium Res (50 µm)" );
    _layerThickness50Button->setFont( font12pt );
    QObject::connect( _layerThickness50Button, &QPushButton::clicked, this, &PrepareTab::layerThickness50Button_clicked );

    _layerThicknessCustomButton->setEnabled( false );
    _layerThicknessCustomButton->setChecked( false );
    _layerThicknessCustomButton->setText( "Custom (advanced slicing)" );
    _layerThicknessCustomButton->setFont( font12pt );
    QObject::connect( _layerThicknessCustomButton, &QPushButton::clicked, this, &PrepareTab::layerThicknessCustomButton_clicked );

#if defined EXPERIMENTAL
    _layerThickness20Button->setEnabled( false );
    _layerThickness20Button->setChecked( false );
    _layerThickness20Button->setText( "High Res (20 µm)" );
    _layerThickness20Button->setFont( font12pt );
    QObject::connect( _layerThickness20Button, &QPushButton::clicked, this, &PrepareTab::layerThickness20Button_clicked );
#endif

    _sliceStatusLabel->setText( "Slicer:" );

    _sliceStatus->setText( "Idle" );
    _sliceStatus->setFont( boldFont );

    _imageGeneratorStatusLabel->setText( "Image generator:" );

    _imageGeneratorStatus->setText( "Idle" );
    _imageGeneratorStatus->setFont( boldFont );

    _prepareMessage->setAlignment( Qt::AlignCenter );
    _prepareMessage->setTextFormat( Qt::RichText );
    _prepareMessage->setWordWrap( true );
    _prepareMessage->setText( "Tap the <b>Prepare</b> button to prepare the printer." );

    _prepareProgress->setAlignment( Qt::AlignCenter );
    _prepareProgress->setFormat( { } );
    _prepareProgress->setRange( 0, 0 );
    _prepareProgress->setTextVisible( false );
    _prepareProgress->hide( );

    _prepareGroup->setTitle( "Printer preparation" );
    _prepareGroup->setLayout( WrapWidgetsInVBox(
        nullptr,
        WrapWidgetsInHBox( _prepareMessage ),
        nullptr,
        WrapWidgetsInHBox( _prepareProgress ),
        nullptr
    ) );

    _prepareGroup->setFixedHeight(110);
    _warningHotImage = new QPixmap { QString { ":images/warning-hot.png" } };
    _warningHotLabel->setAlignment( Qt::AlignCenter );
    _warningHotLabel->setContentsMargins( { } );
    _warningHotLabel->setFixedSize( 43, 43 );
    _warningHotLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _warningUvImage = new QPixmap { QString { ":images/warning-uv.png"  } };
    _warningUvLabel->setAlignment( Qt::AlignCenter );
    _warningUvLabel->setContentsMargins( { } );
    _warningUvLabel->setFixedSize( 43, 43 );
    _warningUvLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _prepareButton->setEnabled( false );
    _prepareButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _prepareButton->setFont( font22pt );
    _prepareButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _prepareButton->setText( "Prepare..." );
    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );

    _adjustReset->setEnabled( true );
    _adjustReset->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _adjustReset->setFont( font22pt );
    _adjustReset->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustReset->setText( "Reset" );
    QObject::connect( _adjustReset, &QPushButton::clicked, this, []() {
        printJob.setPrintOffset(QPoint(0,0));
    });

    _adjustUp->setText("⇧");
    _adjustUp->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustUp->setFixedSize( SmallMainButtonSize.height(), SmallMainButtonSize.height() );
    _adjustUp->setFont( font22pt );
    _adjustUp->setVisible(true);
    QObject::connect( _adjustUp, &QPushButton::clicked, this, [this]() {
        QPoint current = printJob.getPrintOffset();
        int step = _adjustPrecision->getValue();

        printJob.setPrintOffset(QPoint(current.x(), current.y() + step));
    });

    _adjustLeft->setText("⇦");
    _adjustLeft->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustLeft->setFixedSize( SmallMainButtonSize.height(), SmallMainButtonSize.height() );
    _adjustLeft->setFont( font22pt );
    _adjustLeft->setVisible(true);
    QObject::connect( _adjustLeft, &QPushButton::clicked, this, [this]() {
        QPoint current = printJob.getPrintOffset();
        int step = _adjustPrecision->getValue();

        printJob.setPrintOffset(QPoint(current.x() - step, current.y()));
    });
    _adjustRight->setText("⇨");
    _adjustRight->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustRight->setFixedSize( SmallMainButtonSize.height(), SmallMainButtonSize.height() );
    _adjustRight->setFont( font22pt );
    _adjustRight->setVisible(true);
    QObject::connect( _adjustRight, &QPushButton::clicked, this, [this]() {
        QPoint current = printJob.getPrintOffset();
        int step = _adjustPrecision->getValue();

        printJob.setPrintOffset(QPoint(current.x() + step, current.y()));
    });

    _adjustDown ->setText("⇩");
    _adjustDown->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustDown->setFixedSize( SmallMainButtonSize.height(), SmallMainButtonSize.height() );
    _adjustDown->setFont( font22pt );
    _adjustDown->setVisible(true);
    QObject::connect( _adjustDown, &QPushButton::clicked, this, [this]() {
        QPoint current = printJob.getPrintOffset();
        int step = _adjustPrecision->getValue();

        printJob.setPrintOffset(QPoint(current.x(), current.y() - step));
    });

    _adjustLightBulb->setText("💡");
    _adjustLightBulb->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustLightBulb->setFixedSize( SmallMainButtonSize.height(), SmallMainButtonSize.height() );
    _adjustLightBulb->setFont( font22pt );
    _adjustLightBulb->setVisible(true);
    _adjustLightBulb->setCheckable(true);
    //_adjustLightBulb->setStyleSheet("QPushButton{background-color:red;} QPushButton:checked{background-color:green;}");

    QObject::connect( _adjustLightBulb, &QPushButton::toggled, [this](bool toggled) {

        if(toggled) {
            _pngDisplayer->loadImageFile(printJob.getLayerPath(_visibleLayer));
            QProcess::startDetached( SetProjectorPowerCommand, { QString { "%1" }.arg( PercentagePowerLevelToRawLevel( activeProfileRef->baseLayerParameters().powerLevel() )) } );

        } else {
            _pngDisplayer->clear();
            QProcess::startDetached( SetProjectorPowerCommand, { QString { "%1" }.arg( PercentagePowerLevelToRawLevel( 0 )) } );
        }

        //_pngDisplayer->loadImageFile(printJob.getLayerPath(_visibleLayer));
    });


    _adjustValue->setFont(boldFont);

    _adjustGroup->setTitle("Digital Projection Offset");
    _adjustGroup->setVisible(false);
    _adjustGroup->setLayout(WrapWidgetsInVBox(
        nullptr,
        WrapWidgetsInHBox(nullptr, _adjustUp, nullptr),
        WrapWidgetsInHBox(_adjustLeft, nullptr, _adjustValue, nullptr, _adjustRight),
        WrapWidgetsInHBox(nullptr, _adjustDown, nullptr),
        WrapWidgetsInHBox(nullptr, _adjustLightBulb),
        _adjustPrecision,
        _adjustReset
    ));

    _adjustGroup->setFixedWidth( MainButtonSize.width( ) );
    _adjustGroup->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

    _adjustProjection->setEnabled(false);
    _adjustProjection->setFixedSize( 43, 43 );
    _adjustProjection->setFont( font22pt );
    _adjustProjection->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _adjustProjection->setText( "✥" );
    _adjustProjection->setCheckable(true);
    QObject::connect( _adjustProjection, &QPushButton::toggled, [this](bool toggled) {

        if(toggled) {
            _optionsContainer->setVisible(false);
            _prepareButton->setVisible(false);
            _orderButton->setVisible(false);
            _sliceButton->setVisible(false);
            _adjustGroup->setVisible(true);
        } else {
            _optionsContainer->setVisible(true);
            _prepareButton->setVisible(true);
            _orderButton->setVisible(true);
            _sliceButton->setVisible(true);
            _adjustGroup->setVisible(false);
            if(_adjustLightBulb->isChecked()) {
                _adjustLightBulb->click();
            }
        }
    });

    //_copyToUSBButton->setEnabled( false );
    //_copyToUSBButton->setFixedSize( MainButtonSize );
    //_copyToUSBButton->setFont( font22pt );
    //_copyToUSBButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    //_copyToUSBButton->setText( "Copy to USB" );
    //QObject::connect( _copyToUSBButton, &QPushButton::clicked, this, &PrepareTab::copyToUSB_clicked );

    _optionsContainer->setFixedWidth( MainButtonSize.width( ) );
    _optionsContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _optionsContainer->setLayout( WrapWidgetsInVBox(
        _layerThicknessLabel,
#if defined EXPERIMENTAL
         WrapWidgetsInVBox(
             _layerThickness100Button,
             _layerThickness50Button,
             _layerThickness20Button,
             _layerThicknessCustomButton
         ),
#else
          WrapWidgetsInVBox(
              _layerThickness100Button,
              _layerThickness50Button,
              _layerThicknessCustomButton
          ),
#endif // defined EXPERIMENTAL
        WrapWidgetsInHBox( _sliceStatusLabel,          nullptr, _sliceStatus          ),
        WrapWidgetsInHBox( _imageGeneratorStatusLabel, nullptr, _imageGeneratorStatus ),
        _prepareGroup,
        WrapWidgetsInVBox(
            WrapWidgetsInHBox( nullptr, _warningHotLabel, nullptr, _warningUvLabel, nullptr )
        )
    ) );

    _sliceButton->setEnabled( false );
    _sliceButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _sliceButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _sliceButton->setFont( font22pt );
    _sliceButton->setText( "Slice..." );
    QObject::connect( _sliceButton, &QPushButton::clicked, this, &PrepareTab::sliceButton_clicked );

    _orderButton->setEnabled( false );
    _orderButton->setFixedSize( MainButtonSize.width(), SmallMainButtonSize.height() );
    _orderButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _orderButton->setFont( font22pt );
    _orderButton->setText( "Order editor" );
    QObject::connect( _orderButton, &QPushButton::clicked, this, &PrepareTab::orderButton_clicked );

    _currentLayerImage->setAlignment( Qt::AlignCenter );
    _currentLayerImage->setContentsMargins( { } );
    _currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerImage->setStyleSheet( "QWidget { background: black }" );

    for ( auto button : { _navigateFirst, _navigatePrevious, _navigateNext, _navigateLast } ) {
        button->setFont( fontAwesome );
    }

    _navigateFirst   ->setText( FA_FastBackward );
    _navigatePrevious->setText( FA_Backward     );
    _navigateNext    ->setText( FA_Forward      );
    _navigateLast    ->setText( FA_FastForward  );

    QObject::connect( _navigateFirst,    &QPushButton::clicked, this, &PrepareTab::navigateFirst_clicked    );
    QObject::connect( _navigatePrevious, &QPushButton::clicked, this, &PrepareTab::navigatePrevious_clicked );
    QObject::connect( _navigateNext,     &QPushButton::clicked, this, &PrepareTab::navigateNext_clicked     );
    QObject::connect( _navigateLast,     &QPushButton::clicked, this, &PrepareTab::navigateLast_clicked     );

    _navigateCurrentLabel->setAlignment( Qt::AlignCenter );
    _navigateCurrentLabel->setText( "0/0" );

    _navigationLayout = WrapWidgetsInHBox( nullptr, _navigateFirst, _navigatePrevious, _navigateCurrentLabel, _navigateNext, _navigateLast, nullptr );
    _navigationLayout->setAlignment( Qt::AlignCenter );

    _setNavigationButtonsEnabled( false );

    QLabel* spacer { new QLabel };
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    spacer->setFixedWidth(170);
    _currentLayerLayout = WrapWidgetsInVBox(
        _currentLayerImage,
        WrapWidgetsInHBox(spacer, _navigationLayout, nullptr, _printOffsetLabel, nullptr, _adjustProjection)
    );
    _currentLayerLayout->setAlignment( Qt::AlignTop | Qt::AlignHCenter );

    _currentLayerGroup->setTitle( "Current layer" );
    _currentLayerGroup->setMinimumSize( MaximalRightHandPaneSize );
    _currentLayerGroup->setLayout( _currentLayerLayout );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _optionsContainer, 0, 0, 1, 1 );
    _layout->addWidget( _prepareButton, 1, 0, 1, 1 );
    _layout->addWidget( _orderButton, 2, 0, 1, 1 );
    _layout->addWidget( _sliceButton, 3, 0, 1, 1 );
    _layout->addWidget( _currentLayerGroup, 0, 1, 2, 1 );
    _layout->addWidget( _adjustGroup, 0, 0, 4, 1);
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );


    setLayout( _layout );
}

PrepareTab::~PrepareTab( ) {
    /*empty*/
}

void PrepareTab::_connectPrintManager( ) {
    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::lampStatusChange, this, &PrepareTab::printManager_lampStatusChange );
    }
}

void PrepareTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,            this, &PrepareTab::printer_online            );
        QObject::connect( _shepherd, &Shepherd::printer_offline,           this, &PrepareTab::printer_offline           );
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &PrepareTab::printer_temperatureReport );
    }
}

void PrepareTab::_initialShowEvent( QShowEvent* event ) {
    _currentLayerImage->setFixedSize(_currentLayerImage->width( ),
        static_cast<int>(_currentLayerImage->width( ) / AspectRatio16to10 + 0.5));
    update();

    event->accept();
}

void PrepareTab::_connectUsbMountManager()
{
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemMounted,   this, &PrepareTab::usbMountManager_filesystemMounted   );
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemUnmounted, this, &PrepareTab::usbMountManager_filesystemUnmounted );
}

bool PrepareTab::_checkPreSlicedFiles(const QString &directory, bool isBody)
{
    debug( "+ PrepareTab::_checkPreSlicedFiles\n" );

    // check that the sliced SVG file is newer than the STL file
    auto modelFile = QFileInfo {printJob.getModelFilename()};
    if ( !modelFile.exists( ) ) {
        debug( "  + Fail: model file does not exist\n" );
        return false;
    }
    auto slicedSvgFile = QFileInfo { directory + Slash + SlicedSvgFileName };
    if ( !slicedSvgFile.exists( ) ) {
        debug( "  + Fail: sliced SVG file does not exist\n" );
        return false;
    }

    // check that the sliced SVG file is newer than the STL file
    // disabled
    //auto slicedSvgFileLastModified = slicedSvgFile.lastModified( );
    if ( !printJob.getModelFilename().isEmpty( ) ) {
        auto modelFile = QFileInfo { printJob.getModelFilename() };
        if ( !modelFile.exists( ) ) {
            debug( "  + Fail: model file does not exist\n" );
            return false;
        }
        /*if ( !printJob.getModelFilename().isEmpty( ) && ( modelFile.lastModified( ) > slicedSvgFileLastModified ) ) {
            debug( "  + Fail: model file is newer than sliced SVG file\n" );
            return false;
        }*/
    }

    int layerNumber     = -1;
    int prevLayerNumber = -1;

    QSharedPointer<OrderManifestManager> manifestMgr { new OrderManifestManager() };

    manifestMgr->setPath(directory);
    QStringList errors;
    QStringList warnings;

    switch (manifestMgr->parse(&errors, &warnings))
    {
    case ManifestParseResult::POSITIVE_WITH_WARNINGS: {
        QString warningsStr = warnings.join("<br>");

            _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually: " % warningsStr);
        }
        /* FALLTHROUGH */

    case ManifestParseResult::POSITIVE:
        break;
    case ManifestParseResult::FILE_CORRUPTED:
    case ManifestParseResult::FILE_NOT_EXIST: {
            QString errorsStr = errors.join("<br>");
            _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually. <br>" % errorsStr);

            SlicesOrderPopup slicesOrderPopup { manifestMgr };
            slicesOrderPopup.exec();

            break;
        }
    }

    OrderManifestManager::Iterator iter = manifestMgr->iterator();

    while (iter.hasNext()) {
        QFileInfo entry(directory % Slash % *iter);
        ++iter;

        if (!entry.exists()) {
            debug( "  + Fail: layer PNG file %s does not exist\n", entry.fileName().toUtf8().data());
            return false;
        }

        layerNumber = RemoveFileExtension(entry.baseName()).toInt();
        if ( layerNumber != ( prevLayerNumber + 1 ) ) {
            debug("  + Fail: gap in layer numbers between %d and %d\n", prevLayerNumber, layerNumber);
            return false;
        }

        prevLayerNumber = layerNumber;
    }

    if(isBody) {
        printJob.setBodyManager(manifestMgr);
    } else {
        printJob.setBaseManager(manifestMgr);   
    }

    debug("  + Success\n");
    return true;
}

bool PrepareTab::_checkOneSliceDirectory(const QString &directory, bool isBody)
{
    bool isPreSliced;

    QDir slicesDir { directory };
    isPreSliced = _checkPreSlicedFiles(directory, isBody);
    debug("  + pre-sliced layers are %sgood\n", isPreSliced ? "" : "NOT ");

    if (!isPreSliced) {
        slicesDir.removeRecursively();
    }

    return isPreSliced;
}

bool PrepareTab::_checkSliceDirectories()
{
    QString sliceDirectoryBase { JobWorkingDirectoryPath % Slash % printJob.getModelHash() };
    QString baseSliceDirectory;
    QString bodySliceDirectory;
    bool preSliced = true;

    if(printJob.getDirectoryMode()) {
        debug("+ PrepareTab::_checkSliceDirectories: directory mode, nothing to do\n");
        emit uiStateChanged(TabIndex::Prepare, UiState::PrintJobReady);
        return true;
    }

    if(printJob.hasBaseLayers()) {
        baseSliceDirectory = QString("%1-%2")
            .arg(sliceDirectoryBase)
            .arg(printJob.getSelectedBaseLayerThickness());
    }

    bodySliceDirectory = QString("%1-%2")
        .arg(sliceDirectoryBase)
        .arg(printJob.getSelectedBodyLayerThickness());

    debug(
        "+ PrepareTab::_checkSliceDirectories:"
        "  + model filename:        '%s'\n"
        "  + base slices directory: '%s'\n"
        "  + body slices directory: '%s'\n"
        "",
        printJob.getModelFilename().toUtf8().data(),
        baseSliceDirectory.toUtf8().data(),
        bodySliceDirectory.toUtf8().data()
    );

    if(printJob.hasBaseLayers())
        preSliced &= _checkOneSliceDirectory(baseSliceDirectory, false);
    preSliced &= _checkOneSliceDirectory(bodySliceDirectory, true);

    _setNavigationButtonsEnabled(preSliced);
    _setSliceControlsEnabled(true);

    if (preSliced) {
        _sliceButton->setText(_layerThicknessCustomButton->isChecked() ? "Custom reslice..." : "Reslice...");
        _orderButton->setEnabled(false);
        _reslice = true;
        _restartPreview();
        //_copyToUSBButton->setEnabled( true );

        emit uiStateChanged(TabIndex::Prepare, UiState::PrintJobReady);
    } else {
        _navigateCurrentLabel->setText("0/0");
        _sliceButton->setText(_layerThicknessCustomButton->isChecked() ? "Custom slice..." : "Slice..."); 
        _orderButton->setEnabled(false);
        _reslice = false;
        //_copyToUSBButton->setEnabled( false );
    }

    emit slicingNeeded(!preSliced);

    update();
    return preSliced;
}

void PrepareTab::layerThickness100Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness100Button_clicked\n" );
    printJob.setBaseLayerCount(2);
    printJob.setSelectedBaseLayerThickness(100);
    printJob.setSelectedBodyLayerThickness(100);

    _checkSliceDirectories();
}

void PrepareTab::layerThickness50Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness50Button_clicked\n" );
    printJob.setBaseLayerCount(2);
    printJob.setSelectedBaseLayerThickness(50);
    printJob.setSelectedBodyLayerThickness(50);

    _checkSliceDirectories();
}

#if defined EXPERIMENTAL
void PrepareTab::layerThickness20Button_clicked( bool ) {
    debug( "+ PrepareTab::layerThickness20Button_clicked\n" );
    printJob.setBaseLayerCount(2);
    printJob.setSelectedBaseLayerThickness(20);
    printJob.setSelectedBodyLayerThickness(20);

    _checkSliceDirectories();
}
#endif // defined EXPERIMENTAL

void PrepareTab::layerThicknessCustomButton_clicked( bool ) {
    ThicknessWindow *dialog = new ThicknessWindow(_initAfterSelect, this);
    switch (dialog->exec()) {
    case QDialog::Rejected:
        _layerThicknessCustomButton->setChecked(false);
        _layerThickness100Button->setChecked(true);
    }

    _adjustProjection->setEnabled(false);
    _initAfterSelect = false;
    _checkSliceDirectories();
}

void PrepareTab::_setNavigationButtonsEnabled( bool const enabled ) {
    _navigateFirst   ->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigatePrevious->setEnabled( enabled && ( _visibleLayer > 0 ) );
    _navigateNext    ->setEnabled( enabled && ( _visibleLayer + 1 < printJob.totalLayerCount() ) );
    _navigateLast    ->setEnabled( enabled && ( _visibleLayer + 1 < printJob.totalLayerCount() ) );

    update( );
}

void PrepareTab::_showLayerImage(int const layer)
{
    _navigateCurrentLabel->setText(QString { "%1/%2" }
        .arg(layer + 1)
        .arg(printJob.totalLayerCount()));

    _showLayerImage(printJob.getLayerPath(layer));
    update();
}

void PrepareTab::_showLayerImage(const QString &path)
{
    debug("+ PrepareTab::_showLayerImage by path %s\n", path.toUtf8().data());
    QPixmap pixmap { path };

    if ((pixmap.width() > _currentLayerImage->width()) ||
        (pixmap.height() > _currentLayerImage->height())) {
        pixmap = pixmap.scaled(_currentLayerImage->size(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }

    if (_adjustLightBulb->isChecked()) {
        _pngDisplayer->loadImageFile(path);
    }

    _currentLayerImage->setPixmap(pixmap);
    update();
}

void PrepareTab::_setSliceControlsEnabled(bool const enabled)
{
    _sliceButton->setEnabled(enabled);
    _layerThicknessLabel->setEnabled(enabled);
    _layerThickness100Button->setEnabled(enabled);
    _layerThickness50Button->setEnabled(enabled);
    _layerThicknessCustomButton->setEnabled(enabled);
#if defined EXPERIMENTAL
    _layerThickness20Button->setEnabled(enabled);
#endif // defined EXPERIMENTAL

    update( );
}

void PrepareTab::_updatePrepareButtonState()
{
    _prepareButton->setEnabled(_isPrinterOnline && _isPrinterAvailable);
    update( );
}

void PrepareTab::_handlePrepareFailed( ) {
    _prepareMessage->setText( "Preparation failed." );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );

    _prepareButton->setText( "Retry" );
    _prepareButton->setEnabled( true );

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );
    emit preparePrinterComplete( false );

    update( );
}

void PrepareTab::navigateFirst_clicked( bool ) {
    _visibleLayer = 0;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigatePrevious_clicked( bool ) {
    if ( _visibleLayer > 0 ) {
        --_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigateNext_clicked( bool ) {
    if ( _visibleLayer + 1 < printJob.totalLayerCount() ) {
        ++_visibleLayer;
    }
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::navigateLast_clicked( bool ) {
    _visibleLayer = printJob.totalLayerCount() - 1;
    _setNavigationButtonsEnabled( true );
    _showLayerImage( _visibleLayer );

    update( );
}

void PrepareTab::orderButton_clicked( bool ) {
    QSharedPointer<OrderManifestManager> manifestMgr { new OrderManifestManager() };

    manifestMgr->setPath(printJob.getDirectoryPath());

    SlicesOrderPopup popup { manifestMgr };
    popup.exec();

    emit uiStateChanged(TabIndex::File, UiState::SelectCompleted);
}

void PrepareTab::sliceButton_clicked(bool)
{
    debug("+ PrepareTab::sliceButton_clicked\n");
    debug("  + number of base layers: %d\n", printJob.getBaseLayerCount());
    debug("  + base layer thickness: %d\n", printJob.getSelectedBaseLayerThickness());
    debug("  + body layer thickness: %d\n", printJob.getSelectedBodyLayerThickness());

    QString sliceDirectoryBase { JobWorkingDirectoryPath % Slash % printJob.getModelHash() };
    QString baseSliceDirectory = QString("%1-%2")
        .arg(sliceDirectoryBase)
        .arg(printJob.getSelectedBaseLayerThickness());
    QString bodySliceDirectory = QString("%1-%2")
        .arg(sliceDirectoryBase)
        .arg(printJob.getSelectedBodyLayerThickness());

    _sliceStatus->setText("starting base layers");
    _imageGeneratorStatus->setText( "waiting");
    _setNavigationButtonsEnabled(false);

    TimingLogger::startTiming(TimingId::SlicingSvg, GetFileBaseName(printJob.getModelFilename()));

    bool basePresliced = _checkOneSliceDirectory(baseSliceDirectory, false);
    bool bodyPresliced = _checkOneSliceDirectory(bodySliceDirectory, true);

    SlicerTask *task { new SlicerTask(baseSliceDirectory, !basePresliced || _reslice,
        bodySliceDirectory, !bodyPresliced || _reslice) };
    QObject::connect(task, &SlicerTask::sliceStatus, this, &PrepareTab::slicingStatusUpdate);
    QObject::connect(task, &SlicerTask::renderStatus, this, &PrepareTab::renderingStatusUpdate);
    QObject::connect(task, &SlicerTask::layerCount, this, &PrepareTab::layerCountUpdate);
    QObject::connect(task, &SlicerTask::layerDone, this, &PrepareTab::layerDoneUpdate);
    QObject::connect(task, &SlicerTask::done, this, &PrepareTab::slicingDone);
    _threadPool.start(task);

    _setSliceControlsEnabled(false);
    emit uiStateChanged(TabIndex::Prepare, UiState::SliceStarted);

    update();
}

void PrepareTab::hasher_resultReady(QString const hash)
{
    debug(
        "+ PrepareTab::hasher_resultReady:\n"
        "  + result hash:           '%s'\n"
        "",
        hash.toUtf8( ).data( )
    );

    printJob.setModelHash(
        hash.isEmpty() ? QString("%1-%2").arg(time(nullptr)).arg(getpid()) : hash);

    _sliceStatus->setText("Idle");
    _hasher = nullptr;

    bool goodJobDir = _checkSliceDirectories();
    
    if (goodJobDir)
        _restartPreview();
        
    _updateSliceControls();
    update();
}

void PrepareTab::slicingStatusUpdate(const QString &status)
{
    _sliceStatus->setText(status);
    update();
}

void PrepareTab::renderingStatusUpdate(const QString &status)
{
    _imageGeneratorStatus->setText(status);
    update();
}

void PrepareTab::layerCountUpdate(int count)
{
    _navigateCurrentLabel->setText(QString("1/%1").arg(count));
    update();
}

void PrepareTab::layerDoneUpdate(int layer, QString path)
{
    _visibleLayer = layer;
    _navigateCurrentLabel->setText(QString("%1").arg(layer));
    _showLayerImage(path);
}

void PrepareTab::slicingDone(bool success)
{
    _setSliceControlsEnabled(true);

    if (success) {
        _checkSliceDirectories();
        _restartPreview();
        emit uiStateChanged(TabIndex::Prepare, UiState::SliceCompleted);
    } else {
        QMessageBox msgbox { QMessageBox::Icon::Critical, "Error", "Slicing error" };
        msgbox.exec();
    }
}

void PrepareTab::_loadDirectoryManifest()
{
    QSharedPointer<OrderManifestManager> manifestMgr { new OrderManifestManager() };
    QStringList errors;
    QStringList warnings;
    QString warningsStr;

    manifestMgr->setPath(printJob.getDirectoryPath());

    switch(manifestMgr->parse(&errors, &warnings))
    {
    case ManifestParseResult::POSITIVE_WITH_WARNINGS:
        warningsStr = warnings.join("<br>");
        _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually: " % warningsStr);
        /* FALLTHROUGH */

    case ManifestParseResult::POSITIVE:
        break;

    case ManifestParseResult::FILE_CORRUPTED:
    case ManifestParseResult::FILE_NOT_EXIST: {
            QString errorsStr = errors.join("<br>");
            _showWarning("Manifest file containing order of slices doesn't exist or file is corrupted. <br>You must enter the order manually. <br>" % errorsStr);

            manifestMgr->setTiled(false);
            manifestMgr->requireAreaCalculation();
            printJob.setBaseLayerCount(2);
            printJob.setSelectedBodyLayerThickness(DefaultBodyLayerThickness);
            printJob.setSelectedBaseLayerThickness(DefaultBaseLayerThickness);

            SlicesOrderPopup slicesOrderPopup { manifestMgr };
            slicesOrderPopup.exec();
            break;
        }
    }

    printJob.setBodyManager(manifestMgr);

    _orderButton->setEnabled(!manifestMgr->tiled());
    _setSliceControlsEnabled(false);

    layerCountUpdate(printJob.totalLayerCount());

    _restartPreview();
    emit uiStateChanged(TabIndex::Prepare, UiState::PrintJobReady);
}

void PrepareTab::_restartPreview()
{
    _visibleLayer = 0;
    _showLayerImage(_visibleLayer);

    if (printJob.totalLayerCount())
        _setNavigationButtonsEnabled(true);

    _adjustProjection->setEnabled(true);
}

void PrepareTab::_showWarning(const QString& content)
{
    QMessageBox msgBox { this } ;
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setText(content);
    msgBox.show();
    msgBox.exec();
}

void PrepareTab::prepareButton_clicked(bool)
{
    debug("+ PrepareTab::prepareButton_clicked\n");

    QObject::disconnect(_prepareButton, &QPushButton::clicked, this, nullptr);

    _prepareMessage->setText("Moving the build platform to its home location");
    _prepareProgress->show();

    _prepareButton->setText("Continue...");
    _prepareButton->setEnabled(false);

    QObject::connect(_shepherd, &Shepherd::action_homeComplete, this,
        &PrepareTab::shepherd_homeComplete);

    _shepherd->doHome();

    setPrinterAvailable(false);
    emit printerAvailabilityChanged(false);
    emit preparePrinterStarted();

    update();
}

void PrepareTab::shepherd_homeComplete( bool const success ) {
    debug( "+ PrepareTab::shepherd_homeComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    if ( !success ) {
        _handlePrepareFailed( );
        return;
    }

    _prepareMessage->setText( "Adjust the build platform position, then tap <b>Continue</b>." );

    QObject::disconnect( _prepareButton, &QPushButton::clicked, this, nullptr                                   );
    QObject::connect   ( _prepareButton, &QPushButton::clicked, this, &PrepareTab::adjustBuildPlatform_complete );
    _prepareButton->setEnabled( true );

    update( );
}

void PrepareTab::adjustBuildPlatform_complete( bool ) {
    debug( "+ PrepareTab::adjustBuildPlatform_complete\n" );

    QObject::disconnect( _prepareButton, &QPushButton::clicked, this, nullptr );
    _prepareButton->setEnabled( false );

    _prepareMessage->setText( "Raising the build platform" );
    _prepareProgress->show( );

    QObject::connect( _shepherd, &Shepherd::action_moveAbsoluteComplete, this, &PrepareTab::shepherd_raiseBuildPlatformMoveToComplete );
    _shepherd->doMoveAbsolute( PrinterRaiseToMaximumZ, PrinterDefaultHighSpeed );

    update( );
}

void PrepareTab::shepherd_raiseBuildPlatformMoveToComplete( bool const success ) {
    debug( "+ PrepareTab::shepherd_raiseBuildPlatformMoveToComplete: success: %s\n", success ? "true" : "false" );

    QObject::disconnect( _shepherd, nullptr, this, nullptr );
    _prepareProgress->hide( );

    QObject::connect( _prepareButton, &QPushButton::clicked, this, &PrepareTab::prepareButton_clicked );
    _prepareButton->setEnabled( true );

    if ( !success ) {
        _handlePrepareFailed( );
        return;
    }

    _prepareMessage->setText( "Preparation completed." );
    _prepareButton->setText( "Prepare..." );

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );
    emit preparePrinterComplete( true );

    update( );
}

void PrepareTab::_updateSliceControls() {
    _layerThickness100Button->setChecked(false);
    _layerThickness50Button->setChecked(false);
    _layerThicknessCustomButton->setChecked(false);
#if defined EXPERIMENTAL
    _layerThickness20Button->setChecked(false);
#endif

    if (printJob.getSelectedBaseLayerThickness() == printJob.getSelectedBodyLayerThickness() &&
        printJob.getBaseLayerCount() == 2) {
        switch (printJob.getSelectedBaseLayerThickness()) {
        case 100:
            _layerThickness100Button->setChecked(true);
            return;

        case 50:
            _layerThickness50Button->setChecked(true);
            return;

#if defined EXPERIMENTAL
        case 20:
            _layerThickness20Button->setChecked(true);
            return;
#endif
        }
    }

    //_layerThicknessCustomButton->setChecked(true);
}

void PrepareTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ PrepareTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch (_uiState) {
    case UiState::SelectCompleted:

        if (!printJob.getDirectoryMode()) {
            _layerThickness100Button->click();

            printJob.setBaseLayerCount(printJob.getBaseLayerCount());
            printJob.setSelectedBaseLayerThickness(printJob.baseLayerParameters().layerThickness());
            printJob.setSelectedBodyLayerThickness(printJob.bodyLayerParameters().layerThickness());

            _setSliceControlsEnabled(false);
            _sliceStatus->setText("idle");
            _imageGeneratorStatus->setText("idle");
            _currentLayerImage->clear();
            _navigateCurrentLabel->setText("0/0");
            _setNavigationButtonsEnabled(false);
            _initAfterSelect = true;

            if (_hasher)
                _hasher->deleteLater();

            _hasher = new Hasher;
            QObject::connect(_hasher, &Hasher::resultReady, this, &PrepareTab::hasher_resultReady, Qt::QueuedConnection);
            _hasher->hash(printJob.getModelFilename(), QCryptographicHash::Md5);

        } else {
            _setSliceControlsEnabled(false);
            _loadDirectoryManifest();
            _updateSliceControls();
        }
        break;

    case UiState::SliceStarted:
        _setSliceControlsEnabled(false);
        break;

    case UiState::SliceCompleted:
        if (!printJob.getDirectoryMode())
            _setSliceControlsEnabled(true);
        break;

    case UiState::PrintStarted:
        _setSliceControlsEnabled(false);
        setPrinterAvailable(false);
        _orderButton->setEnabled(false);
        emit printerAvailabilityChanged(false);
        break;

    case UiState::PrintCompleted:
        _setSliceControlsEnabled(!printJob.getDirectoryMode() && !printJob.isTiled());
        _orderButton->setEnabled(printJob.getDirectoryMode());
        setPrinterAvailable(true);
        emit printerAvailabilityChanged(true);
        break;

    default:
        break;
    }

    update();
}

void PrepareTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ PrepareTab::printer_online: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ PrepareTab::printer_offline: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::printer_temperatureReport( double const bedCurrentTemperature, double const, int const ) {
    if ( bedCurrentTemperature >= 30.0 ) {
        _warningHotLabel->setPixmap( *_warningHotImage );
    } else {
        _warningHotLabel->clear( );
    }

    update( );
}

void PrepareTab::printManager_lampStatusChange( bool const on ) {
    if ( on ) {
        _warningUvLabel->setPixmap( *_warningUvImage );
    } else {
        _warningUvLabel->clear( );
    }

    update( );
}

void PrepareTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ PrepareTab::setPrinterAvailable: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updatePrepareButtonState( );
}

void PrepareTab::usbMountManager_filesystemMounted( QString const& mountPoint ) {
    debug( "+ PrepareTab::usbMountManager_filesystemMounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

    if ( !_usbPath.isEmpty( ) ) {
        debug( "  + We already have a USB storage device at '%s' mounted; ignoring new mount\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    QFileInfo usbPathInfo { mountPoint };
    if ( !usbPathInfo.isReadable( ) || !usbPathInfo.isExecutable( ) ) {
        debug( "  + Unable to access mount point '%s' (uid: %u; gid: %u; mode: 0%03o)\n", _usbPath.toUtf8( ).data( ), usbPathInfo.ownerId( ), usbPathInfo.groupId( ), usbPathInfo.permissions( ) & 07777 );
        return;
    }

    _usbPath = mountPoint;

    update( );
}

void PrepareTab::usbMountManager_filesystemUnmounted( QString const& mountPoint ) {
    debug( "+ PrepareTab::usbMountManager_filesystemUnmounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );

    if ( mountPoint != _usbPath ) {
        debug( "  + not our filesystem; ignoring\n", _usbPath.toUtf8( ).data( ) );
        return;
    }

    _usbPath.clear( );

    update( );
}

void PrepareTab::printJobChanged() {
    connect(&printJob, &PrintJob::printOffsetChanged, this, [this](QPoint offset) {
        if(offset.x() != 0 || offset.y() != 0) {
            _printOffsetLabel->setText(QString("offset (%1, %2)").arg(offset.x()).arg(offset.y()));
            _adjustValue->setText(QString("%1, %2").arg(offset.x()).arg(offset.y()));
        } else {
            _printOffsetLabel->setText(QString(""));
            _adjustValue->setText(QString("0, 0").arg(offset.x()).arg(offset.y()));
        }
    });

    if(_adjustProjection->isChecked()) {
        _adjustProjection->click();
    }

#if defined EXPERIMENTAL
    _adjustProjection->setEnabled(false);
    _adjustGroup->setVisible(false);
#endif
}
void PrepareTab::setPngDisplayer( PngDisplayer* pngDisplayer ) {
    _pngDisplayer = pngDisplayer;
}

void PrepareTab::activeProfileChanged(QSharedPointer<PrintProfile> newProfile) {
    (void)newProfile;
    QPoint offset = printJob.getPrintOffset();

    if(offset.x() != 0 || offset.y() != 0) {
        _printOffsetLabel->setText(QString("offset (%1, %2)").arg(offset.x()).arg(offset.y()));
        _adjustValue->setText(QString("%1, %2").arg(offset.x()).arg(offset.y()));
    } else {
        _printOffsetLabel->setText(QString(""));
        _adjustValue->setText(QString("0, 0").arg(offset.x()).arg(offset.y()));
    }

    if(_adjustProjection->isChecked()) {
        _adjustProjection->click();
    }
}
