
#ifndef UBWEBPLUGINWIDGET_H
#define UBWEBPLUGINWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QToolButton>

#include "network/UBHttpGet.h"

class UBWebPluginWidget : public QWidget
{
    Q_OBJECT

    public:
        UBWebPluginWidget(const QUrl &url, QWidget *parent = 0);
        virtual ~UBWebPluginWidget();

        virtual QString title() const;

    protected:
        virtual void handleFile(const QString &filePath) = 0;

        virtual void paintEvent(QPaintEvent *paintEvent) = 0;
        virtual void resizeEvent(QResizeEvent *event);

    private slots:
        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData);

    private:
        QProgressBar mLoadingProgressBar;
};

#endif // UBWEBPLUGINWIDGET_H
