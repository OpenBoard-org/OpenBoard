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

#ifndef UBTOOLWIDGET_H_
#define UBTOOLWIDGET_H_

#include <QtGui>

class UBGraphicsWidgetItem;
class QWidget;
class UBGraphicsScene;

class UBToolWidget : public QGraphicsWidget
{
    Q_OBJECT

    public:
        UBToolWidget(const QUrl& pUrl, QGraphicsItem *pParent = 0);
        UBToolWidget(UBGraphicsWidgetItem* pGraphicsWidgetItem, QGraphicsItem *pParent = 0);
        virtual ~UBToolWidget();

        void centerOn(const QPointF& pos);

        QPointF naturalCenter() const;

        UBGraphicsWidgetItem* graphicsWidgetItem() const;
        virtual UBGraphicsScene* scene();

    protected:
        void initialize();
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void javaScriptWindowObjectCleared();

    protected:

        static QPixmap *sClosePixmap;
        static QPixmap *sUnpinPixmap;

        UBGraphicsWidgetItem *mGraphicsWidgetItem;

        QPointF mMousePressPos;

        bool mShouldMoveWidget;

        int mContentMargin;
        int mFrameWidth;
};

#endif /* UBTOOLWIDGET_H_ */
