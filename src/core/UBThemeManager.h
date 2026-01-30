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
        const QColor OpaquePaletteColor(50, 50, 50, 240);
    }
    
    namespace Light {
        const QColor Window(240, 240, 240);
        const QColor WindowText(Qt::black);
        const QColor Base(Qt::white);
        const QColor AlternateBase(245, 245, 245);
        const QColor ToolTipBase(255, 255, 220);
        const QColor ToolTipText(Qt::black);
        const QColor Text(Qt::black);
        const QColor Button(240, 240, 240);
        const QColor ButtonText(Qt::black);
        const QColor BrightText(Qt::red);
        const QColor Link(0, 0, 255);
        const QColor Highlight(0, 120, 215);
        const QColor HighlightedText(Qt::white);
        const QColor Light(245, 245, 245);
        const QColor Midlight(242, 242, 242);
        const QColor Dark(160, 160, 160);
        const QColor Mid(160, 160, 160);
        const QColor Shadow(105, 105, 105);
        const QColor DisabledText(120, 120, 120);
        const QColor DisabledBase(240, 240, 240);
        const QColor DisabledButton(240, 240, 240);
        
        // OpenBoard-specific palette colors
        const QColor PaletteColor(180, 180, 180, 220);
        const QColor OpaquePaletteColor(200, 200, 200, 240);
    }
}

#endif // UBTHEMEMANAGER_H
