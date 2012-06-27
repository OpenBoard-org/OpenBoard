/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef UBGRAPHICSSCENE_H_
#define UBGRAPHICSSCENE_H_

#include <QtGui>

#include "frameworks/UBCoreGraphicsScene.h"

#include "core/UB.h"

#include "UBItem.h"
#include "tools/UBGraphicsCurtainItem.h"


class UBGraphicsPixmapItem;
class UBGraphicsProxyWidget;
class UBGraphicsSvgItem;
class UBGraphicsPolygonItem;
class UBGraphicsVideoItem;
class UBGraphicsAudioItem;
class UBGraphicsWidgetItem;
class UBGraphicsW3CWidgetItem;
class UBGraphicsAppleWidgetItem;
class UBGraphicsPDFItem;
class UBGraphicsTextItem;
class UBGraphicsRuler;
class UBGraphicsProtractor;
class UBGraphicsCompass;
class UBAbstractWidget;
class UBDocumentProxy;
class UBGraphicsCurtainItem;
class UBGraphicsStroke;
class UBMagnifierParams;
class UBMagnifier;
class UBGraphicsCache;

const double PI = 4.0 * atan(1.0);

class UBGraphicsScene: public UBCoreGraphicsScene, public UBItem
{
    Q_OBJECT

    public:
    //        tmp stub for divide addings scene objects from undo mechanism implementation
    void setURStackEnable(bool set = true) {enableUndoRedoStack = set;}

        UBGraphicsScene(UBDocumentProxy *parent);
        virtual ~UBGraphicsScene();

        virtual UBItem* deepCopy() const;

        UBGraphicsScene* sceneDeepCopy() const;

        void clearItemsAndAnnotations();
        void clearItems();
        void clearAnnotations();

        bool inputDevicePress(const QPointF& scenePos, const qreal& pressure = 1.0);
        bool inputDeviceMove(const QPointF& scenePos, const qreal& pressure = 1.0);
        bool inputDeviceRelease();

        void leaveEvent (QEvent* event);

        void addItem(QGraphicsItem* item);
        void removeItem(QGraphicsItem* item);

        void addItems(const QSet<QGraphicsItem*>& item);
        void removeItems(const QSet<QGraphicsItem*>& item);

        UBGraphicsWidgetItem* addWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsAppleWidgetItem* addAppleWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsW3CWidgetItem* addW3CWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0),int widgetType = UBGraphicsItemType::W3CWidgetItemType);
        void addGraphicsWidget(UBGraphicsWidgetItem* graphicsWidget, const QPointF& pPos = QPointF(0, 0));

        UBGraphicsVideoItem* addVideo(const QUrl& pVideoFileUrl, bool shouldPlayAsap, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsAudioItem* addAudio(const QUrl& pAudioFileUrl, bool shouldPlayAsap, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsSvgItem* addSvg(const QUrl& pSvgFileUrl, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsTextItem* addText(const QString& pString, const QPointF& pTopLeft = QPointF(0, 0));

        UBGraphicsTextItem*  addTextWithFont(const QString& pString, const QPointF& pTopLeft = QPointF(0, 0)
                , int pointSize = -1, const QString& fontFamily = "", bool bold = false, bool italic = false);
        UBGraphicsTextItem* addTextHtml(const QString &pString = QString(), const QPointF& pTopLeft = QPointF(0, 0));

        UBGraphicsW3CWidgetItem* addOEmbed(const QUrl& pContentUrl, const QPointF& pPos = QPointF(0, 0));

        QGraphicsItem* setAsBackgroundObject(QGraphicsItem* item, bool pAdaptTransformation = false, bool expand = false);

        QGraphicsItem* backgroundObject() const
        {
            return mBackgroundObject;
        }

        bool isBackgroundObject(const QGraphicsItem* item) const
        {
            return item == mBackgroundObject;
        }

        QGraphicsItem* scaleToFitDocumentSize(QGraphicsItem* item, bool center = false, int margin = 0, bool expand = false);

        QRectF normalizedSceneRect(qreal ratio = -1.0);

        void moveTo(const QPointF& pPoint);
        void drawLineTo(const QPointF& pEndPoint, const qreal& pWidth, bool bLineStyle);
        void eraseLineTo(const QPointF& pEndPoint, const qreal& pWidth);
        void drawArcTo(const QPointF& pCenterPoint, qreal pSpanAngle);

        bool isEmpty() const;

        bool isModified() const
        {
            return mIsModified;
        }

        void setModified(bool pModified)
        {
            mIsModified = pModified;
        }

        void setDocument(UBDocumentProxy* pDocument);

        UBDocumentProxy* document() const
        {
            return mDocument;
        }

        bool isDarkBackground() const
        {
            return mDarkBackground;
        }

        bool isLightBackground() const
        {
            return !mDarkBackground;
        }

        bool isCrossedBackground() const
        {
            return mCrossedBackground;
        }

        void setDrawingZIndex(qreal pDrawingZIndex)
        {
            mDrawingZIndex = pDrawingZIndex;
        }

        void setObjectZIndex(qreal pObjectZIndex)
        {
            mObjectZIndex = pObjectZIndex;
        }

        bool hasBackground()
        {
            return (mBackgroundObject != 0);
        }

        qreal getNextObjectZIndex();

        void addRuler(QPointF center);
        void addProtractor(QPointF center);
        void addCompass(QPointF center);
        void addTriangle(QPointF center);
        void addMagnifier(UBMagnifierParams params);

        void addMask(const QPointF &center = QPointF());
        void addCache();

        QList<QGraphicsItem*> getFastAccessItems()
        {
            return mFastAccessItems;
        }

        class SceneViewState
        {
            public:
                SceneViewState()
                {
                    zoomFactor = 1;
                    horizontalPosition = 0;
                    verticalPostition = 0;
                }

                SceneViewState(qreal pZoomFactor, int pHorizontalPosition, int pVerticalPostition)
                {
                    zoomFactor = pZoomFactor;
                    horizontalPosition = pHorizontalPosition;
                    verticalPostition = pVerticalPostition;
                }

                qreal zoomFactor;
                int horizontalPosition;
                int verticalPostition;
        };

        SceneViewState viewState() const
        {
            return mViewState;
        }

        void setViewState(const SceneViewState& pViewState)
        {
            mViewState = pViewState;
        }

        virtual void setRenderingQuality(UBItem::RenderingQuality pRenderingQuality);

        QList<QUrl> relativeDependencies() const;

        QSize nominalSize();

        void setNominalSize(const QSize& pSize);

        void setNominalSize(int pWidth, int pHeight);

        qreal currentObjectZIndex()
        {
            return mObjectZIndex;
        }

        enum RenderingContext
        {
            Screen = 0, NonScreen, PdfExport, Podcast
        };

        void setRenderingContext(RenderingContext pRenderingContext)
        {
            mRenderingContext = pRenderingContext;
        }

        RenderingContext renderingContext() const
        {
            return mRenderingContext;
        }

        static qreal backgroundLayerStart;
        static qreal objectLayerStart;
        static qreal drawingLayerStart;
        static qreal toolLayerStart;

        static qreal toolOffsetEraser;
        static qreal toolOffsetPointer;
        static qreal toolOffsetRuler;
        static qreal toolOffsetProtractor;
        static qreal toolOffsetCompass;
        static qreal toolOffsetCurtain;
        static qreal toolOffsetTriangle;
        static qreal toolOffsetCache;

        QSet<QGraphicsItem*> tools(){ return mTools;}

        void registerTool(QGraphicsItem* item)
        {
            mTools << item;
        }

		const QPointF& previousPoint()
		{
			return mPreviousPoint;
		}

		void initStroke();

    public slots:

        void hideEraser();

        void setBackground(bool pIsDark, bool pIsCrossed);

        void deselectAllItems();

        UBGraphicsPixmapItem* addPixmap(const QPixmap& pPixmap, 
            QGraphicsItem* replaceFor,
            const QPointF& pPos = QPointF(0,0), 
            qreal scaleFactor = 1.0, 
            bool pUseAnimation = false);

        void textUndoCommandAdded(UBGraphicsTextItem *textItem);

        void setToolCursor(int tool);

        void selectionChangedProcessing();

        void moveMagnifier(QPoint newPos);
        void closeMagnifier();
        void zoomInMagnifier();
        void zoomOutMagnifier();
        void resizedMagnifier(qreal newPercent);

    signals:

       void pageSizeChanged();

    protected:

        UBGraphicsPolygonItem* lineToPolygonItem(const QLineF& pLine, const qreal& pWidth);
        UBGraphicsPolygonItem* arcToPolygonItem(const QLineF& pStartRadius, qreal pSpanAngle, qreal pWidth);
        UBGraphicsPolygonItem* polygonToPolygonItem(const QPolygonF pPolygon);

        void initPolygonItem(UBGraphicsPolygonItem*);

        void drawEraser(const QPointF& pEndPoint, bool isFirstDraw = false);
        void drawPointer(const QPointF& pEndPoint, bool isFirstDraw = false);
        void DisposeMagnifierQWidgets();


        virtual void keyReleaseEvent(QKeyEvent * keyEvent);

        void recolorAllItems();

        qreal getNextDrawingZIndex();

        virtual void drawItems (QPainter * painter, int numItems,
                QGraphicsItem * items[], const QStyleOptionGraphicsItem options[], QWidget * widget = 0);

        QGraphicsItem* rootItem(QGraphicsItem* item) const;

    private:
        void setDocumentUpdated();

        qreal mDrawingZIndex;
        qreal mObjectZIndex;

        QGraphicsEllipseItem* mEraser;
        QGraphicsEllipseItem* mPointer;

        QSet<QGraphicsItem*> mAddedItems;
        QSet<QGraphicsItem*> mRemovedItems;

        UBDocumentProxy* mDocument;

        bool mDarkBackground;
        bool mCrossedBackground;

        bool mIsModified;

        QGraphicsItem* mBackgroundObject;

        QPointF mPreviousPoint;
        qreal mPreviousWidth;

        QList<UBGraphicsPolygonItem*> mPreviousPolygonItems;

        SceneViewState mViewState;

        bool mInputDeviceIsPressed;

        QSet<QGraphicsItem*> mTools;

        UBGraphicsPolygonItem *mArcPolygonItem;

        QSize mNominalSize;

        RenderingContext mRenderingContext;

        UBGraphicsStroke* mCurrentStroke;

        bool mShouldUseOMP;

        int mItemCount;

        QList<QGraphicsItem*> mFastAccessItems; // a local copy as QGraphicsScene::items() is very slow in Qt 4.6

        //int mMesure1Ms, mMesure2Ms;

        bool mHasCache;
        //        tmp stub for divide addings scene objects from undo mechanism implementation
        bool enableUndoRedoStack;

        UBMagnifier *magniferControlViewWidget;
        UBMagnifier *magniferDisplayViewWidget;


};

#endif /* UBGRAPHICSSCENE_H_ */
