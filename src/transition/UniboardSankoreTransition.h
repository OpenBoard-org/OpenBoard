#ifndef UNIBOARDSANKORETRANSITION_H
#define UNIBOARDSANKORETRANSITION_H

#include <QObject>
#include <QFileInfo>
#include "gui/UBUpdateDlg.h"
#include "document/UBDocumentProxy.h"

class UniboardSankoreTransition : public QObject
{
    Q_OBJECT
public:
    explicit UniboardSankoreTransition(QObject *parent = 0);
    ~UniboardSankoreTransition();
    void documentTransition();


private:
    void rollbackDocumentsTransition(QFileInfoList& fileInfoList);
    UBUpdateDlg* mTransitionDlg;

protected:
    QString mUniboardSourceDirectory;

signals:
    void transitionFinished(bool result);
    void docAdded(UBDocumentProxy* doc);

private slots:
    void startDocumentTransition();

};

#endif // UNIBOARDSANKORETRANSITION_H
