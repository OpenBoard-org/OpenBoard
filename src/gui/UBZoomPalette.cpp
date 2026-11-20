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




#include "UBZoomPalette.h"

#include <QtGui>
#include <QPushButton>
#include <math.h>

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"

#include "core/memcheck.h"

UBZoomPalette::UBZoomPalette(QWidget* parent)
    : UBFloatingPalette(Qt::BottomRightCorner, parent)
{
    mBoardController = UBApplication::boardController;
    QLayout* layout = new QVBoxLayout(this);
    mCurrentZoomButton = new QPushButton(parent);
    mCurrentZoomButton->setStyleSheet(QString("QPushButton { background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    mCurrentZoomButton->setFocusPolicy(Qt::NoFocus);
    connect(mCurrentZoomButton, SIGNAL(clicked(bool)), this, SLOT(showHideExtraButton()));
    connect(mBoardController, SIGNAL(zoomChanged(qreal)), this, SLOT(refreshPalette()));
    connect(mBoardController, SIGNAL(activeSceneChanged()), this, SLOT(refreshPalette()));
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this, &UBZoomPalette::refreshPalette);

    mZoom100 = new QToolButton(this);
    mZoom100->setIcon(QIcon(":/images/zoomPalette/zoom100.svg"));
    mZoom100->setIconSize({48, 36});
    mZoom100->setToolTip(tr("100%"));
    connect(mZoom100, &QToolButton::clicked, this, &UBZoomPalette::goHundred);

    mZoomFit = new QToolButton(this);
    mZoomFit->setIcon(QIcon(":/images/zoomPalette/zoomFit.svg"));
    mZoomFit->setIconSize({48, 36});
    mZoomFit->setToolTip(tr("Fit page"));
    connect(mZoomFit, &QToolButton::clicked, this, &UBZoomPalette::goFit);

    mZoomFitContent = new QToolButton(this);
    mZoomFitContent->setIcon(QIcon(":/images/zoomPalette/zoomFitContent.svg"));
    mZoomFitContent->setIconSize({48, 36});
    mZoomFitContent->setToolTip(tr("Fit content"));
    connect(mZoomFitContent, &QToolButton::clicked, this, &UBZoomPalette::goFitContent);

    mZoomWidth = new QToolButton(this);
    mZoomWidth->setIcon(QIcon(":/images/zoomPalette/zoomWidth.svg"));
    mZoomWidth->setIconSize({48, 36});
    mZoomWidth->setToolTip(tr("Fit width"));
    connect(mZoomWidth, &QToolButton::clicked, this, &UBZoomPalette::goWidth);

    mZoomWidthTop = new QToolButton(this);
    mZoomWidthTop->setIcon(QIcon(":/images/zoomPalette/zoomWidthTop.svg"));
    mZoomWidthTop->setIconSize({48, 36});
    mZoomWidthTop->setToolTip(tr("Fit width, scroll to top"));
    connect(mZoomWidthTop, &QToolButton::clicked, this, &UBZoomPalette::goWidthTop);

    QGridLayout* grid = new QGridLayout(this);
    grid->addWidget(mZoom100, 0, 0, 1, 2);
    grid->addWidget(mZoomFit, 0, 2, 1, 2);
    grid->addWidget(mZoomFitContent, 0, 4, 1, 2);
    grid->addWidget(mZoomWidth, 1, 1, 1, 2);
    grid->addWidget(mZoomWidthTop, 1, 3, 1, 2);

    layout->setContentsMargins(radius(), 4, radius(), 4);
    layout->addItem(grid);
    layout->addWidget(mCurrentZoomButton);

    hide();
    refreshPalette();
}

UBZoomPalette::~UBZoomPalette()
{
    delete mCurrentZoomButton;
}

void UBZoomPalette::showHideExtraButton()
{
    mIsExpanded = !mIsExpanded;

    mZoom100->setVisible(mIsExpanded);
    mZoomFit->setVisible(mIsExpanded);
    mZoomFitContent->setVisible(mIsExpanded);
    mZoomWidth->setVisible(mIsExpanded);
    mZoomWidthTop->setVisible(mIsExpanded);

    if (!mIsExpanded)
    {
        resize(layout()->sizeHint());
    }

    adjustSizeAndPosition(mIsExpanded);
}

void UBZoomPalette::goHundred()
{
    mBoardController->zoomRestore();
    UBSettings::settings()->boardInitialViewState->set(ViewState::OriginalSize);
}

void UBZoomPalette::goFit()
{
    mBoardController->zoomToFit();
    UBSettings::settings()->boardInitialViewState->set(ViewState::FitSize);
}

void UBZoomPalette::goFitContent()
{
    mBoardController->zoomTo(ViewState::FitContent);
    UBSettings::settings()->boardInitialViewState->set(ViewState::FitContent);
}

void UBZoomPalette::goWidth()
{
    mBoardController->zoomTo(ViewState::FitWidth);
    UBSettings::settings()->boardInitialViewState->set(ViewState::FitWidth);
}

void UBZoomPalette::goWidthTop()
{
    mBoardController->zoomTo(ViewState::FitWidthScrollToTop);
    UBSettings::settings()->boardInitialViewState->set(ViewState::FitWidthScrollToTop);
}

void UBZoomPalette::hide()
{
    qreal currentZoomFactor = mBoardController->currentZoom();

    bool showAsNoZoom = (currentZoomFactor > 0.9 && currentZoomFactor < 1.1);
    if (showAsNoZoom)
    {
        UBFloatingPalette::hide();
    }
}

void UBZoomPalette::refreshPalette()
{
    const qreal currentZoomFactor = mBoardController->currentZoom();
    const bool showAsNoZoom = (currentZoomFactor > 0.9 && currentZoomFactor < 1.1);
    const QString stringFactor = tr("%1 x").arg(currentZoomFactor, 0, 'f', 2);

    mCurrentZoomButton->setText(stringFactor);
    adjustSizeAndPosition();

    const auto tool = UBDrawingController::drawingController()->stylusTool();
    const auto zoomTool = tool == UBStylusTool::ZoomIn || tool == UBStylusTool::ZoomOut;

    if (showAsNoZoom && !zoomTool)
    {
        QTimer::singleShot(500, this, SLOT(hide()));
    }
    else
    {
        show();
        raise();
    }
}

int UBZoomPalette::border()
{
    return 5;
}
