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

#include "UBZoomPalette.h"

#include <QtGui>
#include <math.h>

#include "core/UBApplication.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

UBZoomPalette::UBZoomPalette(QWidget* parent)
    : UBFloatingPalette(Qt::BottomRightCorner, parent)
    , mIsExpanded(1)
{
    mBoardController = UBApplication::boardController;
    QLayout* layout = new QVBoxLayout(this);
    mCurrentZoomButton = new QPushButton(parent);
    mCurrentZoomButton->setStyleSheet(QString("QPushButton { color: white; background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    mCurrentZoomButton->setFocusPolicy(Qt::NoFocus);
    connect(mCurrentZoomButton, SIGNAL(clicked(bool)), this, SLOT(showHideExtraButton()));
    connect(mBoardController, SIGNAL(zoomChanged(qreal)), this, SLOT(refreshPalette()));
    connect(mBoardController, SIGNAL(activeSceneChanged()), this, SLOT(refreshPalette()));

    mHundredButton = new QPushButton(parent);
    mHundredButton->setStyleSheet(QString("QPushButton { color: white; background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    mHundredButton->setFocusPolicy(Qt::NoFocus);
    mHundredButton->setIcon(QIcon(":/images/stylusPalette/restoreZoom.png"));
    mHundredButton->setIconSize(QSize(42,42));
    connect(mHundredButton, SIGNAL(clicked(bool)), this, SLOT(goHundred()));

    layout->setContentsMargins(radius() + 15, 4, radius() + 15, 4);
    layout->addWidget(mHundredButton);
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
    if (mIsExpanded)
    {
        mHundredButton->setVisible(false);
        mIsExpanded = false;
        adjustSizeAndPosition(false);
    }
    else
    {
        mHundredButton->setVisible(true);
        mIsExpanded = true;
        adjustSizeAndPosition(true);
    }

}

void UBZoomPalette::goHundred()
{
    mBoardController->zoomRestore();
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
    qreal currentZoomFactor = mBoardController->currentZoom();
    bool showAsNoZoom = (currentZoomFactor > 0.9 && currentZoomFactor < 1.1);
    if (showAsNoZoom)
    {
        currentZoomFactor = 1;
    }
    QString stringFactor = tr("%1 x").arg(currentZoomFactor, 0, 'f', 1);

    mCurrentZoomButton->setText(stringFactor);
    adjustSizeAndPosition();
    if (showAsNoZoom)
    {
        QTimer::singleShot(500, this, SLOT(hide()));
    }
    else
    {
        show();
    }
}
