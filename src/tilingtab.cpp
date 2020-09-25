#include <QtCore>
#include <QtWidgets>
#include "progressdialog.h"
#include "utils.h"
#include "tilingtab.h"
#include "tilingmanager.h"
#include "printmanager.h"
#include "window.h"
#include "printprofilemanager.h"

TilingExpoTimePopup::TilingExpoTimePopup(QWidget* parent): QDialog(parent)
{
    auto origFont = font();
    auto normalFont = ModifyFont(origFont, "FontAwesome", NormalFontSize );
    auto fontAwesome = ModifyFont(origFont, "FontAwesome", LargeFontSize);
    auto font22pt = ModifyFont(origFont, LargeFontSize);
    auto boldFont = ModifyFont( font( ), QFont::Bold );

    this->setModal(true);
    this->setFixedWidth(MainWindowSize.width());

    QLabel* addition1 {new QLabel("+")};
    QLabel* addition2 {new QLabel("+")};
    QLabel* eq1 {new QLabel("=")};
    QLabel* eq2 {new QLabel("=")};
    QFrame* hr {new QFrame};

    auto boldFontBigger = ModifyFont( boldFont, 13);
    _advBodyLbl->setFixedWidth(82);
    _advBodyLbl->setFont(boldFontBigger);
    _advBaseLbl->setFixedWidth(82);
    _advBaseLbl->setFont(boldFontBigger);


    addition1->setFont(boldFontBigger);
    addition2->setFont(boldFontBigger);
    eq1->setFont(boldFontBigger);
    eq1->setFont(boldFontBigger);

    hr->setFrameShape(QFrame::HLine);
    hr->setFrameShadow(QFrame::Sunken);
    hr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hr->setFixedHeight(1);

    _advBodyExpoCorse->setCounterBold(false);
    _advBaseExpoCorse->setCounterBold(false);
    _advBodyExpoFine->setCounterBold(false);
    _advBaseExpoFine->setCounterBold(false);

    _advBodyExpoFine->setFixedWidth(285);
    _advBaseExpoFine->setFixedWidth(285);


    QVBoxLayout* container =
        WrapWidgetsInVBox(
          WrapWidgetsInHBox(_advBaseExpoCorse, addition2, _advBaseExpoFine, eq2, _advBaseLbl),
          _stepBase,
          hr,
          WrapWidgetsInHBox(_advBodyExpoCorse, addition1, _advBodyExpoFine, eq1, _advBodyLbl),
          _stepBody
        );

    QObject::connect(_advBodyExpoCorse, &ParamSlider::valueChanged, this, &TilingExpoTimePopup::updateExposureTimes);
    QObject::connect(_advBodyExpoFine, &ParamSlider::valueChanged, this, &TilingExpoTimePopup::updateExposureTimes);
    QObject::connect(_advBaseExpoCorse, &ParamSlider::valueChanged,  this, &TilingExpoTimePopup::updateExposureTimes);
    QObject::connect(_advBaseExpoFine, &ParamSlider::valueChanged, this, &TilingExpoTimePopup::updateExposureTimes);

    QObject::connect(_okButton, &QPushButton::clicked, this, &TilingExpoTimePopup::confirm);
    QObject::connect(_cancelButton, &QPushButton::clicked, this, &TilingExpoTimePopup::cancel);

    setLayout(
        WrapWidgetsInVBox(
            container,
            nullptr,
            WrapWidgetsInHBox(_okButton, _cancelButton)
        )
    );
}

void TilingExpoTimePopup::confirm(bool)
{
    this->setResult(QDialog::Accepted);
    this->accept();
    this->close();
}

void TilingExpoTimePopup::cancel(bool)
{
    this->setResult(QDialog::Rejected);
    this->reject();
    this->close();
}

void TilingExpoTimePopup::updateExposureTimes()
{
    int bodyExpoTime = _advBodyExpoCorse->getValue() + _advBodyExpoFine->getValue();
    int baseExpoTime = _advBaseExpoCorse->getValue() + _advBaseExpoFine->getValue();

    _advBodyLbl->setText( QString("%1 s").arg(bodyExpoTime / 1000.0));
    _advBaseLbl->setText( QString("%1 s").arg(baseExpoTime / 1000.0));
}

TilingTab::TilingTab(QWidget* parent): TabBase(parent)
{
    auto origFont = font();
    auto boldFont = ModifyFont(origFont, QFont::Bold);
    auto fontAwesome = ModifyFont(origFont, "FontAwesome", LargeFontSize);

    QGroupBox* all {new QGroupBox};

    QVBoxLayout* _currentLayerLayout;
    _currentLayerImage->setAlignment(Qt::AlignCenter);
    _currentLayerImage->setContentsMargins({});
    _currentLayerImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _currentLayerImage->setStyleSheet("QWidget { background: black }");
    _currentLayerImage->setMinimumSize(MaximalRightHandPaneSize);
    _currentLayerImage->setFixedSize(_currentLayerImage->width(),
        static_cast<int>(_currentLayerImage->width() / AspectRatio16to10 + 0.5));

    _setupTiling->setEnabled(false);
    _setupTiling->setMinimumWidth(MainButtonSize.width());
    _setupTiling->setText("Setup tiling");
    QObject::connect(_setupTiling, &QPushButton::clicked, this, &TilingTab::setupTilingClicked);

    _currentLayerLayout = WrapWidgetsInVBox(
        _currentLayerImage
    );
    _currentLayerLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    _confirm->setMinimumWidth(MainButtonSize.width());
    _confirm->setEnabled(false);

    _setupExpoTimeBt->setMinimumWidth(MainButtonSize.width());

    QGroupBox* lrInfo = new QGroupBox();
    QGroupBox* baseLrInfo = new QGroupBox("Base Layer Times");
    QGroupBox* bodyLrInfo = new QGroupBox("Body Layer Times");

    auto fontBoldBigger = boldFont;
    fontBoldBigger.setPointSize(14);

    baseLrInfo->setFont(fontBoldBigger);
    bodyLrInfo->setFont(fontBoldBigger);

    baseLrInfo->setLayout(
        WrapWidgetsInVBox(
            _minExposureBaseLabel,
            _stepBaseLabel
        )
    );

    bodyLrInfo->setLayout(
        WrapWidgetsInVBox(
            _minExposureBodyLabel,
            _stepBodyLabel
        )
    );

    lrInfo->setLayout(
        WrapWidgetsInVBox(baseLrInfo, nullptr, bodyLrInfo)
    );

    all->setLayout(
        WrapWidgetsInVBox(
           _setupTiling,
           _space,
           _count,
           nullptr,
           _setupExpoTimeBt,
           _confirm,
           nullptr,
           lrInfo,
           nullptr
        )
    );

    QObject::connect(_space, &ParamSlider::valueChanged, this, &TilingTab::setStepValue);
    QObject::connect(_count, &ParamSlider::valueChanged, this, &TilingTab::setStepValue);
    QObject::connect(_setupExpoTimeBt, &QPushButton::clicked, this,
        &TilingTab::setupExpoTimeClicked);
    QObject::connect(_confirm, &QPushButton::clicked, this, &TilingTab::confirmButton_clicked);

    all->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    _fileNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    setLayout(WrapWidgetsInHBox(all, WrapWidgetsInVBox(nullptr, _currentLayerImage,_fileNameLabel)));

    _setEnabled(false);
    update();
}

void TilingTab::_connectPrintManager()
{

    if (_printManager) {
        QObject::connect(_printManager, &PrintManager::printStarting, this,
           &TilingTab::printManager_printStarting);
        QObject::connect(_printManager, &PrintManager::printComplete, this,
           &TilingTab::printManager_printComplete);
        QObject::connect(_printManager, &PrintManager::printAborted, this,
           &TilingTab::printManager_printAborted);
    }
}

void TilingTab::printManager_printStarting()
{

    debug("+ TilingTab::printManager_printStarting\n");

    _setupTiling->setEnabled(false);

    update();
}

void TilingTab::printManager_printComplete(const bool success)
{

    debug("+ TilingTab::printManager_printComplete\n");
    (void)success;

    _setupTiling->setEnabled(!printJob.isTiled());

    update();
}

void TilingTab::printManager_printAborted()
{

    debug("+ TilingTab::printManager_printAborted\n");

    _setupTiling->setEnabled(!printJob.isTiled());

    update();
}

void TilingTab::setStepValue()
{
    debug("+ TilingTab::setStepValue\n");

    auto area = QPixmap(_currentLayerImage->width(), _currentLayerImage->height());
    int maxCount = _getMaxCount();
    debug("+ TilingTab::setCount %d\n", maxCount);
    _count->setMaxValue(maxCount);

    int wCount = _count->getValue();
    int spacePx = static_cast<int>(_space->getValue() / ProjectorPixelSize * _wRatio);

    if (maxCount < 1) {
        _showWarningAndClose();
        _space->setEnabled(false);

        return;
    } else
        _space->setEnabled(true);

    //int hCount =  floor( (_currentLayerImage->height( ) - pixmap.height() * value)  / (pixmap.height() + pixmap.height() * value) );

    QPainter painter(&area);

    painter.fillRect(0,0, _currentLayerImage->width(), _currentLayerImage->height(),
        QBrush("#000000"));

    painter.setFont(QFont("Arial", 12, 2));
    painter.setPen(Qt::red);

#if 0
    // multi row tilling
    for (int i=0,z=1; i<wCount; ++i) {
        for (int j=0; j<hCount; ++j, ++z)
        {
                            /*margin*/                /* image */                 /* space */
          int x = ( pixmap.width( ) * value) + ( pixmap.width( ) * i ) + ( pixmap.width( ) * value * i );
            int y = ( pixmap.height( ) * value) + ( pixmap.height( ) * j )  + ( pixmap.height( ) * value * j );

            int e = _minExposure->getValue() + ( ((wCount*hCount) - z) * _step->getValue() );

            painter.drawPixmap( x, y, pixmap );
            painter.drawText( QPoint(x, y), QString( "Exposure %1 sec" ).arg( e ) );
        }
    }
#endif

    // single row tiling
    int y = (_areaHeight - _pixmapHeight) / 2;

    // 3 mm Y offset for first element
    int deltaY = static_cast<int>((3 / ProjectorPixelSize) * _hRatio);
    std::vector<int> tileSlots;

    int deltax = (_areaWidth - (wCount * _pixmapWidth) - (wCount - 1) * spacePx) / 2 - TilingMargin;

    for (int i = 0; i < wCount; ++i) {
        int x1 = TilingMargin + (_pixmapWidth * i) + (spacePx * i);
        tileSlots.push_back(x1 + deltax);
    }

    std::vector<int> tileSlotsText = tileSlots;

    std::rotate(tileSlots.begin(),
                tileSlots.end()-1, // this will be the new first element
                tileSlots.end());

    std::reverse(tileSlotsText.begin(),tileSlotsText.end());
    std::rotate(tileSlotsText.begin(),
                tileSlotsText.end()-1, // this will be the new first element
                tileSlotsText.end());

    for (int i = 0; i < wCount; ++i) {
        int x = tileSlots[static_cast<unsigned int>(i)];
        int text_x = tileSlotsText[static_cast<unsigned int>(i)];

        double minExposureBase = _minExposureBase;
        double stepBase = _stepBase;
        double minExposureBody = _minExposureBody;
        double stepBody = _stepBody;

        double eBase = minExposureBase + ((wCount - (i + 1)) * stepBase);
        double eBody = minExposureBody + ((wCount - (i + 1)) * stepBody);

        if (i == 0) {
            painter.drawPixmap(x, y + deltaY, *_pixmap);
             _renderText(&painter, QPoint(text_x, y - deltaY), eBase, eBody);
        } else {
            painter.drawPixmap(x, y, *_pixmap);
             _renderText(&painter, QPoint(text_x, y), eBase, eBody);
        }
    }

    QTransform rotate_transform;
    QPixmap rotated_area;

    rotate_transform.rotate(180);
    rotated_area = area.transformed(rotate_transform);

    _currentLayerImage->setPixmap(rotated_area);
    update();
}

void TilingTab::_renderText(QPainter* painter, QPoint pos, double expoBase, double expoBody)
{
    QFontMetrics fm(painter->font());
    QString baseText = QString("Base %1s").arg(expoBase);
    QString bodyText = QString("Body %2s").arg(expoBody);
    int textHeight = fm.height();

    painter->scale(-1,-1);
    painter->translate(-_currentLayerImage->width(), -_currentLayerImage->height());
    painter->drawText(QPoint(pos.x(), pos.y() - textHeight), baseText);
    painter->drawText(QPoint(pos.x(), pos.y() - (0.25*textHeight)), bodyText);
    painter->scale(-1,-1);
    painter->translate(-_currentLayerImage->width(), -_currentLayerImage->height());
}

void TilingTab::_showLayerImage()
{
    debug("+ TilingTab::_showLayerImage");

    setStepValue();

    update();
}

void TilingTab::tab_uiStateChanged(TabIndex const sender, UiState const state)
{
    debug("+ TilingTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString(sender),
        ToString(_uiState), ToString(state));
    _uiState = state;

    switch (state) {
    case UiState::SelectCompleted:
        _setEnabled(false);
        _setupTiling->setEnabled(false);

        if (printJob.getDirectoryMode()) {
            _updateExposureTiming();

            this->_space->setValue(1);
            this->_count->setValue(1);
            this->_currentLayerImage->clear();


            _minExposureBaseLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBase));
            _stepBaseLabel->setText(QString("%1s Exposure Step").arg(_stepBase));
            _minExposureBodyLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBody));
            _stepBodyLabel->setText(QString("%1s Exposure Step").arg(_stepBody));

            _setEnabled(false);
            _setupTiling->setEnabled(false);
        }
        break;

    case UiState::SelectStarted:
        _setEnabled(false);
        break;

    case UiState::SliceStarted:
        _setEnabled(false);
        break;

    case UiState::PrintStarted:
        _setEnabled(false);
        break;

    case UiState::PrintJobReady:
        if (!_printManager->isRunning())
            _setupTiling->setEnabled(!printJob.isTiled());
        _setEnabled(false);
        break;

    case UiState::TilingClicked:
        _setEnabled(!printJob.isTiled());
        _setupTiling->setEnabled(false);
        setStepValue();
        break;

    default:
        break;
    }

    update();
}


void TilingTab::confirmButton_clicked(bool)
{
    debug("+ TilingTab::confirmButton_clicked\n");

    TilingManager* tilingMgr = new TilingManager();
    ProgressDialog* dialog = new ProgressDialog(this);

    QObject::connect(tilingMgr, &TilingManager::statusUpdate, dialog, &ProgressDialog::setMessage);
    QObject::connect(tilingMgr, &TilingManager::progressUpdate, dialog,
        &ProgressDialog::setProgress);

    QSharedPointer<OrderManifestManager> orderMgr;
    dialog->show();

    QString modelFilename = printJob.getModelFilename();

    QThread *thread = QThread::create(
        [this, tilingMgr, dialog, &orderMgr]
        {
            OrderManifestManager* orderMgrPtr = tilingMgr->processImages(ProjectorWindowSize.width(), ProjectorWindowSize.height(),
                _minExposureBase, _stepBase, _minExposureBody, _stepBody, _space->getValue(),
                _count->getValue());


            orderMgr.reset(orderMgrPtr);
            dialog->close();
            delete dialog;
        }
    );

    thread->start();

    dialog->exec();

    printJob = PrintJob(_printProfileManager->activeProfile());

    printJob.setModelFilename(modelFilename);
    printJob.setSelectedBaseLayerThickness(-1);
    printJob.setSelectedBodyLayerThickness(-1);
    printJob.setBaseManager(QSharedPointer<OrderManifestManager>(orderMgr));
    printJob.setBodyManager(QSharedPointer<OrderManifestManager>(orderMgr));
    printJob.setDirectoryMode(true);
    printJob.setDirectoryPath(tilingMgr->getPath());

    emit uiStateChanged(TabIndex::Tiling, UiState::SelectCompleted);
}

int TilingTab::_getMaxCount()
{
    int wCount=0;
    int space = static_cast<int>(_space->getValue() / ProjectorPixelSize * _wRatio);

    for (int i = static_cast<int>(TilingMargin * _wRatio);
         i < (_areaWidth - (TilingMargin * _wRatio)); i += _pixmapWidth, wCount++) {
        if (wCount > 0)
            i+=space;

        debug(" i: %d wCount: %d \n", i, wCount);
    }

    wCount--;

    return wCount;
}

void TilingTab::_showWarningAndClose ()
{
    QMessageBox msgBox {this};
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setText("Slices are too wide to be tiled.");
    msgBox.exec();
}

void TilingTab::_setEnabled(bool enabled)
{
    this->_confirm->setEnabled(enabled);
    this->_space->setEnabled(enabled);
    this->_setupExpoTimeBt->setEnabled(enabled);
    this->_count->setEnabled(enabled);

    this->_currentLayerImage->clear();
}

void TilingTab::setupExpoTimeClicked(bool)
{
    debug("+ TilingTab::setupExpoTimeClicked\n");
    _expoTimePopup.setMinExposureBase(_minExposureBase * 1000.0);
    _expoTimePopup.setStepBase(_stepBase);
    _expoTimePopup.setMinExposureBody(_minExposureBody * 1000.0);
    _expoTimePopup.setStepBody(_stepBody);

    if (_expoTimePopup.exec() == QDialog::Accepted) {
        _minExposureBase = _expoTimePopup.minExposureBase() / 1000.0;
        _stepBase = _expoTimePopup.stepBase();
        _minExposureBody = _expoTimePopup.minExposureBody() / 1000.0;
        _stepBody = _expoTimePopup.stepBody();

        QSharedPointer<PrintProfile>& printProfile = printProfileManager()->activeProfile();

        printProfile->baseLayerParameters().setTilingDefaultExposure(_minExposureBase* 1000.0);
        printProfile->baseLayerParameters().setTilingDefaultExposureStep(_stepBase * 1000.0);
        printProfile->bodyLayerParameters().setTilingDefaultExposure(_minExposureBody * 1000.0);
        printProfile->bodyLayerParameters().setTilingDefaultExposureStep(_stepBody* 1000.0);

        _minExposureBaseLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBase ));
        _stepBaseLabel->setText(QString("%1s Exposure Step").arg(_stepBase));
        _minExposureBodyLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBody));
        _stepBodyLabel->setText(QString("%1s Exposure Step").arg(_stepBody));
    }

    setStepValue();
}

void TilingTab::setupTilingClicked(bool)
{
    debug("+ TilingTab::setupTilingClicked\n");

    this->_areaWidth = _currentLayerImage->width();
    this->_areaHeight = _currentLayerImage->height();
    this->_wRatio = (static_cast<double>(_areaWidth)) / ProjectorWindowSize.width();
    this->_hRatio = (static_cast<double>(_areaHeight)) / ProjectorWindowSize.height();

    QPixmap pixmap(QString("%1/%2").arg(printJob.getLayerDirectory(0))
        .arg(printJob.getLayerFileName(0)));

    if (this->_pixmap)
        delete this->_pixmap;

    this->_pixmap = new QPixmap(pixmap.scaled(static_cast<int>(pixmap.width() * _wRatio),
        static_cast<int>(pixmap.height() * _hRatio)));

    this->_pixmapWidth = this->_pixmap->width();
    this->_pixmapHeight = this->_pixmap->height();

    if (_getMaxCount() < 1) {
            _showWarningAndClose();
            return;
    }

    _setEnabled(true);
    _showLayerImage();

    _fileNameLabel->setText( GetFileBaseName(printJob.getModelFilename()) );
    _fileNameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    emit uiStateChanged(TabIndex::Prepare, UiState::TilingClicked);
}

void TilingTab::printJobChanged() {
    _updateExposureTiming();
}

void TilingTab::activeProfileChanged(QSharedPointer<PrintProfile> newProfile) {
    (void)newProfile;
    _updateExposureTiming();
}

void TilingTab::_updateExposureTiming() {
    this->_stepBase = ((double)printJob.baseLayerParameters().tilingDefaultExposureStep()) / 1000.0;
    this->_stepBody = ((double)printJob.bodyLayerParameters().tilingDefaultExposureStep()) / 1000.0;
    this->_minExposureBase = ((double)printJob.baseLayerParameters().tilingDefaultExposure()) / 1000.0;
    this->_minExposureBody = ((double)printJob.bodyLayerParameters().tilingDefaultExposure()) / 1000.0;;

    _minExposureBaseLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBase));
    _stepBaseLabel->setText(QString("%1s Exposure Step").arg(_stepBase));
    _minExposureBodyLabel->setText(QString("%1s Minimum Layer Exposure").arg(_minExposureBody));
    _stepBodyLabel->setText(QString("%1s Exposure Step").arg(_stepBody));
}
