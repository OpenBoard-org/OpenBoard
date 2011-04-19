
#ifndef UBWEBPLUGINPDFWIDGET_H
#define UBWEBPLUGINPDFWIDGET_H

#include "web/UBWebPluginWidget.h"

class PDFRenderer;
class QProgressBar;

class UBWebPluginPDFWidget : public UBWebPluginWidget
{
    Q_OBJECT

    public:
        UBWebPluginPDFWidget(const QUrl &url, QWidget *parent = 0);
        virtual ~UBWebPluginPDFWidget();

        virtual QString title() const;

    public slots:
        virtual void zoomIn();
        virtual void zoomOut();

    protected:
        virtual void handleFile(const QString &filePath);

        virtual void paintEvent(QPaintEvent *paintEvent);

        virtual void keyReleaseEvent(QKeyEvent *keyEvent);

    protected slots:
        virtual bool previousPage();
        virtual bool nextPage();

    private:
        QToolButton mPreviousPageButton;
        QToolButton mNextPageButton;
        QAction *mPreviousPageAction;
        QAction *mNextPageAction;

        PDFRenderer *mRenderer;
        qreal mScale;
        int mPageNumber;
};

#endif // UBWEBPLUGINPDFWIDGET_H
