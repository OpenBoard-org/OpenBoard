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

#ifndef UBTHEMEMANAGER_H
#define UBTHEMEMANAGER_H

#include <QObject>
#include <QPalette>
#include <QColor>

/**
 * @brief Manages application theme (dark/light mode) detection and application
 * 
 * This class is responsible for:
 * - Detecting system theme preference (Windows, macOS, Linux)
 * - Applying appropriate color palettes
 * - Loading and managing theme stylesheets
 * - Notifying components of theme changes
 */
class UBThemeManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Get the singleton instance
     */
    static UBThemeManager* instance();
    
    /**
     * @brief Apply theme based on user preference setting
     * 
     * Respects UBSettings::appThemeMode: 0 = Auto, 1 = Light, 2 = Dark
     */
    void applyUserThemePreference();

    /**
     * @brief Apply a specific theme (dark or light)
     * @param isDark true for dark mode, false for light mode
     */
    void applyTheme(bool isDark);

    /**
     * @brief Check if dark mode is currently active
     * @return true if dark mode is enabled
     */
    bool isDarkMode() const { return mIsDarkMode; }

    /**
     * @brief Update all theme-dependent widgets after theme change
     * 
     * This method updates runtime widget instances including:
     * - Floating palettes
     * - Dock palettes  
     * - Desktop property palettes
     * - Startup hints palette
     */
    void updateWidgets();

public slots:
    /**
     * @brief Handle system color scheme changes (Qt 6.5+)
     * @param colorScheme The new color scheme from the OS
     */
    void onColorSchemeChanged(Qt::ColorScheme colorScheme);

signals:
    /**
     * @brief Emitted when the theme changes
     * @param isDark true if switched to dark mode
     */
    void themeChanged(bool isDark);

private:
    UBThemeManager();
    ~UBThemeManager();

    UBThemeManager(const UBThemeManager&) = delete;
    UBThemeManager& operator=(const UBThemeManager&) = delete;

    /**
     * @brief Setup the dark theme palette
     */
    void setupDarkPalette();

    /**
     * @brief Setup the light theme palette
     */
    void setupLightPalette();

    /**
     * @brief Load and concatenate all stylesheets
     */
    void loadStylesheets();

    static UBThemeManager* sInstance;
    bool mIsDarkMode;
};

/**
 * @brief Theme color constants
 */
namespace UBTheme {
    namespace Dark {
        const QColor Window(53, 53, 53);
        const QColor WindowText(Qt::white);
        const QColor Base(42, 42, 42);
        const QColor AlternateBase(66, 66, 66);
        const QColor ToolTipBase(53, 53, 53);
        const QColor ToolTipText(Qt::white);
        const QColor Text(Qt::white);
        const QColor Button(53, 53, 53);
        const QColor ButtonText(Qt::white);
        const QColor BrightText(Qt::red);
        const QColor Link(42, 130, 218);
        const QColor Highlight(42, 130, 218);
        const QColor HighlightedText(Qt::black);
        const QColor Light(70, 70, 70);
        const QColor Midlight(60, 60, 60);
        const QColor Dark(35, 35, 35);
        const QColor Mid(50, 50, 50);
        const QColor Shadow(20, 20, 20);
        const QColor DisabledText(127, 127, 127);
        const QColor DisabledBase(49, 49, 49);
        const QColor DisabledButton(53, 53, 53);
        
        // OpenBoard-specific palette colors
        const QColor PaletteColor(70, 70, 70, 220);
        const QColor OpaquePaletteColor(50, 50, 50, 255);
    }
    
    namespace Light {
        const QColor Window(237, 242, 247);
        const QColor WindowText(31, 41, 55);
        const QColor Base(Qt::white);
        const QColor AlternateBase(247, 249, 252);
        const QColor ToolTipBase(Qt::white);
        const QColor ToolTipText(31, 41, 55);
        const QColor Text(31, 41, 55);
        const QColor Button(247, 249, 252);
        const QColor ButtonText(31, 41, 55);
        const QColor BrightText(Qt::red);
        const QColor Link(61, 143, 209);
        const QColor Highlight(61, 143, 209);
        const QColor HighlightedText(Qt::white);
        const QColor Light(Qt::white);
        const QColor Midlight(242, 246, 250);
        const QColor Dark(179, 191, 205);
        const QColor Mid(205, 214, 224);
        const QColor Shadow(143, 154, 168);
        const QColor DisabledText(148, 163, 184);
        const QColor DisabledBase(242, 242, 242);
        const QColor DisabledButton(239, 242, 247);
        
        // OpenBoard-specific palette colors
        const QColor PaletteColor(236, 241, 247, 235);
        const QColor OpaquePaletteColor(236, 241, 247, 255);
    }
}

#endif // UBTHEMEMANAGER_H
