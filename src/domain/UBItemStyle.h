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

#include <QColor>

class UBItemStyle
{
public:
    UBItemStyle() = default;
    UBItemStyle(const QColor& lineColorOnLight, const QColor& lineColorOnDark, qreal lineWidth, Qt::PenStyle lineStyle,
                const QColor& fillColorOnLight, const QColor& fillColorOnDark);

    QColor lineColor(bool isDark) const;
    void setLineColor(const QColor& lineColorOnLight, const QColor& lineColorOnDark);

    double lineWidth() const;
    void setLineWidth(double lineWidth);

    Qt::PenStyle lineStyle() const;
    void setLineStyle(Qt::PenStyle lineStyle);

    QColor fillColor(bool isDark) const;
    void setFillColor(const QColor& fillColorOnLight, const QColor& fillColorOnDark);

    UBItemStyle intersected(const UBItemStyle& other);

    friend bool operator==(const UBItemStyle& lhs, const UBItemStyle& rhs);

private:
    class ColorSet
    {
    public:
        ColorSet() = default;
        ColorSet(const QColor& lineColorOnLight, const QColor& lineColorOnDark);

        QColor color(bool isDark) const;
        void setColor(const QColor& colorOnLight, const QColor& colorOnDark);

        bool operator==(const ColorSet& other) const;

    private:
        QColor mColorOnLight{Qt::transparent};
        QColor mColorOnDark{Qt::transparent};
    };

private:
    ColorSet mLineColor{{}, {}};
    double mLineWidth{0};
    Qt::PenStyle mLineStyle{Qt::NoPen};
    ColorSet mFillColor{{}, {}};
};
