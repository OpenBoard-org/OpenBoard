#ifndef UNIBOARDSANKORETRANSITION_H
#define UNIBOARDSANKORETRANSITION_H

#include <QObject>
#include <QFileInfo>

class UniboardSankoreTransition : public QObject
{
    Q_OBJECT
public:
    explicit UniboardSankoreTransition(QObject *parent = 0);
    bool backupUniboardDirectory();
    void documentTransition();

private:
    void rollbackDocumentsTransition(QFileInfoList& fileInfoList);

protected:
    QString mUniboardSourceDirectory;

signals:

public slots:

};

#endif // UNIBOARDSANKORETRANSITION_H
