/*
 * Copyright (C) 2015-2026 Département de l'Instruction Publique (DIP-SEM)
 * and contributors.
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


#pragma once

#include "UBToolbarExtensionPalette.h"
#include "ui_pageSize.h"

#include "core/UB.h"
#include "document/UBDocumentProxy.h"

// forward
class UBDocument;
class UBDocumentContainer;
class QToolBar;
class QWidget;


class UBPageSizePalette : public UBToolbarExtensionPalette, public Ui::PageSize
{
    Q_OBJECT

public:
    UBPageSizePalette(QToolBar* toolBar, UBDocumentContainer* container, QWidget* parent);

protected:
     void showEvent(QShowEvent *event) override;

private slots:
    void showOnToolbar();
    void setDocument(std::shared_ptr<UBDocumentProxy> proxy);
    void selectionChanged();
    void selectFormat();
    void switchToCustom();
    void flip(bool portrait);
    void apply();
    void undo();
    void redo();

private:
    void updateSpinBoxes(bool isRatio);
    static DocumentSize::Enum formatFromSize(QSize size, bool& portrait);

private:
    UBDocumentContainer* mContainer;
    std::shared_ptr<UBDocument> mDocument;
    DocumentSize::Enum mFormat;
    DocumentSize::Value mValue;
    bool mPortrait;
    bool mIsUpdatingSpinBoxes{false};
};

