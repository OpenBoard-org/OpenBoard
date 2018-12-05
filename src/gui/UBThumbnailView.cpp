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




#include "UBThumbnailView.h"
#include "domain/UBGraphicsScene.h"

#include "core/UBMimeData.h"

#include "core/memcheck.h"

UBThumbnailView::UBThumbnailView(UBGraphicsScene *scene, QWidget* parent)
    : QGraphicsView(scene, parent)
    , mHBoxLayout(new QHBoxLayout(this))
{
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

    int nominalWidth = scene->nominalSize().width();
    int nominalHeight = scene->nominalSize().height();
    QRectF nominalSceneRect(-nominalWidth/2, -nominalHeight/2, nominalWidth, nominalHeight);
    fitInView(nominalSceneRect, Qt::KeepAspectRatio);
    setSceneRect(nominalSceneRect);

    setStyleSheet( "QGraphicsView { border-style: none; }" );

    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    //set stylesheet
    setObjectName("DockPaletteWidgetBox");
    setStyleSheet("background:white");

    mHBoxLayout->setAlignment(Qt::AlignHCenter);
    setLayout(mHBoxLayout);
}
