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


#ifndef UBCACHEPROPERTIESWIDGET_H
#define UBCACHEPROPERTIESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QVector>

#include "UBDockPaletteWidget.h"
#include "tools/UBGraphicsCache.h"

#define MAX_SHAPE_WIDTH     200

class UBCachePropertiesWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBCachePropertiesWidget(QWidget* parent=0, const char* name="UBCachePropertiesWidget");
    ~UBCachePropertiesWidget();

    bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD;
    }

public slots:
    void updateCurrentCache();

signals:
    void cacheListEmpty();

private slots:
    void onCloseClicked();
    void updateCacheColor(QColor color);
    void onColorClicked();
    void updateShapeButtons();
    void onSizeChanged(int newSize);
    void onCacheEnabled();

private:
    QVBoxLayout* mpLayout;
    QLabel* mpCachePropertiesLabel;
    QLabel* mpColorLabel;
    QLabel* mpShapeLabel;
    QLabel* mpSizeLabel;
    QPushButton* mpColor;
    QPushButton* mpSquareButton;
    QPushButton* mpCircleButton;
    QPushButton* mpCloseButton;
    QSlider* mpSizeSlider;
    QHBoxLayout* mpColorLayout;
    QHBoxLayout* mpShapeLayout;
    QHBoxLayout* mpSizeLayout;
    QHBoxLayout* mpCloseLayout;
    QWidget* mpProperties;
    QVBoxLayout* mpPropertiesLayout;
    QColor mActualColor;
    eMaskShape mActualShape;
    UBGraphicsCache* mpCurrentCache;

};

#endif // UBCACHEPROPERTIESWIDGET_H
