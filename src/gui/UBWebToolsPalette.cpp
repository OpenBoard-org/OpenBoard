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




#include <QtGui>

#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"

#include "UBWebToolsPalette.h"
#include "UBResources.h"
#include "UBIconButton.h"

#include "core/memcheck.h"

UBWebToolsPalette::UBWebToolsPalette(QWidget *parent)
    : UBActionPalette(Qt::TopRightCorner, parent)
{
    QList<QAction*> actions;

    changeActions([&]{
        addAction(UBApplication::mainWindow->actionCaptureWebContent);

        addAction(UBApplication::mainWindow->actionWebCustomCapture);
        addAction(UBApplication::mainWindow->actionWebWindowCapture);
// NOTE @letsfindaway obsolete, covered by actionWebTrapFlash
//      addAction(UBApplication::mainWindow->actionWebOEmbed);

        addAction(UBApplication::mainWindow->actionWebShowHideOnDisplay);

        if (UBPlatformUtils::hasVirtualKeyboard())
            addAction(UBApplication::mainWindow->actionVirtualKeyboard);

    });
    setButtonIconSize(QSize(42, 42));
    adjustSizeAndPosition();
}


UBWebToolsPalette::~UBWebToolsPalette()
{
    // NOOP
}



