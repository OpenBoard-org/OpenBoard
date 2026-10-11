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

#include "domain/UBItemStyle.h"


/**
 * @brief The UBStyled Item class describes a graphical element to which a style can be assigned.
 * A style includes, in particular, a line color, but may also optionally include a line width,
 * a line style, and a fill color.
 */
class UBStyledItem
{
public:
    UBStyledItem();

    virtual UBItemStyle itemStyle() const;
    void setItemStyle(const UBItemStyle& style);

    virtual void applyItemStyle(const UBItemStyle& style, bool isDark) = 0;
    virtual bool isShape() const = 0;
    virtual bool isMarker() const = 0;

protected:
    UBItemStyle mItemStyle;
};
