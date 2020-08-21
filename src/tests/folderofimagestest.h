#if defined _DEBUG

#ifndef FOLDEROFIMAGESTEST_H
#define FOLDEROFIMAGESTEST_H

#include "abstracttest.h"

class FolderOfImagesTest : public AbstractTest
{
public:
    FolderOfImagesTest() = default;

    static QString testNameString;
    QString testName() override {
        return testNameString;
    }
private slots:
    void start(QString modelName);
};

#endif // FOLDEROFIMAGESTEST_H
#endif //_DEBUG
