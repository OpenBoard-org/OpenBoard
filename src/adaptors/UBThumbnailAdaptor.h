#ifndef UBTHUMBNAILADAPTOR_H
#define UBTHUMBNAILADAPTOR_H

#include <QtCore>

class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;

class UBThumbnailAdaptor : public QObject
{
    Q_OBJECT;

public:

    UBThumbnailAdaptor(QObject *parent = 0);
    ~UBThumbnailAdaptor();

    static void persistScene(const QString& pDocPath, UBGraphicsScene* pScene, const int pageIndex,  const bool overrideModified = false);

    static QList<QPixmap> load(UBDocumentProxy* proxy);

    static QUrl thumbnailUrl(UBDocumentProxy* proxy, const int pageIndex);

};

#endif // UBTHUMBNAILADAPTOR_H
