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


#include "UBDockDownloadWidget.h"
#include "core/UBApplication.h"

#include "globals/UBGlobals.h"

#include "core/memcheck.h"

UBDockDownloadWidget::UBDockDownloadWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent, name)
  , mpLayout(NULL)
  , mpDLWidget(NULL)
{
    mName = "DownloadWidget";
    mVisibleState = false;

    SET_STYLE_SHEET();

    mIconToLeft = QPixmap(":images/download_open.png");
    mIconToRight = QPixmap(":images/download_close.png");

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpDLWidget = new UBDownloadWidget(this);
    mpLayout->addWidget(mpDLWidget);
}

UBDockDownloadWidget::~UBDockDownloadWidget()
{
    if(NULL != mpDLWidget)
    {
        delete mpDLWidget;
        mpDLWidget = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}
