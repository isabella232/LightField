#ifndef SPOILER_H
#define SPOILER_H

#include <QtCore>
#include <QtWidgets>

class Spoiler : public QWidget {
    Q_OBJECT
private:
    QGridLayout _mainLayout;
    QToolButton _toggleButton;
    QFrame _headerLine;
    QParallelAnimationGroup _toggleAnimation;
    QScrollArea _contentArea;
    int _animationDuration{300};
    QString _title;
public:
    explicit Spoiler(const QString & title = "", const int _animationDuration = 150, QWidget *parent = 0);
    void setContentLayout(QVBoxLayout* contentLayout);
    void setCollapsed(bool collapsed);
    bool isCollapsed();
    void setMaxHeight(int maxHeight);

signals:

    void collapseStateChanged(bool collapsed);
};
#endif // SPOILER_H
