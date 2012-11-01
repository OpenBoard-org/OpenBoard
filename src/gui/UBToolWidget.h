/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBTOOLWIDGET_H_
#define UBTOOLWIDGET_H_

#include <QtGui>

class UBGraphicsWidgetItem;
class QWidget;
class UBGraphicsScene;
class QWebView;

class UBToolWidget : public QWidget
{
    Q_OBJECT;

    public:
        UBToolWidget(const QUrl& pUrl, QWidget* pParent = 0);
        UBToolWidget(UBGraphicsWidgetItem* pWidget, QWidget* pParent = 0);
        virtual ~UBToolWidget();

        void remove();
        void centerOn(const QPoint& pos);

        QPoint naturalCenter() const;

        UBGraphicsWidgetItem *toolWidget() const;

    protected:
        void initialize();
        virtual void paintEvent(QPaintEvent *event);

        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

        virtual bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void javaScriptWindowObjectCleared();

    protected:
        QWebView *mWebView;
        UBGraphicsWidgetItem *mToolWidget;

        static QPixmap *sClosePixmap;
        static QPixmap *sUnpinPixmap;

        QPoint mMousePressPos;
        bool mShouldMoveWidget;
        int mContentMargin;
        int mFrameWidth;
};

#endif /* UBTOOLWIDGET_H_ */
