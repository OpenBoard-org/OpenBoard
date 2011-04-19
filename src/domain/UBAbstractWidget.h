/*
 * UBAbstractWidget.h
 *
 *  Created on: 11 Feb. 2009
 *      Author: Luc
 */

#ifndef UBABSTRACTWIDGET_H_
#define UBABSTRACTWIDGET_H_

#include <QtGui>
#include <QtWebKit>

#include "web/UBRoutedMouseEventWebView.h"


struct UBWidgetType
{
    enum Enum
    {
        W3C = 0, Apple, Other
    };
};


class UBAbstractWidget : public UBRoutedMouseEventWebView
{
    Q_OBJECT;

    public:
        UBAbstractWidget(const QUrl& pWidgetUrl, QWidget *parent = 0);
        virtual ~UBAbstractWidget();

        QUrl mainHtml()
        {
            return mMainHtmlUrl;
        }

        QUrl widgetUrl()
        {
            return mWidgetUrl;
        }

        QString mainHtmlFileName()
        {
            return mMainHtmlFileName;
        }

        bool hasEmbededObjects();
        bool hasEmbededFlash();

        void resize(qreal width, qreal height);

        QSize nominalSize() const
        {
            return mNominalSize;
        }

        bool canBeContent() const
        {
            return mCanBeContent;
        }

        bool canBeTool() const
        {
            return mCanBeTool;
        }

        bool hasLoadedSuccessfully() const
        {
            return (mInitialLoadDone && !mLoadIsErronous);
        }

        bool freezable() { return mIsFreezable;}
        bool resizable() { return mIsResizable;}

        static QString iconFilePath(const QUrl& pUrl);
        static QString widgetName(const QUrl& pUrl);
        static int widgetType(const QUrl& pUrl);

        bool isFrozen(){ return mIsFrozen;}

        QPixmap snapshot(){return mSnapshot;}
        void setSnapshot(const QPixmap& pix);

        QPixmap takeSnapshot();

    public slots:
        void freeze();
        void unFreeze();

    signals:

        void geometryChangeRequested(const QRect & geom);

    protected:

        bool mMouseIsPressed;
        bool mFirstReleaseAfterMove;

        virtual bool event(QEvent *e);

        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

        virtual QWebView * createWindow(QWebPage::WebWindowType type);

        QUrl mMainHtmlUrl;
        QString mMainHtmlFileName;
        QUrl mWidgetUrl;
        QSize mNominalSize;
        bool mIsResizable;
        bool mInitialLoadDone;
        bool mLoadIsErronous;

        bool mIsFreezable;
        bool mCanBeContent;
        bool mCanBeTool;

        virtual void injectInlineJavaScript();
        virtual void paintEvent(QPaintEvent * event);

    protected slots:

        void mainFrameLoadFinished(bool ok);

    private:

        static QStringList sInlineJavaScripts;
        static bool sInlineJavaScriptLoaded;

        bool mIsFrozen;
        QPixmap mSnapshot;

        bool mIsTakingSnapshot;

    private slots:
        void javaScriptWindowObjectCleared();

};

#endif /* UBABSTRACTWIDGET_H_ */
