#include "slicesorderpopup.h"
#include "window.h"

SlicesOrderPopup::SlicesOrderPopup(OrderManifestManager* manifestManager)
{
    auto origFont    = font( );
    auto normalFont = ModifyFont( origFont, "FontAwesome", NormalFontSize );
    auto fontAwesome = ModifyFont( origFont, "FontAwesome", LargeFontSize );
    _manifestManager = manifestManager;

    Window* win = App::mainWindow();
    QRect r = win->geometry();
    move( r.x( )+100, r.y( )+100 );
    resize( 824, 400 );

    _okButton->setFont( fontAwesome );
    _okButton->setMinimumSize( QSize( 50, 50 ) );
    _list->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    _arrowUp->setFont( fontAwesome );
    _arrowUp->setMinimumSize( QSize ( 50, 50 ) );

    _arrowDown ->setFont( fontAwesome );
    _arrowDown->setMinimumSize( QSize ( 50, 50 ) );

    _alphaNum->setFont( fontAwesome );
    _numerical->setFont( fontAwesome );
    _custom->setFont( fontAwesome );



    if( _manifestManager->initialized() ) {
        switch(manifestManager->sortType().intVal()) {
        case ManifestSortType::ALPHANUMERIC:
            _alphaNum->setChecked(true);
            break;
        case ManifestSortType::NUMERIC:
            _numerical->setChecked(true);
            break;
        case ManifestSortType::CUSTOM:
            _custom->setChecked(true);
            break;
        }
    }
    else
    {
        _numerical->setChecked( true );
    }

    QGroupBox* sortGB = new QGroupBox( "Sort type" );
    sortGB->setLayout( WrapWidgetsInVBox(
       _alphaNum,
       _numerical,
       _custom
    ) );

    QGroupBox* leftMenu = new QGroupBox( );
    leftMenu->setLayout(
        WrapWidgetsInVBox(
          _arrowUp,
          _arrowDown,
          nullptr,
          sortGB,
          nullptr,
          _okButton
        )
    );

    _list->setContentsMargins(0,0,0,0);
    _list->setFont( fontAwesome );
    _list->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _list->setMinimumSize( QSize ( 600, 350 ) );
    setMinimumSize( QSize( 580, 355 ) );

    QLabel* titleBar = new QLabel("Slices order editor");
    titleBar->setStyleSheet("QLabel { background-color : #105c72; color : #ededed; }");
    titleBar->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
    titleBar->setFont( normalFont );
    titleBar->setContentsMargins(0,0,0,0);

    setLayout(
        WrapWidgetsInVBox(
            titleBar,
            WrapWidgetsInHBox(
                  leftMenu,
                  _list
            )
        )
    );

    _list->setModel(_model);
    _list->setStyleSheet("QAbstractItemView::indicator { width: 20px;height:20px;/*size of checkbox change here */} QTableWidget::item{width: 200px;height: 100px;} "/*size of item */);

    fillModel( );

    _list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    _model->setHeaderData(0, Qt::Horizontal, tr("File name"));
    _model->setHeaderData(1, Qt::Horizontal, tr("Attach"));

    setModal( true );

    QWidget::connect(_okButton,  &QPushButton::clicked,  this, &SlicesOrderPopup::oklClicked_clicked);
    QWidget::connect(_arrowUp,   &QPushButton::clicked,  this, &SlicesOrderPopup::arrowUp_clicked);
    QWidget::connect(_arrowDown, &QPushButton::clicked,  this, &SlicesOrderPopup::arrowDown_clicked);
    QWidget::connect(_alphaNum,  &QRadioButton::clicked, this, &SlicesOrderPopup::alphaNum_clicked);
    QWidget::connect(_numerical, &QRadioButton::clicked, this, &SlicesOrderPopup::numerical_clicked);
    QWidget::connect(_custom,    &QRadioButton::clicked, this, &SlicesOrderPopup::custom_clicked);
}

void SlicesOrderPopup::fillModel( ) {
    debug( "+ SlicesOrderPopup::fillModel \n" );

    QDirIterator iter { _manifestManager->path(), QDir::Files };
    debug( "+ SlicesOrderPopup::fillModel iterating over files\n" );
    while( iter.hasNext( ) )
    {
        QString fileName = iter.next( );
        debug( "+ SlicesOrderPopup::fillModel %s\n",  fileName.toUtf8().data() );


        QStandardItem* fileNameCol = new QStandardItem { GetFileBaseName ( fileName ) };
        QStandardItem* checkBoxCol = new QStandardItem;


        fileNameCol->setEditable( false );

        // Checkable item
        checkBoxCol->setCheckable( true );

        bool initialized = _manifestManager->initialized();
        bool contains = _manifestManager->contains( GetFileBaseName ( fileName ) );
        bool isPng = fileName.endsWith(QString("png"), Qt::CaseInsensitive);

        // Save checke state
        if( (initialized && contains) || (!initialized && isPng) ) {
            checkBoxCol->setData(Qt::Checked, Qt::CheckStateRole);
        } else {
            checkBoxCol->setData(Qt::Unchecked, Qt::CheckStateRole);
        }

        QList<QStandardItem*> row { fileNameCol, checkBoxCol  };
        _model->appendRow( row );

        _model->setColumnCount(2);
    }

    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::oklClicked_clicked( bool ) {
    debug( "+ SlicesOrderPopup::oklClicked_clicked reading element \n" );
    if( _alphaNum->isChecked() )
        _manifestManager->setSortType( ManifestSortType::ALPHANUMERIC );
    else if ( _numerical->isChecked() )
        _manifestManager->setSortType( ManifestSortType::NUMERIC );
    else if ( _custom->isChecked() )
        _manifestManager->setSortType( ManifestSortType::CUSTOM );

    QStringList filenames { };

    for(int i=0; i<_model->rowCount(); ++i)
    {
        QString item = _model->item( i )->text();
        if ( _model->item( i, 1)->checkState() == Qt::CheckState::Checked )
            filenames.push_back( item );
    }

    _manifestManager->setFileList( filenames );
    _manifestManager->save();

    this->setResult( QDialog::Accepted );
    this->accept( );
    this->close( );
}

void SlicesOrderPopup::arrowUp_clicked(bool) {
    debug( "+ SlicesOrderPopup::arrowUp_clicked \n" );
    int currIdx = _list->currentIndex( ).row( );

    if(currIdx <= 0)
        return;

    int nextRow = currIdx - 1;

    auto currRow = _model->takeRow( currIdx );
    _model->removeRows( currIdx, 0 );
    _model->insertRow( nextRow, currRow );

    QModelIndex indexOfTheCellIWant = _model->index( nextRow, 0 );
    _list->setCurrentIndex( indexOfTheCellIWant );
    _custom->setChecked(true);
}

void SlicesOrderPopup::arrowDown_clicked(bool) {
    debug( "+ SlicesOrderPopup::arrowDown_clicked \n" );
    int currIdx = _list->currentIndex( ).row( );

    if(currIdx == _model->rowCount()-1)
        return;

    int nextRow = currIdx + 1;

    auto currRow = _model->takeRow( currIdx );
    _model->removeRows( currIdx, 0 );
    _model->insertRow( nextRow, currRow );

    QModelIndex indexOfTheCellIWant = _model->index( nextRow, 0 );
    _list->setCurrentIndex( indexOfTheCellIWant );
    _custom->setChecked(true);
}

void SlicesOrderPopup::alphaNum_clicked(bool) {
    debug( "+ SlicesOrderPopup::alphaNum_clicked \n" );

    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::numerical_clicked(bool) {
    debug( "+ SlicesOrderPopup::numerical_clicked \n" );

    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::custom_clicked(bool) {

    debug( "+ SlicesOrderPopup::custom_clicked \n" );
}


