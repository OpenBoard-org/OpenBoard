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
    eraserCursor    = QCursor(QPixmap(":/images/cursors/eraser.png"), 5, 25);
    markerCursor    = QCursor(QPixmap(":/images/cursors/marker.png"), 3, 30);
    pointerCursor   = QCursor(QPixmap(":/images/cursors/laser.png"), 2, 1);
    handCursor      = QCursor(Qt::OpenHandCursor);
    zoomInCursor    = QCursor(QPixmap(":/images/cursors/zoomIn.png"), 9, 9);
    zoomOutCursor   = QCursor(QPixmap(":/images/cursors/zoomOut.png"), 9, 9);
    arrowCursor     = QCursor(Qt::ArrowCursor);
    playCursor      = QCursor(QPixmap(":/images/cursors/play.png"), 6, 1);
    textCursor      = QCursor(Qt::ArrowCursor);
    rotateCursor    = QCursor(QPixmap(":/images/cursors/rotate.png"), 16, 16);
    drawLineRulerCursor = QCursor(QPixmap(":/images/cursors/drawRulerLine.png"), 3, 12);
}

void UBResources::buildFontList()
{
    QString customFontDirectory = UBSettings::settings()->applicationCustomFontDirectory();
    QStringList fontFiles = UBFileSystemUtils::allFiles(customFontDirectory);
    foreach(QString fontFile, fontFiles){
        int fontId = QFontDatabase::addApplicationFont(fontFile);
        mCustomFontList << QFontDatabase::applicationFontFamilies(fontId);
    }
}
