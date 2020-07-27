#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

class AbstractTest: public QObject {
    Q_OBJECT
public:
    static QString testName;

    virtual void start() = 0;
signals:
    void successed();
    void failed();
};

#endif // ABSTRACTTEST_H
