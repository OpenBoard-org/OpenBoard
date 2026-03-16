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




#ifndef UBCOLORPREFERENCESDIALOG_H
#define UBCOLORPREFERENCESDIALOG_H

#include <QDialog>
#include <QList>
#include <QColor>
#include <QString>

class QWidget;
class QGridLayout;
class QLabel;
class QVBoxLayout;
class QSlider;
class QTabWidget;
class QToolButton;
class QPushButton;
class QAction;
class QIcon;

class UBColorPreferencesDialog : public QDialog
{
    Q_OBJECT

    public:
        UBColorPreferencesDialog(QWidget* parent,
                                 int paletteSize,
                                 const QList<QColor>& penLightColors,
                                 const QList<QColor>& penDarkColors,
                                 const QList<QColor>& markerLightColors,
                                 const QList<QColor>& markerDarkColors,
                                 int markerOpacityPercent);

        int paletteSize() const;
        QList<QColor> penLightColors() const;
        QList<QColor> penDarkColors() const;
        QList<QColor> markerLightColors() const;
        QList<QColor> markerDarkColors() const;
        int markerOpacity() const;

    private slots:
        void paletteSizeChanged(int value);
        void penSwatchClicked();
        void markerSwatchClicked();
        void markerOpacityChanged(int value);
        void resetPenSettings();
        void resetMarkerSettings();
        void resetDefaultSettings();
        void updateTabIcons(int currentIndex);

    private:
        QWidget* createToolTab(const QString& lightTitle,
                               const QString& darkTitle,
                               QWidget*& lightFrame,
                               QWidget*& darkFrame,
                               QList<QLabel*>& lightShortcutLabels,
                               QList<QLabel*>& darkShortcutLabels,
                               QList<QToolButton*>& lightButtons,
                               QList<QToolButton*>& darkButtons,
                               const QList<QColor>& lightColors,
                               const QList<QColor>& darkColors,
                               const QString& lightRole,
                               const QString& darkRole,
                               QPushButton*& resetButton);
        void buildColorSection(QVBoxLayout* parentLayout,
                               const QString& title,
                               QWidget*& frame,
                               QList<QLabel*>& shortcutLabels,
                               QList<QToolButton*>& buttons,
                               const QList<QColor>& colors,
                               const QString& role);
        void buildSwatches(QGridLayout* layout,
                           QList<QLabel*>& shortcutLabels,
                           QList<QToolButton*>& buttons,
                           const QList<QColor>& colors,
                           const QString& role);
        void refreshSwatches(QList<QLabel*>& shortcutLabels, QList<QToolButton*>& buttons, const QList<QColor>& colors, int visibleCount);
        void refreshPenSwatches();
        void refreshMarkerSwatches();
        void ensureSize(QList<QColor>& colors, const QList<QColor>& defaults);
        void updateBackgroundFrames();
        static QIcon swatchIcon(const QColor& color);
        static QList<QColor> opaqueColors(const QList<QColor>& colors);
        QList<QColor> markerColorsWithOpacity(const QList<QColor>& colors) const;
        QString colorIndexLabel(int index) const;
        QAction* colorActionForIndex(int index) const;
        QString shortcutTextForIndex(int index) const;
        QString colorToolTip(int index) const;

        QSlider* mPaletteSizeSlider{nullptr};
        QLabel* mPaletteSizeValue{nullptr};
        QTabWidget* mTabWidget{nullptr};
        QSlider* mMarkerOpacitySlider{nullptr};
        QLabel* mMarkerOpacityValue{nullptr};
        QWidget* mPenLightFrame{nullptr};
        QWidget* mPenDarkFrame{nullptr};
        QWidget* mMarkerLightFrame{nullptr};
        QWidget* mMarkerDarkFrame{nullptr};
        QPushButton* mResetPenButton{nullptr};
        QPushButton* mResetMarkerButton{nullptr};
        QList<QLabel*> mPenLightShortcutLabels;
        QList<QLabel*> mPenDarkShortcutLabels;
        QList<QLabel*> mMarkerLightShortcutLabels;
        QList<QLabel*> mMarkerDarkShortcutLabels;
        QList<QToolButton*> mPenLightButtons;
        QList<QToolButton*> mPenDarkButtons;
        QList<QToolButton*> mMarkerLightButtons;
        QList<QToolButton*> mMarkerDarkButtons;
        QList<QColor> mPenLightColors;
        QList<QColor> mPenDarkColors;
        QList<QColor> mMarkerLightColors;
        QList<QColor> mMarkerDarkColors;
        QList<QColor> mDefaultPenLightColors;
        QList<QColor> mDefaultPenDarkColors;
        QList<QColor> mDefaultMarkerLightColors;
        QList<QColor> mDefaultMarkerDarkColors;
        const int mDefaultMarkerOpacityPercent;
        const int mMinPaletteSize;
        const int mMaxPaletteSize;
};



#endif // UBCOLORPREFERENCESDIALOG_H
