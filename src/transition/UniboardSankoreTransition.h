#ifndef UNIBOARDSANKORETRANSITION_H
#define UNIBOARDSANKORETRANSITION_H

#include <QObject>
#include <QFileInfo>
#include <QThread>
#include "gui/UBUpdateDlg.h"
#include "document/UBDocumentProxy.h"

class UniboardSankoreThread : public QThread
{
    Q_OBJECT
public:
    UniboardSankoreThread(QObject* parent = 0);
    ~UniboardSankoreThread();

    void run();

};

class UniboardSankoreTransition : public QObject
{
    Q_OBJECT
public:
    explicit UniboardSankoreTransition(QObject *parent = 0);
    ~UniboardSankoreTransition();
    void documentTransition();
    void executeTransition();


private:
    void rollbackDocumentsTransition(QFileInfoList& fileInfoList);
    UBUpdateDlg* mTransitionDlg;

protected:
    QString mUniboardSourceDirectory;
    QString mOldSankoreDirectory;
    UniboardSankoreThread* mThread;

signals:
    void transitionFinished(bool result);
    void docAdded(UBDocumentProxy* doc);
    void transitioningFile(QString documentName);

private slots:
    void startDocumentTransition();

};

#endif // UNIBOARDSANKORETRANSITION_H
