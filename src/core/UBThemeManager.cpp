/*
 * Copyright (C) 2015-2025 Département de l'Instruction Publique (DIP-SEM)
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

#include "UBThemeManager.h"
#include "UBApplication.h"
#include "UBSettings.h"
#include "UBApplicationController.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "gui/UBMainWindow.h"
#include "gui/UBFloatingPalette.h"
#include "gui/UBStartupHintsPalette.h"
#include "desktop/UBDesktopPropertyPalette.h"
#include "desktop/UBDesktopAnnotationController.h"
#include "desktop/UBDesktopPalette.h"

#include <QApplication>
#include <QStyleHints>
#include <QDebug>
#include <QTimer>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

UBThemeManager* UBThemeManager::sInstance = nullptr;

UBThemeManager::UBThemeManager()
    : QObject(nullptr)
    , mIsDarkMode(false)
{
}

UBThemeManager::~UBThemeManager()
{
}

UBThemeManager* UBThemeManager::instance()
{
    if (!sInstance)
    {
        sInstance = new UBThemeManager();
    }
    return sInstance;
}

void UBThemeManager::applyUserThemePreference()
{
    int themeMode = UBSettings::settings()->appThemeMode->get().toInt();
    
    bool isDarkMode = false;
    
    if (themeMode == 0) // Auto - detect from system
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        // Qt 6.5+: Use the new colorScheme API
        Qt::ColorScheme colorScheme = QGuiApplication::styleHints()->colorScheme();
        isDarkMode = (colorScheme == Qt::ColorScheme::Dark);
        
        qDebug() << "Theme detection using Qt 6.5+ API:" << (isDarkMode ? "Dark" : "Light");
#else
        // Fallback for Qt 6.0-6.4: platform-specific detection
        #ifdef Q_OS_WIN
            // Windows: Check registry for dark mode
            QSettings windowsSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", QSettings::NativeFormat);
            isDarkMode = (windowsSettings.value("AppsUseLightTheme", 1).toInt() == 0);
            qDebug() << "Theme detection (Windows registry):" << (isDarkMode ? "Dark" : "Light");
        #elif defined(Q_OS_MACOS)
            // macOS: Check system appearance using palette lightness
            isDarkMode = (QApplication::palette().color(QPalette::Window).lightness() < 128);
            qDebug() << "Theme detection (macOS palette):" << (isDarkMode ? "Dark" : "Light");
        #elif defined(Q_OS_LINUX)
            // Linux: Check if current palette suggests dark theme
            isDarkMode = (QApplication::palette().color(QPalette::Window).lightness() < 128);
            qDebug() << "Theme detection (Linux palette):" << (isDarkMode ? "Dark" : "Light");
        #endif
#endif
    }
    else if (themeMode == 1) // Light
    {
        isDarkMode = false;
        qDebug() << "Theme: User preference - Light";
    }
    else if (themeMode == 2) // Dark
    {
        isDarkMode = true;
        qDebug() << "Theme: User preference - Dark";
    }

    applyTheme(isDarkMode);
}

void UBThemeManager::applyTheme(bool isDark)
{
    bool hasThemeChanged = (mIsDarkMode != isDark);
    mIsDarkMode = isDark;
    
    if (mIsDarkMode)
    {
        setupDarkPalette();
    }
    else
    {
        setupLightPalette();
    }

    loadStylesheets();

    updateWidgets();
    
    if (hasThemeChanged)
    {
        emit themeChanged(mIsDarkMode);
    }
}

void UBThemeManager::setupDarkPalette()
{
    QPalette darkPalette;
    
    // Base colors
    darkPalette.setColor(QPalette::Window, UBTheme::Dark::Window);
    darkPalette.setColor(QPalette::WindowText, UBTheme::Dark::WindowText);
    darkPalette.setColor(QPalette::Base, UBTheme::Dark::Base);
    darkPalette.setColor(QPalette::AlternateBase, UBTheme::Dark::AlternateBase);
    darkPalette.setColor(QPalette::ToolTipBase, UBTheme::Dark::ToolTipBase);
    darkPalette.setColor(QPalette::ToolTipText, UBTheme::Dark::ToolTipText);
    darkPalette.setColor(QPalette::Text, UBTheme::Dark::Text);
    darkPalette.setColor(QPalette::Button, UBTheme::Dark::Button);
    darkPalette.setColor(QPalette::ButtonText, UBTheme::Dark::ButtonText);
    darkPalette.setColor(QPalette::BrightText, UBTheme::Dark::BrightText);
    darkPalette.setColor(QPalette::Link, UBTheme::Dark::Link);
    darkPalette.setColor(QPalette::Highlight, UBTheme::Dark::Highlight);
    darkPalette.setColor(QPalette::HighlightedText, UBTheme::Dark::HighlightedText);
    darkPalette.setColor(QPalette::Light, UBTheme::Dark::Light);
    darkPalette.setColor(QPalette::Midlight, UBTheme::Dark::Midlight);
    darkPalette.setColor(QPalette::Dark, UBTheme::Dark::Dark);
    darkPalette.setColor(QPalette::Mid, UBTheme::Dark::Mid);
    darkPalette.setColor(QPalette::Shadow, UBTheme::Dark::Shadow);
    
    // Disabled state colors
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, UBTheme::Dark::DisabledText);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, UBTheme::Dark::DisabledText);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, UBTheme::Dark::DisabledText);
    darkPalette.setColor(QPalette::Disabled, QPalette::Base, UBTheme::Dark::DisabledBase);
    darkPalette.setColor(QPalette::Disabled, QPalette::Button, UBTheme::Dark::DisabledButton);
    
    QApplication::setPalette(darkPalette);
    
    // Update OpenBoard-specific palette colors
    UBSettings::paletteColor = UBTheme::Dark::PaletteColor;
    UBSettings::opaquePaletteColor = UBTheme::Dark::OpaquePaletteColor;
}

void UBThemeManager::setupLightPalette()
{
    QPalette lightPalette;
    
    // Base colors
    lightPalette.setColor(QPalette::Window, UBTheme::Light::Window);
    lightPalette.setColor(QPalette::WindowText, UBTheme::Light::WindowText);
    lightPalette.setColor(QPalette::Base, UBTheme::Light::Base);
    lightPalette.setColor(QPalette::AlternateBase, UBTheme::Light::AlternateBase);
    lightPalette.setColor(QPalette::ToolTipBase, UBTheme::Light::ToolTipBase);
    lightPalette.setColor(QPalette::ToolTipText, UBTheme::Light::ToolTipText);
    lightPalette.setColor(QPalette::Text, UBTheme::Light::Text);
    lightPalette.setColor(QPalette::Button, UBTheme::Light::Button);
    lightPalette.setColor(QPalette::ButtonText, UBTheme::Light::ButtonText);
    lightPalette.setColor(QPalette::BrightText, UBTheme::Light::BrightText);
    lightPalette.setColor(QPalette::Link, UBTheme::Light::Link);
    lightPalette.setColor(QPalette::Highlight, UBTheme::Light::Highlight);
    lightPalette.setColor(QPalette::HighlightedText, UBTheme::Light::HighlightedText);
    lightPalette.setColor(QPalette::Light, UBTheme::Light::Light);
    lightPalette.setColor(QPalette::Midlight, UBTheme::Light::Midlight);
    lightPalette.setColor(QPalette::Dark, UBTheme::Light::Dark);
    lightPalette.setColor(QPalette::Mid, UBTheme::Light::Mid);
    lightPalette.setColor(QPalette::Shadow, UBTheme::Light::Shadow);
    
    // Disabled state colors
    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, UBTheme::Light::DisabledText);
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, UBTheme::Light::DisabledText);
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, UBTheme::Light::DisabledText);
    lightPalette.setColor(QPalette::Disabled, QPalette::Base, UBTheme::Light::DisabledBase);
    lightPalette.setColor(QPalette::Disabled, QPalette::Button, UBTheme::Light::DisabledButton);
    
    QApplication::setPalette(lightPalette);
    
    // Update OpenBoard-specific palette colors
    UBSettings::paletteColor = UBTheme::Light::PaletteColor;
    UBSettings::opaquePaletteColor = UBTheme::Light::OpaquePaletteColor;
}

void UBThemeManager::loadStylesheets()
{
    QString fullStylesheet;
    
    // Layer 1: Theme-specific stylesheet (dark or light)
    if (mIsDarkMode)
    {
        QString darkThemeCss = UBFileSystemUtils::readTextFile(":/darkTheme.qss");
        if (darkThemeCss.isEmpty()) {
            // Fallback to file system if resource not found (allows user customization)
            darkThemeCss = UBFileSystemUtils::readTextFile(
                UBPlatformUtils::applicationResourcesDirectory() + "/darkTheme.qss");
            
            if (darkThemeCss.isEmpty()) {
                qWarning() << "Failed to load dark theme stylesheet from resources and filesystem";
            }
        }
        fullStylesheet = darkThemeCss;
    } else {
        QString lightThemeCss = UBFileSystemUtils::readTextFile(":/lightTheme.qss");
        if (lightThemeCss.isEmpty()) {
            // Fallback to file system if resource not found (allows user customization)
            lightThemeCss = UBFileSystemUtils::readTextFile(
                UBPlatformUtils::applicationResourcesDirectory() + "/lightTheme.qss");
            
            if (lightThemeCss.isEmpty()) {
                qWarning() << "Failed to load light theme stylesheet from resources and filesystem";
            }
        }
        fullStylesheet = lightThemeCss;
    }
    
    // Layer 2: Embedded style.qss (palette-aware styling for OpenBoard widgets)
    QString embeddedStyle = UBFileSystemUtils::readTextFile(":/style.qss");
    if (!embeddedStyle.isEmpty()) {
        if (!fullStylesheet.isEmpty()) {
            fullStylesheet += "\n";
        }
        fullStylesheet += embeddedStyle;
    }
    
    // Layer 3: Standard CSS (user customizable, uses palette() references)
    QString css = UBFileSystemUtils::readTextFile(
        UBPlatformUtils::applicationEtcDirectory() + "/" + qApp->applicationName() + ".css");
    if (!css.isEmpty()) {
        if (!fullStylesheet.isEmpty()) {
            fullStylesheet += "\n";
        }
        fullStylesheet += css;
    }
    
    // Apply the combined stylesheet once
    qApp->setStyleSheet(fullStylesheet);
}

void UBThemeManager::updateWidgets()
{
    // Update all widgets after a theme change
    if (!UBApplication::mainWindow)
    {
        qDebug() << "Main window not initialized, skipping widget updates";
        return;
    }
        
    // Update all floating palettes
    auto floatingPalettes = UBApplication::mainWindow->findChildren<UBFloatingPalette*>();
    for (auto floatingPalette: std::as_const(floatingPalettes))
    {
        floatingPalette->setBackgroundBrush(QBrush(UBSettings::paletteColor));
    }

    // Update left and right dock palettes
    if (UBApplication::boardController && UBApplication::boardController->paletteManager())
    {
        auto paletteManager = UBApplication::boardController->paletteManager();

        if (paletteManager->leftPalette())
        {
            paletteManager->leftPalette()->setBackgroundBrush(QBrush(UBSettings::paletteColor));
        }

        if (paletteManager->rightPalette())
        {
            paletteManager->rightPalette()->setBackgroundBrush(QBrush(UBSettings::paletteColor));
        }
    }

    // Update desktop left palette and property palettes (pen, marker and eraser sub palettes)
    if (UBApplication::applicationController &&
        UBApplication::applicationController->uninotesController())
    {
        auto desktopController = UBApplication::applicationController->uninotesController();

        if (desktopController->desktopPalette())
        {
            desktopController->desktopPalette()->setBackgroundBrush(QBrush(UBSettings::opaquePaletteColor));
        }

        auto desktopPropertyPalettes = desktopController->findChildren<UBDesktopPropertyPalette*>();

        for (auto desktopPropertyPalette : std::as_const(desktopPropertyPalettes))
        {
            desktopPropertyPalette->setBackgroundBrush(QBrush(UBSettings::opaquePaletteColor));
        }
    }

    // Update startup hints palette background
    // (HTML content updates via themeChanged signal → JavaScript listener)
    if (UBApplication::boardController &&
        UBApplication::boardController->paletteManager() &&
        UBApplication::boardController->paletteManager()->tipsPalette())
    {
        auto startupHint = UBApplication::boardController->paletteManager()->tipsPalette();

        startupHint->setBackgroundBrush(QBrush(UBSettings::paletteColor));
        startupHint->setPalette(QApplication::palette());
        startupHint->update();
    }

    // Force all widgets to update their appearance
    // This ensures that all widgets pick up the new palette and stylesheet
    QList<QWidget*> allWidgets = UBApplication::mainWindow->findChildren<QWidget*>();
    for (QWidget* widget : allWidgets)
    {
        widget->update();
    }
    
    // Also update the main window itself
    UBApplication::mainWindow->update();

    qDebug() << "Widget updates completed";
}

void UBThemeManager::onColorSchemeChanged(Qt::ColorScheme colorScheme)
{
    qDebug() << "System color scheme changed to:" << (colorScheme == Qt::ColorScheme::Dark ? "Dark" : "Light");
    applyTheme(colorScheme == Qt::ColorScheme::Dark);
}
