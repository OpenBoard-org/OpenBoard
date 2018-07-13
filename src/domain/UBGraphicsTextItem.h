/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBGRAPHICSTEXTITEM_H_
#define UBGRAPHICSTEXTITEM_H_

#include <QtGui>
#include "UBItem.h"
#include "core/UB.h"
#include "UBResizableGraphicsItem.h"

class UBGraphicsItemDelegate;
class UBGraphicsScene;

class UBGraphicsTextItem : public QGraphicsTextItem, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    Q_OBJECT

    public:
        UBGraphicsTextItem(QGraphicsItem * parent = 0);
        virtual ~UBGraphicsTextItem();

        enum { Type = UBGraphicsItemType::TextItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        virtual void copyItemParameters(UBItem *copy) const;

        virtual UBGraphicsScene* scene();

        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;

        // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
        bool isToolbarExtended();
        void setToolbarExtended(bool mode);
        // END Issue

        void setTextWidth(qreal width);
        void setTextHeight(qreal height);
        qreal textHeight() const;
        qreal pixelsPerPoint() const;

        void insertImage(QString src);
        void insertTable(const int lines, const int columns);
        void insertColumn(bool onRight = false);
        void insertRow(bool onRight = false);
        void deleteColumn();
        void deleteRow();
        void setCellWidth(int percent);
        void setBackgroundColor(const QColor& color);
        void setForegroundColor(const QColor& color);
        void setAlignmentToLeft();
        void setAlignmentToCenter();
        void setAlignmentToJustify(); // Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
        void setAlignmentToRight();        

        void distributeColumn();

        bool htmlMode() const;
        void setHtmlMode(const bool mode);

        void contentsChanged();

        virtual void resize(qreal width, qreal height);

        virtual QSizeF size() const;

        static QColor lastUsedTextColor;

        QColor colorOnDarkBackground() const
        {
            return mColorOnDarkBackground;
        }

        void setColorOnDarkBackground(QColor pColorOnDarkBackground)
        {
            mColorOnDarkBackground = pColorOnDarkBackground;
        }

        QColor colorOnLightBackground() const
        {
            return mColorOnLightBackground;
        }

        void setColorOnLightBackground(QColor pColorOnLightBackground)
        {
            mColorOnLightBackground = pColorOnLightBackground;
        }

        QColor backgroundColor() const
        {
            return mBackgroundColor;
        }

        virtual void clearSource(){;}
        virtual void setUuid(const QUuid &pUuid);

        //issue 1554 - NNE - 20131008
        void activateTextEditor(bool activateTextEditor);
        void setSelected(bool selected);
    signals:
        void textUndoCommandAdded(UBGraphicsTextItem *textItem);

    private slots:
        void loadUrl(QString url);
        void undoCommandAdded();
        void documentSizeChanged(const QSizeF & newSize);
        void onLinkHovered(const QString &currentLinkUrl); // ALTI/AOU - 20140602 : make possible to click on Links with Play tool

    public slots:
            void changeHTMLMode();

    private:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void keyReleaseEvent(QKeyEvent *event);

        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event);

        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        //N/C - NNE - 20140520
        QString formatTable(QString &source);
        QString removeTextBackgroundColor(QString& source);
        QString loadImages(QString& source, bool onlyLoad = false);
        QString formatParagraph(QString& source);
        QString formatList(QString& source);
        //N/C - NNE - 20140520 : END

    private:
        qreal mTextHeight;

        int mMultiClickState;
        QTime mLastMousePressTime;
        QString mTypeTextHereLabel;

        bool mHtmlIsInterpreted;

        QColor mColorOnDarkBackground;
        QColor mColorOnLightBackground;
        QColor mBackgroundColor;

        //issue 1554
        bool isActivatedTextEditor;

        // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
        bool toolbarExtended;

        // Issue 16/03/2018 - OpenBoard - Fixed width of the TEXEDITOR WINDOW to avoid problems with no-hidden icons
        qreal MIN_TEXT_WIDTH_EXTENDED = 618;
        qreal MIN_TEXT_WIDTH_REDUCED = 176;
        qreal MIN_TEXT_HEIGHT = 120;

        QString findAndReplaceAttribute(QString tag, QString oldAttribute, QString newAttribute, QString& source);

        QString mCurrentLinkUrl;

    protected:
         //issue 1539 - NNE - 20131018
         /**
           * Override beacause we have to clean the clipboard
           * (if contains html data) before paste.
           *
           * \param event The event send by Qt.
           */
         void keyPressEvent(QKeyEvent *event);

         //issue 1539 - NNE - 20131211
         /**
           * Override beacause we have to clean the clipboard
           * (if contains html data) before paste.
           *
           * \param event The event send by Qt.
           */
         void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif /* UBGRAPHICSTEXTITEM_H_ */
