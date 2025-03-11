/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBDOCUMENTTHUMBNAILSVIEW_H_
#define UBDOCUMENTTHUMBNAILSVIEW_H_

#include <QtGui>
#include <QtSvg>
#include <QElapsedTimer>
#include <QGraphicsTextItem>

#include "core/UBSettings.h"

#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailsView.h"


#define STARTDRAGTIME   1000000

class UBDocument;
class UBThumbnail;

class UBDocumentThumbnailsView : public UBThumbnailsView
{
    Q_OBJECT

    public:
        UBDocumentThumbnailsView(QWidget* parent);
        virtual ~UBDocumentThumbnailsView();

        void setDocument(std::shared_ptr<UBDocument> document);
        std::shared_ptr<UBDocument> document() const;
        QList<QGraphicsItem*> selectedItems();
        void selectItemAt(int pIndex, bool extend = false);
        void clearSelection();

        qreal thumbnailWidth()
        {
            return mThumbnailWidth;
        }

        void setBackgroundBrush(const QBrush& brush);

    public slots:
        void setThumbnailWidth(qreal pThumbnailWidth);
        void sceneSelectionChanged();

    signals:
        void resized();
        void selectionChanged();
        void thumbnailWidthChanged(int thumbnailWidth);
        void mouseDoubleClick(QGraphicsItem* item, int index);
        void mouseClick(QGraphicsItem* item, int index);


    protected:
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void resizeEvent(QResizeEvent * event);
        void mouseDoubleClickEvent(QMouseEvent * event);

        virtual void keyPressEvent(QKeyEvent *event);
        virtual void focusInEvent(QFocusEvent *event);

        QPoint mMousePressPos;

    protected:
        QList<QUrl> mItemsPaths;
        bool bSelectionInProgress;
        bool bCanDrag;

    private:
        void selectAll();
        void selectItems(int startIndex, int count);
        int rowCount() const;
        int columnCount() const;

        static bool thumbnailLessThan(QGraphicsItem* item1, QGraphicsItem* item2);

    private:
        std::shared_ptr<UBDocument> mDocument{nullptr};

        QString mMimeType;

        QPointF prevMoveMousePos;

        qreal mThumbnailWidth;

        int mSelectionSpan;
        QElapsedTimer mClickTime;

        /**
         * @brief The UBDocumentThumbnailArranger class is the arranger for Document mode.
         */
        class UBDocumentThumbnailArranger : public UBThumbnailArranger
        {
        public:
            UBDocumentThumbnailArranger(UBDocumentThumbnailsView* thumbnailView);

            virtual int columnCount() const override;
            virtual double thumbnailWidth() const override;

        private:
            void setThumbnailWidth(int width);

        private:
            double mThumbnailWidth{double(UBSettings::defaultThumbnailWidth)};
        };
};


class UBThumbnailTextItem : public QGraphicsTextItem
{
    Q_OBJECT

    public:
        UBThumbnailTextItem()
            : QGraphicsTextItem()
        {
        }

        UBThumbnailTextItem(int index)
            : QGraphicsTextItem()
            , mUnelidedText(toPlainText())
        {
            setPageNumber(index + 1);
        }

        UBThumbnailTextItem(const QString& text)
            : QGraphicsTextItem(text)
            , mUnelidedText(text)
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
        }

        QRectF boundingRect() const { return QRectF(QPointF(0.0, 0.0), QSize(mWidth, QFontMetricsF(font()).height() + 5));}

        void setWidth(qreal pWidth)
        {
            if (mWidth != pWidth)
            {
                prepareGeometryChange();
                mWidth = pWidth;
                computeText();

                // center text
                setTextWidth(mWidth);
                document()->setDefaultTextOption(QTextOption(Qt::AlignCenter));
            }
        }

        qreal width() {return mWidth;}

        void setPageNumber(int i)
        {
            mUnelidedText = tr("Page %0").arg(i);
            computeText();
        }

        void setText(const QString& text)
        {
            mUnelidedText = text;
            computeText();
        }

        void computeText()
        {
            QFontMetricsF fm(font());
            QString elidedText = fm.elidedText(mUnelidedText, Qt::ElideLeft, mWidth - 2 * document()->documentMargin() - 1);
            setPlainText(elidedText);
        }

    private:
        qreal mWidth{0};
        QString mUnelidedText{};
};


#endif /* UBDOCUMENTTHUMBNAILSVIEW_H_ */
