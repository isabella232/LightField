#include "tilingtab.h"
#include "tilingmanager.h"
#include "window.h"

TilingTab::TilingTab( QWidget* parent ): TabBase( parent ) {
    auto origFont    = font( );
    auto boldFont    = ModifyFont( origFont, QFont::Bold );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome", LargeFontSize );


    QGroupBox* all { new QGroupBox };

    QVBoxLayout* _currentLayerLayout;
    _currentLayerImage->setAlignment( Qt::AlignCenter );
    _currentLayerImage->setContentsMargins( { } );
    _currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerImage->setStyleSheet( "QWidget { background: black }" );
    _currentLayerImage->setMinimumSize( MaximalRightHandPaneSize );
    _currentLayerImage->setFixedSize( _currentLayerImage->width( ), _currentLayerImage->width( ) / AspectRatio16to10 + 0.5 );

    _currentLayerLayout = WrapWidgetsInVBox(
        _currentLayerImage
    );
    _currentLayerLayout->setAlignment( Qt::AlignTop | Qt::AlignHCenter );


    _confirm->setFixedSize( MainButtonSize );
    _confirm->setFont( fontAwesome );
    _confirm->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _confirm->setEnabled( false );

    all->setLayout(
        WrapWidgetsInVBox(
           nullptr,
           _minExposure,
           _step,
           _space,
           _count,
           nullptr,
           _confirm
        )
    );

    QObject::connect( _minExposure, &ParamSlider::valuechanged, this, &TilingTab::setStepValue );
    QObject::connect( _step, &ParamSlider::valuechanged, this, &TilingTab::setStepValue );
    QObject::connect( _space, &ParamSlider::valuechanged, this, &TilingTab::setStepValue );
    QObject::connect( _count, &ParamSlider::valuechanged, this, &TilingTab::setStepValue );
    QObject::connect( _confirm, &QPushButton::clicked, this, &TilingTab::confirmButton_clicked );


    all->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Expanding );

    setLayout( WrapWidgetsInHBox( all, _currentLayerImage ) );

    update( );
}

void TilingTab::setStepValue()
{
    debug( "+ ProfilesTab::setStepValue\n");

    if(_manifestManager->getFirstElement() == nullptr)
        return;

    auto area = QPixmap( _currentLayerImage->width( ), _currentLayerImage->height( ) );
    int maxCount = _getMaxCount( );

    _count->setMaxValue( maxCount );

    int wCount = _count->getValue();
    int spacePx = ( (double)_space->getValue() ) / ProjectorPixelSize * _wRatio;

    if( maxCount < 1 ) {
        _showWarningAndClose();
        _space->setEnabled( false );

        return;
    } else {
        _space->setEnabled( true );
    }

    //int hCount =  floor( (_currentLayerImage->height( ) - pixmap.height() * value)  / (pixmap.height() + pixmap.height() * value) );

    QPainter painter ( &area );

    painter.fillRect(0,0, _currentLayerImage->width( ), _currentLayerImage->height( ), QBrush("#000000"));


    painter.setFont( QFont( "Arial", 15 ) );
    painter.setPen( Qt::red );

    // multi row tilling
    /*for(int i=0,z=1; i<wCount; ++i) {
        for(int j=0; j<hCount; ++j,++z)
        {
                            /*margin*/                /* image */                 /* space */
    /*      int x = ( pixmap.width( ) * value) + ( pixmap.width( ) * i ) + ( pixmap.width( ) * value * i );
            int y = ( pixmap.height( ) * value) + ( pixmap.height( ) * j )  + ( pixmap.height( ) * value * j );

            int e = _minExposure->getValue() + ( ((wCount*hCount) - z) * _step->getValue() );

            painter.drawPixmap( x, y, pixmap );
            painter.drawText( QPoint(x, y), QString( "Exposure %1 sec" ).arg( e ) );
        }
    }*/


    // single row tiling
    int y = ( _areaHeight - _pixmapHeight ) / 2;

    for(int i=0; i<wCount; ++i) {
        int x = TilingMargin + ( _pixmapWidth * i ) + ( spacePx * i );

        int e = _minExposure->getValue() + ( ( wCount - ( i + 1 ) ) * _step->getValue( ) );

        painter.drawPixmap( x, y, *_pixmap );
        painter.drawText( QPoint(x, y), QString( "Exposure %1 sec" ).arg( e ) );
    }

    _currentLayerImage->setPixmap( area );

    update( );
}

void TilingTab::_showLayerImage( ) {
    debug( "+ ProfilesTab::_showLayerImage");

    setStepValue ();

    update( );
}



void TilingTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ ProfilesTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( _uiState ) {
        case UiState::SelectStarted:
        case UiState::SliceStarted:
        case UiState::SliceCompleted:
        case UiState::PrintStarted:
        case UiState::PrintCompleted:
        case UiState::TilingClicked:
            break;
        case UiState::SelectedDirectory:
        case UiState::SelectCompleted:
            this->_step->setValue( 2 );
            //this->_space->setValueDouble( 0.25f );
            this->_minExposure->setValue( 2 );
            this->_printJob = nullptr;
            this->_manifestManager = nullptr;
            this->_currentLayerImage->clear();
            this->_confirm->setEnabled( false );
            this->_step->setEnabled( false );
            this->_space->setEnabled( false );
            this->_minExposure->setEnabled( false );
            this->_count->setEnabled( false );
    }

    update( );
}


void TilingTab::confirmButton_clicked ( bool ) {
    TilingManager* tilingMgr = new  TilingManager( _manifestManager, _printJob );

    QDialog* dialog = new QDialog();
    Window* w = App::mainWindow();
    QRect r = w->geometry();

    dialog->setModal( true );
    dialog->setLayout( WrapWidgetsInHBox ( nullptr, new QLabel ( "Processing files ... <br>please wait" ), nullptr ) );

    dialog->resize(300, 100);
    dialog->setContentsMargins( { } );
    dialog->setMinimumSize(QSize(300, 100));
    dialog->show();

    dialog->move(r.x() + ((r.width() - dialog->width())/2), r.y() + ((r.height() - dialog->height())/2) );



    QThread *thread = QThread::create(
        [this, tilingMgr, dialog]
        {
            tilingMgr->processImages( ProjectorWindowSize.width(),
                                      ProjectorWindowSize.height(),
                                     _minExposure->getValue(),
                                     _step->getValue(),
                                     _space->getValue(),
                                     _count->getValue() );

            _printJob->jobWorkingDirectory = tilingMgr->getPath();

            emit uiStateChanged( TabIndex::Tiling, UiState::SelectedDirectory );

            dialog->done(0);
            delete dialog;
            delete tilingMgr;

        }
    );
    thread->start();
    dialog->exec();
}

int TilingTab::_getMaxCount()
{
    int wCount=0;
    int space = ((double)_space->getValue()) / ProjectorPixelSize * _wRatio;

    for (
         int i = ( TilingMargin * _wRatio );
         i < ( _areaWidth - ( TilingMargin * _wRatio ) );
         i += _pixmapWidth, wCount++
    ) {
        if(wCount>0)
            i+=space;

        debug( " i: %d wCount: %d \n", i, wCount );
    }

    wCount--;

    return wCount;
}

void TilingTab::_showWarningAndClose ()
{
    auto origFont    = font( );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome" );

    Window* w = App::mainWindow();
    QRect r = w->geometry();

    QMessageBox msgBox;
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setFont(fontAwesome);
    msgBox.setText( "Slices are too wide to be tiled." );
    msgBox.show();
    msgBox.move(r.x() + ((r.width() - msgBox.width())/2), r.y() + ((r.height() - msgBox.height())/2) );
    msgBox.exec();
}
