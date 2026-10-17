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




#include "UBStylusPalette.h"

#include <QtGui>

#include "UBMainWindow.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplicationController.h"
#include "core/UBShortcutManager.h"


#include "board/UBDrawingController.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/memcheck.h"

UBStylusPalette::UBStylusPalette(QWidget *parent, Qt::Orientation orient)
    : UBActionPalette(Qt::TopLeftCorner, parent, orient)
    , mLastSelectedId(-1)
{
    QList<QAction*> actions;

    actions << UBApplication::mainWindow->actionPen;
    actions << UBApplication::mainWindow->actionEraser;
    actions << UBApplication::mainWindow->actionMarker;
    actions << UBApplication::mainWindow->actionSelector;
    actions << UBApplication::mainWindow->actionPlay;

    actions << UBApplication::mainWindow->actionHand;
    actions << UBApplication::mainWindow->actionZoomIn;
    actions << UBApplication::mainWindow->actionZoomOut;

    actions << UBApplication::mainWindow->actionPointer;
    actions << UBApplication::mainWindow->actionLine;
    actions << UBApplication::mainWindow->actionText;
    actions << UBApplication::mainWindow->actionCapture;

    if(UBPlatformUtils::hasVirtualKeyboard())
    {
        actions << UBApplication::mainWindow->actionVirtualKeyboard;
        UBApplication::mainWindow->actionVirtualKeyboard->setProperty("ungrouped", true);
    }

    actions << UBApplication::mainWindow->actionSnap;
    UBApplication::mainWindow->actionSnap->setProperty("ungrouped", true);

    setActions(actions);
    groupActions();

    UBShortcutManager::shortcutManager()->addActionGroup(mActionGroup);

    UBSettings* settings = UBSettings::settings();
    connect(settings->desktopPaletteScalePercent, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));
    connect(settings->desktopPaletteShowPen, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));
    connect(settings->desktopPaletteShowEraser, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));
    connect(settings->desktopPaletteShowMarker, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));
    connect(settings->desktopPaletteShowSelector, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));
    connect(settings->desktopPaletteShowPointer, SIGNAL(changed(QVariant)),
            this, SLOT(paletteSettingsChanged(QVariant)));

    const QList<QAction*> configurableActions = {
        UBApplication::mainWindow->actionPen,
        UBApplication::mainWindow->actionEraser,
        UBApplication::mainWindow->actionMarker,
        UBApplication::mainWindow->actionSelector,
        UBApplication::mainWindow->actionPointer
    };

    for (QAction* action : configurableActions)
    {
        // Reapply palette visibility after QAction state changes.
        connect(action, &QAction::changed, this, [this]() { applyToolVisibility(); });
    }

    paletteSettingsChanged();
    initPosition();

    foreach(const UBActionPaletteButton* button, mButtons)
    {
        connect(button, SIGNAL(doubleClicked()), this, SLOT(stylusToolDoubleClicked()));
    }

}

void UBStylusPalette::paletteSettingsChanged(QVariant value)
{
    Q_UNUSED(value);

    UBSettings* settings = UBSettings::settings();
    const int percent = qBound(100, settings->desktopPaletteScalePercent->get().toInt(), 200);
    const int iconExtent = qRound(42.0 * percent / 100.0);
    setButtonIconSize(QSize(iconExtent, iconExtent));

    applyToolVisibility();
    adjustSizeAndPosition();
}

void UBStylusPalette::applyToolVisibility()
{
    UBSettings* settings = UBSettings::settings();

    const auto setButtonVisible = [this](QAction* action, bool visible) {
        if (UBActionPaletteButton* button = getButtonFromAction(action))
            button->setVisible(visible);
    };

    setButtonVisible(UBApplication::mainWindow->actionPen,
                     settings->desktopPaletteShowPen->get().toBool());
    setButtonVisible(UBApplication::mainWindow->actionEraser,
                     settings->desktopPaletteShowEraser->get().toBool());
    setButtonVisible(UBApplication::mainWindow->actionMarker,
                     settings->desktopPaletteShowMarker->get().toBool());
    setButtonVisible(UBApplication::mainWindow->actionSelector,
                     settings->desktopPaletteShowSelector->get().toBool());
    setButtonVisible(UBApplication::mainWindow->actionPointer,
                     settings->desktopPaletteShowPointer->get().toBool());
}

int UBStylusPalette::buttonSpacing() const
{
    // Button size follows the palette scale, so the gaps grow with it. Keep
    // them tight and configurable instead.
    return qBound(0, UBSettings::settings()->paletteButtonSpacingPx->get().toInt(), 30);
}

void UBStylusPalette::initPosition()
{
    QWidget* pParentW = parentWidget();
    if(!pParentW) return ;

    mCustomPosition = true;

    QPoint pos;
    int parentWidth = pParentW->width();
    int parentHeight = pParentW->height();

    if(UBSettings::settings()->appToolBarOrientationVertical->get().toBool()){
        int posX = border();
        int posY = (parentHeight / 2) - (height() / 2);
        pos.setX(posX);
        pos.setY(posY);
    }
    else {
        int posX = (parentWidth / 2) - (width() / 2);
        int posY = parentHeight - border() - height();
        pos.setX(posX);
        pos.setY(posY);
    }
    moveInsideParent(pos);
}

UBStylusPalette::~UBStylusPalette()
{
    if (mActionGroup)
    {
        UBShortcutManager::shortcutManager()->removeActionGroup(mActionGroup);
    }
}

void UBStylusPalette::stylusToolDoubleClicked()
{
    emit stylusToolDoubleClicked(mActionGroup->checkedAction()->property("id").toInt());
}
