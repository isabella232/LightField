#include "slicesorderpopup.h"
#include "window.h"

SlicesOrderPopup::SlicesOrderPopup(OrderManifestManager* manifestManager)
{
    auto origFont    = font( );
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

    _numerical->setChecked( true );

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

    _list->setFont( fontAwesome );
    _list->setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Maximum );
    _list->setMinimumSize( QSize ( 600, 400 ) );
    setMinimumSize( QSize( 580, 355 ) );

    setLayout(
        WrapWidgetsInHBox(
              leftMenu,
              _list
        )
    );

    _list->setModel(_model);
    fillModel( );

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

    QDirIterator iter { _manifestManager->path(), QStringList() << "*.png", QDir::Files };

    debug( "+ SlicesOrderPopup::fillModel adding column\n" );
    //QList<QStandardItem*> columns {};
    //columns.append( new QStandardItem( ) );

    //_model->appendColumn( columns );

    debug( "+ SlicesOrderPopup::fillModel iterating over files\n" );
    while( iter.hasNext( ) )
    {
        QString fileName = iter.next( );
        debug( "+ SlicesOrderPopup::fillModel %s\n",  fileName.toUtf8().data() );
        _model->appendRow( new QStandardItem( GetFileBaseName ( fileName ) ) );
    }

    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::oklClicked_clicked(bool) {
    this->setResult(QDialog::Accepted);
    this->accept();
    this->close();
}

void SlicesOrderPopup::arrowUp_clicked(bool) {
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
    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::numerical_clicked(bool) {
    _model->sort(0, Qt::AscendingOrder);
}

void SlicesOrderPopup::custom_clicked(bool) {

}
