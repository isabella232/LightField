#include "spoiler.h"
#include <QPropertyAnimation>


Spoiler::Spoiler(const QString & title, const int animationDuration, QWidget *parent) : QWidget(parent), _animationDuration(animationDuration) {
    _toggleButton.setStyleSheet("QToolButton { border: none; }");
    _toggleButton.setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    _toggleButton.setArrowType(Qt::ArrowType::RightArrow);
    _toggleButton.setText(title);
    _toggleButton.setCheckable(true);
    _toggleButton.setChecked(false);
    auto font = _toggleButton.font();
    font.setPixelSize(30);
    _toggleButton.setFont(font);

    _title = title;

    _headerLine.setFrameShape(QFrame::HLine);
    _headerLine.setFrameShadow(QFrame::Sunken);
    _headerLine.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

    //contentArea.setStyleSheet("QScrollArea { background-color: white; border: none; }");
    _contentArea.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // start out collapsed
    _contentArea.setMaximumHeight(0);
    _contentArea.setMinimumHeight(0);
    // let the entire widget grow and shrink with its content
    _toggleAnimation.addAnimation(new QPropertyAnimation(this, "minimumHeight"));
    _toggleAnimation.addAnimation(new QPropertyAnimation(this, "maximumHeight"));
    _toggleAnimation.addAnimation(new QPropertyAnimation(&_contentArea, "maximumHeight"));
    // don't waste space
    _mainLayout.setVerticalSpacing(0);
    _mainLayout.setContentsMargins(0, 0, 0, 0);
    int row = 0;
    _mainLayout.addWidget(&_toggleButton, row, 0, 1, 1, Qt::AlignLeft);
    _mainLayout.addWidget(&_headerLine, row++, 2, 1, 1);
    _mainLayout.addWidget(&_contentArea, row, 0, 1, 3);
    setLayout(&_mainLayout);
    QObject::connect(&_toggleButton, &QToolButton::clicked, [this](const bool checked) {
        _toggleButton.setArrowType(checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
        _toggleAnimation.setDirection(checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
        _toggleAnimation.start();

        _toggleButton.setText( !checked ? _title : QString("%1 ✓").arg(_title));

        emit collapseStateChanged(checked);
    });

    this->setStyleSheet(_contentArea.styleSheet());
}

void Spoiler::setContentLayout(QVBoxLayout* contentLayout) {
    delete _contentArea.layout();
    _contentArea.setLayout(contentLayout);
    const auto collapsedHeight = sizeHint().height() - _contentArea.maximumHeight();
    auto contentHeight = contentLayout->sizeHint().height();
    for (int i = 0; i < _toggleAnimation.animationCount() - 1; ++i) {
        QPropertyAnimation * spoilerAnimation = static_cast<QPropertyAnimation *>(_toggleAnimation.animationAt(i));
        spoilerAnimation->setDuration(_animationDuration);
        spoilerAnimation->setStartValue(collapsedHeight);
        spoilerAnimation->setEndValue(collapsedHeight + contentHeight);
    }
    QPropertyAnimation * contentAnimation = static_cast<QPropertyAnimation *>(_toggleAnimation.animationAt(_toggleAnimation.animationCount() - 1));
    contentAnimation->setDuration(_animationDuration);
    contentAnimation->setStartValue(0);
    contentAnimation->setEndValue(contentHeight);
}

void Spoiler::setCollapsed(bool checked) {
    _toggleButton.setArrowType(!checked ? Qt::ArrowType::DownArrow : Qt::ArrowType::RightArrow);
    _toggleButton.setChecked(!checked);
    _toggleAnimation.setDirection(!checked ? QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    _toggleAnimation.start();

    _toggleButton.setText( checked ? _title : QString("%1 ✓").arg(_title));
}

bool Spoiler::isCollapsed() {
    return _toggleButton.isChecked();
}

void Spoiler::setMaxHeight(int maxHeight)
{
    _contentArea.setMaximumHeight(maxHeight);
}
