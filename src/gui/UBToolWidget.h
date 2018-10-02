/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBTOOLWIDGET_H_
#define UBTOOLWIDGET_H_

#include <QtGui>
#include <QWidget>

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
