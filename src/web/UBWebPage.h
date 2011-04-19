
#ifndef UBWEBPAGE_H_
#define UBWEBPAGE_H_

/*
 * Use UBWebPage instead of QWebPage to log JavaScript console messages with qDebug
 */

#include <QtCore>
#include <QWebPage>

class UBWebPage : public QWebPage
{
    Q_OBJECT;

    public:
        UBWebPage(QObject *parent = 0);
        virtual ~UBWebPage();

        virtual void javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID);

        virtual QString userAgentForUrl(const QUrl& url) const;

    private:
        QString mCachedUserAgentString;
        QWebPluginFactory *mPluginFactory;

};

#endif /* UBWEBPAGE_H_ */
