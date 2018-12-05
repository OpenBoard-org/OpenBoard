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




#include <QColor>
#include <QPainter>
#include <QPixmap>
#include <QColorDialog>

#include "UBCachePropertiesWidget.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

static QVector<UBGraphicsCache*> mCaches;

UBCachePropertiesWidget::UBCachePropertiesWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
  , mpLayout(NULL)
  , mpCachePropertiesLabel(NULL)
  , mpColorLabel(NULL)
  , mpShapeLabel(NULL)
  , mpSizeLabel(NULL)
  , mpColor(NULL)
  , mpSquareButton(NULL)
  , mpCircleButton(NULL)
  , mpCloseButton(NULL)
  , mpSizeSlider(NULL)
  , mpColorLayout(NULL)
  , mpShapeLayout(NULL)
  , mpSizeLayout(NULL)
  , mpCloseLayout(NULL)
  , mpProperties(NULL)
  , mpPropertiesLayout(NULL)
  , mpCurrentCache(NULL)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mName = "CachePropWidget";
    mVisibleState = false;
    mIconToLeft = QPixmap(":images/cache_open.png");
    mIconToRight = QPixmap(":images/cache_close.png");
    setContentsMargins(10, 10, 10, 10);

    // Build the UI
    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    // Title
    mpCachePropertiesLabel = new QLabel(tr("Cache Properties"), this);
    mpCachePropertiesLabel->setObjectName("DockPaletteWidgetTitle");
    mpLayout->addWidget(mpCachePropertiesLabel, 0);

    // Properties Box
    mpProperties = new QWidget(this);
    mpProperties->setObjectName("DockPaletteWidgetBox");
    mpLayout->addWidget(mpProperties, 1);
    mpPropertiesLayout = new QVBoxLayout();
    mpProperties->setLayout(mpPropertiesLayout);


    // Color
    mpColorLayout = new QHBoxLayout();
    mpColorLabel = new QLabel(tr("Color:"), mpProperties);
    mpColor = new QPushButton(mpProperties);
    mpColor->setObjectName("DockPaletteWidgetButton");
    updateCacheColor(Qt::black);
    mpColorLayout->addWidget(mpColorLabel, 0);
    mpColorLayout->addWidget(mpColor, 0);
    mpColorLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpColorLayout, 0);

    // Shape
    mpShapeLayout = new QHBoxLayout();
    mpShapeLabel = new QLabel(tr("Shape:"), mpProperties);
    mpSquareButton = new QPushButton(mpProperties);
    mpSquareButton->setIcon(QIcon(":images/cache_square.png"));
    mpSquareButton->setObjectName("DockPaletteWidgetButton");
    mpSquareButton->setCheckable(true);
    mpCircleButton = new QPushButton(mpProperties);
    mpCircleButton->setIcon(QIcon(":images/cache_circle.png"));
    mpCircleButton->setObjectName("DockPaletteWidgetButton");
    mpCircleButton->setCheckable(true);
    mpShapeLayout->addWidget(mpShapeLabel, 0);
    mpShapeLayout->addWidget(mpSquareButton, 0);
    mpShapeLayout->addWidget(mpCircleButton, 0);
    mpShapeLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpShapeLayout, 0);

    mpCircleButton->setChecked(true);

    // Shape Size
    mpSizeLayout = new QHBoxLayout();
    mpSizeLabel = new QLabel(tr("Size:"), mpProperties);
    mpSizeSlider = new QSlider(Qt::Horizontal, mpProperties);
    mpSizeSlider->setMinimumHeight(20);
    mpSizeSlider->setMinimum(50);
    mpSizeSlider->setMaximum(MAX_SHAPE_WIDTH);
    mpSizeLayout->addWidget(mpSizeLabel, 0);
    mpSizeLayout->addWidget(mpSizeSlider, 1);
    mpPropertiesLayout->addLayout(mpSizeLayout, 0);

    // Close
    mpCloseLayout =  new QHBoxLayout();
    mpCloseButton = new QPushButton(tr("Close"), mpProperties);
    mpCloseButton->setObjectName("DockPaletteWidgetButton");
    mpCloseLayout->addWidget(mpCloseButton, 0);
    mpCloseLayout->addStretch(1);
    mpPropertiesLayout->addLayout(mpCloseLayout, 0);

    // Fill the empty space
    mpPropertiesLayout->addStretch(1);

    // Connect signals / slots
    connect(mpCloseButton, SIGNAL(clicked()), this, SLOT(onCloseClicked()));
    connect(mpColor, SIGNAL(clicked()), this, SLOT(onColorClicked()));
    connect(mpCircleButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpSquareButton, SIGNAL(clicked()), this, SLOT(updateShapeButtons()));
    connect(mpSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSizeChanged(int)));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(updateCurrentCache()));
    connect(UBApplication::boardController, SIGNAL(cacheEnabled()), this, SLOT(onCacheEnabled()));
}

UBCachePropertiesWidget::~UBCachePropertiesWidget()
{
    if(NULL != mpCachePropertiesLabel)
    {
        delete mpCachePropertiesLabel;
        mpCachePropertiesLabel = NULL;
    }
    if(NULL != mpColorLabel)
    {
        delete mpColorLabel;
        mpColorLabel = NULL;
    }
    if(NULL != mpShapeLabel)
    {
        delete mpShapeLabel;
        mpShapeLabel = NULL;
    }
    if(NULL != mpSizeLabel)
    {
        delete mpSizeLabel;
        mpSizeLabel = NULL;
    }
    if(NULL != mpColor)
    {
        delete mpColor;
        mpColor = NULL;
    }
    if(NULL != mpSquareButton)
    {
        delete mpSquareButton;
        mpSquareButton = NULL;
    }
    if(NULL != mpCircleButton)
    {
        delete mpCircleButton;
        mpCircleButton = NULL;
    }
    if(NULL != mpCloseButton)
    {
        delete mpCloseButton;
        mpCloseButton = NULL;
    }
    if(NULL != mpSizeSlider)
    {
        delete mpSizeSlider;
        mpSizeSlider = NULL;
    }
    if(NULL != mpColorLayout)
    {
        delete mpColorLayout;
        mpColorLayout = NULL;
    }
    if(NULL != mpShapeLayout)
    {
        delete mpShapeLayout;
        mpShapeLayout = NULL;
    }
    if(NULL != mpSizeLayout)
    {
        delete mpSizeLayout;
        mpSizeLayout = NULL;
    }
    if(NULL != mpCloseLayout)
    {
        delete mpCloseLayout;
        mpCloseLayout = NULL;
    }
    if(NULL != mpPropertiesLayout)
    {
        delete mpPropertiesLayout;
        mpPropertiesLayout = NULL;
    }
    if(NULL != mpProperties)
    {
        delete mpProperties;
        mpProperties = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBCachePropertiesWidget::onCloseClicked()
{
    if(!mCaches.empty())
    {
        // Remove the current cache from the list
        mCaches.remove(mCaches.indexOf(mpCurrentCache));

        // Remove the cache from the board
        UBApplication::boardController->activeScene()->removeItem(mpCurrentCache);
        mpCurrentCache = NULL;

        emit hideTab(this);
    }
}

void UBCachePropertiesWidget::updateCacheColor(QColor color)
{
    mActualColor = color;

    //  Update the color on the color button
    QPixmap pix(32, 32);
    QPainter p;

    p.begin(&pix);

    p.setBackground(Qt::transparent);
    p.setBrush(color);      // The current color
    p.drawRect(0, 0, 32, 32);

    p.end();

    mpColor->setIcon(QIcon(pix));

    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setMaskColor(mActualColor);
    }
}

void UBCachePropertiesWidget::onColorClicked()
{
    // Show the color picker
    QColor newColor = QColorDialog::getColor(mActualColor,this);
    updateCacheColor(newColor);
}

void UBCachePropertiesWidget::updateShapeButtons()
{
    if(mpCircleButton->hasFocus())
    {
        mActualShape = eMaskShape_Circle;
        mpSquareButton->setChecked(false);
    }
    else if(mpSquareButton->hasFocus())
    {
        mActualShape = eMaskShap_Rectangle;
        mpCircleButton->setChecked(false);
    }

    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setMaskShape(mActualShape);
    }
}

void UBCachePropertiesWidget::updateCurrentCache()
{
    bool isBoardMode = false;
    // this widget can work only on Board mode
    if( UBApplication::applicationController != NULL )
    {
        // if app controller is available, and current mode is Board, and no show desktop, than all ok, just process
        if( UBApplication::applicationController->displayMode() == UBApplicationController::Board &&
            !UBApplication::applicationController->isShowingDesktop())
            isBoardMode = true;
    }
    // if app controller == null, than we do not know what mode now, so just process
    else
        isBoardMode = true;

    if(isBoardMode)
    {
        // Get the current page cache
        QList<QGraphicsItem*> items = UBApplication::boardController->activeScene()->items();
        foreach(QGraphicsItem* it, items)
        {
            if("Cache" == it->data(Qt::UserRole).toString())
            {
                setEnabled(true);
                emit showTab(this);
                mpCurrentCache = dynamic_cast<UBGraphicsCache*>(it);
                if((NULL != mpCurrentCache) && (!mCaches.contains(mpCurrentCache)))
                {
                    mCaches.append(mpCurrentCache);
                }

                // Update the values of the cache properties
                mpSizeSlider->setValue(mpCurrentCache->shapeWidth());
                updateCacheColor(mpCurrentCache->maskColor());
                switch(mpCurrentCache->maskshape())
                {
                    case eMaskShape_Circle:
                        mpCircleButton->setChecked(true);
                        mpSquareButton->setChecked(false);
                        break;
                    case eMaskShap_Rectangle:
                        mpCircleButton->setChecked(false);
                        mpSquareButton->setChecked(true);
                        break;
                }

                return;
            }
        }
    }

    // If we fall here, that means:
    // 1 - that this page has no cache
    // 2 - we are not in Board mode
    // 3 - we are in Board mode, but show desktop (as really - Desktop mode)
    emit hideTab(this);
    mpCurrentCache = NULL;
    setDisabled(true);
}

void UBCachePropertiesWidget::onSizeChanged(int newSize)
{
    if(NULL != mpCurrentCache)
    {
        mpCurrentCache->setShapeWidth(newSize);
    }
}

void UBCachePropertiesWidget::onCacheEnabled()
{
    emit showTab(this);
}
