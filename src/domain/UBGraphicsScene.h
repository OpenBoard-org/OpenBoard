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
class UBGraphicsMediaItem;
class UBGraphicsWidgetItem;
class UBGraphicsW3CWidgetItem;
class UBGraphicsAppleWidgetItem;
class UBToolWidget;
class UBGraphicsPDFItem;
class UBGraphicsTextItem;
class UBGraphicsRuler;
class UBGraphicsProtractor;
class UBGraphicsCompass;
class UBDocumentProxy;
class UBGraphicsCurtainItem;
class UBGraphicsStroke;
class UBMagnifierParams;
class UBMagnifier;
class UBGraphicsCache;
class UBGraphicsGroupContainerItem;
class UBSelectionFrame;
class UBBoardView;

const double PI = 4.0 * atan(1.0);

class UBZLayerController : public QObject
{
    Q_OBJECT

public:
    struct ItemLayerTypeData {
        ItemLayerTypeData() : bottomLimit(0), topLimit(0), curValue(0), incStep(1) {;}
        ItemLayerTypeData(qreal bot, qreal top, qreal increment = 1) : bottomLimit(bot), topLimit(top), curValue(bot), incStep(increment) {;}
        qreal bottomLimit; //bottom bound of the layer
        qreal topLimit;//top bound of the layer
        qreal curValue;//current value of variable
        qreal incStep;//incremental step
    };

    enum moveDestination {
        up
        , down
        , top
        , bottom
    };

    typedef QMap<itemLayerType::Enum, ItemLayerTypeData> ScopeMap;

    UBZLayerController(QGraphicsScene *scene);

    qreal getBottomLimit(itemLayerType::Enum key) const {return scopeMap.value(key).bottomLimit;}
    qreal getTopLimit(itemLayerType::Enum key) const {return scopeMap.value(key).topLimit;}
    bool validLayerType(itemLayerType::Enum key) const {return scopeMap.contains(key);}

    static qreal errorNum() {return errorNumber;}

    qreal generateZLevel(itemLayerType::Enum key);
    qreal generateZLevel(QGraphicsItem *item);

    qreal changeZLevelTo(QGraphicsItem *item, moveDestination dest);
    itemLayerType::Enum typeForData(QGraphicsItem *item) const;
    void setLayerType(QGraphicsItem *pItem, itemLayerType::Enum pNewType);
    void shiftStoredZValue(QGraphicsItem *item, qreal zValue);

    bool zLevelAvailable(qreal z);

private:
    ScopeMap scopeMap;
    static qreal errorNumber;
    QGraphicsScene *mScene;
};

class UBGraphicsScene: public UBCoreGraphicsScene, public UBItem
{
    Q_OBJECT

    public:

    enum clearCase {
        clearItemsAndAnnotations = 0
        , clearAnnotations
        , clearItems
        , clearBackground
    };

    //        tmp stub for divide addings scene objects from undo mechanism implementation
        void enableUndoRedoStack(){mUndoRedoStackEnabled = true;}
        void setURStackEnable(bool enable){mUndoRedoStackEnabled = enable;}
        bool isURStackIsEnabled(){return mUndoRedoStackEnabled;}

        UBGraphicsScene(UBDocumentProxy *parent, bool enableUndoRedoStack = true);
        virtual ~UBGraphicsScene();

        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const {Q_UNUSED(copy);}

        UBGraphicsScene* sceneDeepCopy() const;

        void clearContent(clearCase pCase = clearItemsAndAnnotations);

        bool inputDevicePress(const QPointF& scenePos, const qreal& pressure = 1.0);
        bool inputDeviceMove(const QPointF& scenePos, const qreal& pressure = 1.0);
        bool inputDeviceRelease(int tool = -1);

        void leaveEvent (QEvent* event);

        void addItem(QGraphicsItem* item);
        void removeItem(QGraphicsItem* item);

        void addItems(const QSet<QGraphicsItem*>& item);
        void removeItems(const QSet<QGraphicsItem*>& item);

        UBGraphicsWidgetItem* addWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsAppleWidgetItem* addAppleWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsW3CWidgetItem* addW3CWidget(const QUrl& pWidgetUrl, const QPointF& pPos = QPointF(0, 0));
        void addGraphicsWidget(UBGraphicsWidgetItem* graphicsWidget, const QPointF& pPos = QPointF(0, 0));

        QPointF lastCenter();
        void setLastCenter(QPointF center);

        UBGraphicsMediaItem* addMedia(const QUrl& pMediaFileUrl, bool shouldPlayAsap, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsMediaItem* addVideo(const QUrl& pVideoFileUrl, bool shouldPlayAsap, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsMediaItem* addAudio(const QUrl& pAudioFileUrl, bool shouldPlayAsap, const QPointF& pPos = QPointF(0, 0));
        UBGraphicsSvgItem* addSvg(const QUrl& pSvgFileUrl, const QPointF& pPos = QPointF(0, 0), const QByteArray pData = QByteArray());
        UBGraphicsTextItem* addText(const QString& pString, const QPointF& pTopLeft = QPointF(0, 0));

        UBGraphicsTextItem*  addTextWithFont(const QString& pString, const QPointF& pTopLeft = QPointF(0, 0)
                , int pointSize = -1, const QString& fontFamily = "", bool bold = false, bool italic = false);
        UBGraphicsTextItem* addTextHtml(const QString &pString = QString(), const QPointF& pTopLeft = QPointF(0, 0));

        UBGraphicsW3CWidgetItem* addOEmbed(const QUrl& pContentUrl, const QPointF& pPos = QPointF(0, 0));

        UBGraphicsGroupContainerItem *createGroup(QList<QGraphicsItem*> items);
        void addGroup(UBGraphicsGroupContainerItem *groupItem);

        QGraphicsItem* setAsBackgroundObject(QGraphicsItem* item, bool pAdaptTransformation = false, bool expand = false);
        void unsetBackgroundObject();

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

        QGraphicsItem *itemForUuid(QUuid uuid);

        void moveTo(const QPointF& pPoint);
        void drawLineTo(const QPointF& pEndPoint, const qreal& pWidth, bool bLineStyle);
        void drawLineTo(const QPointF& pEndPoint, const qreal& pStartWidth, const qreal& endWidth, bool bLineStyle);
        void eraseLineTo(const QPointF& pEndPoint, const qreal& pWidth);
        void drawArcTo(const QPointF& pCenterPoint, qreal pSpanAngle);
        void drawCurve(const QList<QPair<QPointF, qreal> > &points);
        void drawCurve(const QList<QPointF>& points, qreal startWidth, qreal endWidth);

        bool isEmpty() const;

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

        UBPageBackground pageBackground() const
        {
            return mPageBackground;
        }

        int backgroundGridSize() const
        {
            return mBackgroundGridSize;
        }

        bool intermediateLines() const
        {
            return mIntermediateLines;
        }

        bool hasBackground()
        {
            return (mBackgroundObject != 0);
        }

        void addRuler(QPointF center);
        void addAxes(QPointF center);
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
                    mLastSceneCenter = QPointF();
                }

                SceneViewState(qreal pZoomFactor, int pHorizontalPosition, int pVerticalPostition, QPointF sceneCenter = QPointF())// 1595/1605
                {
                    zoomFactor = pZoomFactor;
                    horizontalPosition = pHorizontalPosition;
                    verticalPostition = pVerticalPostition;
                    mLastSceneCenter = sceneCenter;
                }

                QPointF lastSceneCenter() // Save Scene Center to replace the view when the scene becomes active
                {
                    return mLastSceneCenter;
                }

                void setLastSceneCenter(QPointF center)
                {
                    mLastSceneCenter = center;
                }

                QPointF mLastSceneCenter;

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

        virtual void setRenderingQuality(UBItem::RenderingQuality pRenderingQuality, UBItem::CacheBehavior cacheBehavior);

        QList<QUrl> relativeDependencies() const;

        QSize nominalSize();

        QSize sceneSize();

        void setNominalSize(const QSize& pSize);

        void setNominalSize(int pWidth, int pHeight);

        qreal changeZLevelTo(QGraphicsItem *item, UBZLayerController::moveDestination dest, bool addUndo=false);

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

        QSet<QGraphicsItem*> tools(){ return mTools;}

        void registerTool(QGraphicsItem* item)
        {
            mTools << item;
        }

        const QPointF& previousPoint()
        {
            return mPreviousPoint;
        }

        void setSelectedZLevel(QGraphicsItem *item);
        void setOwnZlevel(QGraphicsItem *item);

        static QUuid getPersonalUuid(QGraphicsItem *item);

        UBGraphicsPolygonItem* polygonToPolygonItem(const QPolygonF pPolygon);
        void clearSelectionFrame();
        UBBoardView *controlView();
        void notifyZChanged(QGraphicsItem *item, qreal zValue);
        void deselectAllItemsExcept(QGraphicsItem* graphicsItem);

        QRectF annotationsBoundingRect() const;

public slots:
        void updateSelectionFrame();
        void updateSelectionFrameWrapper(int);
        void initStroke();
        void hideTool();

        void setBackground(bool pIsDark, UBPageBackground pBackground);
        void setBackgroundZoomFactor(qreal zoom);
        void setBackgroundGridSize(int pSize);
        void setIntermediateLines(bool checked);
        void setDrawingMode(bool bModeDesktop);
        void deselectAllItems();

        UBGraphicsPixmapItem* addPixmap(const QPixmap& pPixmap,
            QGraphicsItem* replaceFor,
            const QPointF& pPos = QPointF(0,0),
            qreal scaleFactor = 1.0,
            bool pUseAnimation = false,
            bool useProxyForDocumentPath = false);

        void textUndoCommandAdded(UBGraphicsTextItem *textItem);

        void setToolCursor(int tool);

        void selectionChangedProcessing();
        void moveMagnifier();
        void moveMagnifier(QPoint newPos, bool forceGrab = false);
        void closeMagnifier();
        void zoomInMagnifier();
        void zoomOutMagnifier();
        void changeMagnifierMode(int mode);
        void resizedMagnifier(qreal newPercent);

        void stylusToolChanged(int tool, int previousTool);

    protected:

        UBGraphicsPolygonItem* lineToPolygonItem(const QLineF& pLine, const qreal& pWidth);
        UBGraphicsPolygonItem* lineToPolygonItem(const QLineF &pLine, const qreal &pStartWidth, const qreal &pEndWidth);

        UBGraphicsPolygonItem* arcToPolygonItem(const QLineF& pStartRadius, qreal pSpanAngle, qreal pWidth);
        UBGraphicsPolygonItem* curveToPolygonItem(const QList<QPair<QPointF, qreal> > &points);
        UBGraphicsPolygonItem* curveToPolygonItem(const QList<QPointF> &points, qreal startWidth, qreal endWidth);
        void addPolygonItemToCurrentStroke(UBGraphicsPolygonItem* polygonItem);

        void initPolygonItem(UBGraphicsPolygonItem*);

        void drawEraser(const QPointF& pEndPoint, bool pressed = true);
        void redrawEraser(bool pressed);
        void hideEraser();
        void drawPointer(const QPointF& pEndPoint, bool isFirstDraw = false);
        void drawMarkerCircle(const QPointF& pEndPoint);
        void drawPenCircle(const QPointF& pEndPoint);
        void hideMarkerCircle();
        void hidePenCircle();
        void DisposeMagnifierQWidgets();


        virtual void keyReleaseEvent(QKeyEvent * keyEvent);

        void recolorAllItems();

        virtual void drawItems (QPainter * painter, int numItems,
                               QGraphicsItem * items[], const QStyleOptionGraphicsItem options[], QWidget * widget = 0);

        QGraphicsItem* rootItem(QGraphicsItem* item) const;

        virtual void drawBackground(QPainter *painter, const QRectF &rect);


    private:
        void setDocumentUpdated();
        void createEraiser();
        void createPointer();
        void createMarkerCircle();
        void createPenCircle();
        void updateEraserColor();
        void updateMarkerCircleColor();
        void updatePenCircleColor();
        bool hasTextItemWithFocus(UBGraphicsGroupContainerItem* item);
        void simplifyCurrentStroke();

        QGraphicsEllipseItem* mEraser;
        QGraphicsEllipseItem* mPointer; // "laser" pointer
        QGraphicsEllipseItem* mMarkerCircle; // dotted circle around marker
        QGraphicsEllipseItem* mPenCircle; // dotted circle around pen

        QSet<QGraphicsItem*> mAddedItems;
        QSet<QGraphicsItem*> mRemovedItems;

        UBDocumentProxy* mDocument;

        bool mDarkBackground;
        UBPageBackground mPageBackground;
        int mBackgroundGridSize;
        bool mIntermediateLines;

        bool mIsDesktopMode;
        qreal mZoomFactor;

        QGraphicsItem* mBackgroundObject;

        QPointF mPreviousPoint;
        qreal mPreviousWidth;
        qreal mDistanceFromLastStrokePoint;

        QList<UBGraphicsPolygonItem*> mPreviousPolygonItems;

        SceneViewState mViewState;

        bool mInputDeviceIsPressed;

        QSet<QGraphicsItem*> mTools;

        UBGraphicsPolygonItem *mArcPolygonItem;

        QSize mNominalSize;

        RenderingContext mRenderingContext;

        UBGraphicsStroke* mCurrentStroke;

        int mItemCount;

        QList<QGraphicsItem*> mFastAccessItems; // a local copy as QGraphicsScene::items() is very slow in Qt 4.6


        bool mHasCache;
        //        tmp stub for divide addings scene objects from undo mechanism implementation
        bool mUndoRedoStackEnabled;

        UBMagnifier *magniferControlViewWidget;
        UBMagnifier *magniferDisplayViewWidget;

        UBZLayerController *mZLayerController;
        UBGraphicsPolygonItem* mpLastPolygon;
        UBGraphicsPolygonItem* mTempPolygon;

        bool mDrawWithCompass;
        UBGraphicsPolygonItem *mCurrentPolygon;
        UBSelectionFrame *mSelectionFrame;
};



#endif /* UBGRAPHICSSCENE_H_ */
