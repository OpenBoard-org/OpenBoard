/* Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique    (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBOPENSANKOREIMPORTERWIDGET_H
#define UBOPENSANKOREIMPORTERWIDGET_H

class QCheckBox;
class QPushButton;

#include "UBFloatingPalette.h"

class UBOpenSankoreImporterWidget : public UBFloatingPalette
{
    Q_OBJECT

public:
    UBOpenSankoreImporterWidget(QWidget* parent);
    QPushButton* proceedButton(){return mProceedButton;}

protected:
    void showEvent(QShowEvent *event);
    int border();

    QCheckBox* mDisplayOnNextRestart;
    QPushButton* mProceedButton;

private slots:
    void onNextRestartCheckBoxClicked(bool clicked);


};

#endif // UBOPENSANKOREIMPORTERWIDGET_H
