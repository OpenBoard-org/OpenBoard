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

#include <QVector>

#include "UBAbstractHandle.h"

// forward
class UBEditShapeUndoCommand;


class UBAbstractEditable
{
public:
    explicit UBAbstractEditable();
    virtual ~UBAbstractEditable();

    void addHandle(UBAbstractHandle* mhandle);

    UBAbstractHandle* getHandle(HandleId id) const;

    void showEditMode(bool show);

    virtual void updateHandle(UBAbstractHandle* handle) = 0;

    virtual void deactivateEditionMode();

    virtual void focusHandle(UBAbstractHandle* handle)
    {
        Q_UNUSED(handle)
    }

    bool isInEditMode() const;

    void setModified();

protected:
    void addHandleShapes(const QGraphicsItem* item, QPainterPath& path) const;

protected:
    QVector<UBAbstractHandle*> mHandles;

private:
    bool mEditMode{false};
    bool mModified{false};
    UBEditShapeUndoCommand* mUndoCommand{nullptr};
};
