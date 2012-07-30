/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

        void loadMainHtml();

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

        bool canBeContent();
        bool canBeTool();

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
        int mCanBeContent;
        int mCanBeTool;
        enum OSType
        {
            type_NONE = 0, // 0000
            type_WIN  = 1, // 0001
            type_MAC  = 2, // 0010
            type_UNIX = 4, // 0100
            type_ALL  = 7, // 0111
        };

        virtual void injectInlineJavaScript();
        virtual void paintEvent(QPaintEvent * event);
        virtual void dropEvent(QDropEvent *);

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
