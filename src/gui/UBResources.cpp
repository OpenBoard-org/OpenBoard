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




#include "UBResources.h"

#include <QtGui>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/memcheck.h"


UBResources* UBResources::sSingleton = 0;

UBResources::UBResources(QObject* pParent)
 : QObject(pParent)
{
    // NOOP
}

UBResources::~UBResources()
{
    // NOOP
}

UBResources* UBResources::resources()
{
    if (!sSingleton)
    {
        sSingleton = new UBResources(UBApplication::staticMemoryCleaner);
        sSingleton->init();
        sSingleton->buildFontList();
    }

    return sSingleton;

}

void UBResources::init()
{
    // Cursors
    penCursor       = QCursor(Qt::CrossCursor);
    eraserCursor    = QCursor(QPixmap(":/images/cursors/eraser.svg"), 5, 25);
    markerCursor    = QCursor(QPixmap(":/images/cursors/marker.svg"), 3, 30);
    pointerCursor   = QCursor(QPixmap(":/images/cursors/laser.svg"), 2, 1);
    handCursor      = QCursor(QPixmap(":/images/cursors/scrollPage.svg"), 6, 1);
    closedHandCursor= QCursor(QPixmap(":/images/cursors/scrollPageOn.svg"), 6, 1);
    zoomInCursor    = QCursor(QPixmap(":/images/cursors/zoomIn.svg"), 9, 9);
    zoomOutCursor   = QCursor(QPixmap(":/images/cursors/zoomOut.svg"), 9, 9);
    arrowCursor     = QCursor(Qt::ArrowCursor);
    playCursor      = QCursor(QPixmap(":/images/cursors/play.svg"), 6, 1);
    textCursor      = QCursor(Qt::ArrowCursor);
    rotateCursor    = QCursor(QPixmap(":/images/cursors/rotate.svg"), 16, 16);
    drawLineRulerCursor = QCursor(QPixmap(":/images/cursors/drawRulerLine.svg"), 3, 12);
}

void UBResources::buildFontList()
{
    QString customFontDirectory = UBSettings::settings()->applicationCustomFontDirectory();
    QStringList fontFiles = UBFileSystemUtils::allFiles(customFontDirectory);
    foreach(QString fontFile, fontFiles){
        int fontId = QFontDatabase::addApplicationFont(fontFile);
        mCustomFontList << QFontDatabase::applicationFontFamilies(fontId);
    }

    mCustomFontList.removeDuplicates();
}
