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


#ifndef UBSTYLUSPALLETTE_H_
#define UBSTYLUSPALLETTE_H_

#include <QButtonGroup>

#include "UBActionPalette.h"


class UBStylusPalette : public UBActionPalette
{
    Q_OBJECT

    public:
        UBStylusPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        virtual ~UBStylusPalette();

        void initPosition();

    private slots:

        void stylusToolDoubleClicked();

    private:
        int mLastSelectedId;

    signals:
        void stylusToolDoubleClicked(int tool);
};

#endif /* UBSTYLUSPALLETTE_H_ */
