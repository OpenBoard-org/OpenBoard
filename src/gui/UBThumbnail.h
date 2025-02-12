/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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


#pragma once

#include <QGraphicsRectItem>

// forward
class UBGraphicsScene;
class UBThumbnailTextItem;

/**
 * @brief The UBThumbnail class represents the graphics item of a thumbnail.
 *
 * It is a group consisting of an invisible rectangle containing the thumbnail pixmap
 * and a text item below that for the page number.
 *
 * setThumbnailSize sets the size of the invisible rectangle, scales the pixmap and
 * arranges pixmap and caption so that both are centered in the rectangle.
 */
class UBThumbnail : public QGraphicsRectItem
{
public:
    UBThumbnail();

    void setPixmap(const QPixmap& pixmap);
    QPixmap pixmap() const;
    void setSceneIndex(int sceneIndex);
    int sceneIndex() const;
    void setThumbnailSize(QSizeF size);

    void setColumn(int column);
    void setRow(int row);

    int column() const;
    int row() const;

    void setPageScene(std::shared_ptr<UBGraphicsScene> pageScene);
    void setExposed(bool exposed);
    bool isExposed() const;
    void setDeletable(bool deletable);

    void updatePixmap(const QRectF& region = QRectF());
    void adjustThumbnail();

    static double heightForWidth(double width);

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    static int labelSpacing();

private:
    QGraphicsPixmapItem* mPixmapItem{nullptr};
    UBThumbnailTextItem* mTextItem{nullptr};
    int mIndex{0};
    int mColumn{0};
    int mRow{0};
    std::weak_ptr<UBGraphicsScene> mPageScene{}; // weak, don't keep the scene from being deleted
    bool mExposed{false};
    QTransform mTransform{};
    bool mEditable{false};
    bool mDeletable{true};
};

namespace UBThumbnailUI
{
constexpr int ICONSIZE = 32;
constexpr int ICONSPACING = 10;

class UBThumbnailUIIcon : public QPixmap
{
public:
    UBThumbnailUIIcon(const QString& filename, int pos);

    int pos() const;
    bool triggered(QPointF p) const;

private:
    const int mPos;
};

namespace _private
{
// do not use this directly
static QMap<QString, UBThumbnailUIIcon*> catalog;
void initCatalog();
} // namespace _private

UBThumbnailUIIcon* addIcon(const QString& thumbnailIcon, int pos);
UBThumbnailUIIcon* getIcon(const QString& thumbnailIcon);
void draw(QPainter* painter, const UBThumbnailUIIcon& thumbnailIcon);
} // namespace UBThumbnailUI
