
#include <QtGui>

#include "pdf/UBWebPluginPDFWidget.h"
#include "frameworks/UBFileSystemUtils.h"

UBWebPluginWidget::UBWebPluginWidget(const QUrl &url, QWidget *parent)
    : QWidget(parent)
    , mLoadingProgressBar(this)
{
    UBHttpGet* httpGet = new UBHttpGet(this);
    
    connect(httpGet, SIGNAL(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(downloadFinished(bool, QUrl, QString, QByteArray)));
    connect(httpGet, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    
    httpGet->get(url);
}

UBWebPluginWidget::~UBWebPluginWidget()
{
    // NOOP
}

void UBWebPluginWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    mLoadingProgressBar.move(geometry().center() - mLoadingProgressBar.geometry().center());
}

void UBWebPluginWidget::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal == -1)
    {
        mLoadingProgressBar.setMinimum(0);
        mLoadingProgressBar.setMaximum(0);
    } 
    else
    {
        mLoadingProgressBar.setMaximum(bytesTotal);
        mLoadingProgressBar.setValue(bytesReceived);
    }
}

void UBWebPluginWidget::downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData)
{
    Q_UNUSED(pSuccess);
    Q_UNUSED(pContentTypeHeader);

    QString tempFile = UBFileSystemUtils::createTempDir("UBWebPluginTemplate") + "/" + QFileInfo(sourceUrl.path()).fileName();
    QFile pdfFile(tempFile);
    pdfFile.open(QIODevice::WriteOnly);
    pdfFile.write(pData);
    pdfFile.close();
    handleFile(tempFile);
    mLoadingProgressBar.hide();
    update();
}

QString UBWebPluginWidget::title() const
{
    return QString(tr("Loading..."));
}
