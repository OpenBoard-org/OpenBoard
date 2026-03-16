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

#include "UBColorPreferencesDialog.h"

#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QAction>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QStyle>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QtGlobal>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBMainWindow.h"


UBColorPreferencesDialog::UBColorPreferencesDialog(QWidget* parent,
                                                   int paletteSize,
                                                   const QList<QColor>& penLightColors,
                                                   const QList<QColor>& penDarkColors,
                                                   const QList<QColor>& markerLightColors,
                                                   const QList<QColor>& markerDarkColors,
                                                   int markerOpacityPercent)
    : QDialog(parent)
    , mPenLightColors(penLightColors)
    , mPenDarkColors(penDarkColors)
    , mMarkerLightColors(opaqueColors(markerLightColors))
    , mMarkerDarkColors(opaqueColors(markerDarkColors))
    , mDefaultPenLightColors(UBSettings::settings()->defaultPenColors(false))
    , mDefaultPenDarkColors(UBSettings::settings()->defaultPenColors(true))
    , mDefaultMarkerLightColors(opaqueColors(UBSettings::settings()->defaultMarkerColors(false)))
    , mDefaultMarkerDarkColors(opaqueColors(UBSettings::settings()->defaultMarkerColors(true)))
    , mDefaultMarkerOpacityPercent(qBound(1, qRound(UBSettings::settings()->boardMarkerAlpha->defaultValue().toDouble() * 100.0), 100))
    , mMinPaletteSize(UBSettings::minColorPaletteSize)
    , mMaxPaletteSize(UBSettings::maxColorPaletteSize)
{
    setModal(true);
    setWindowTitle(tr("Color Preferences"));
    setMinimumWidth(520);
    setObjectName(QStringLiteral("colorPreferencesDialog"));

    const int boundedPaletteSize = qBound(mMinPaletteSize, paletteSize, mMaxPaletteSize);

    ensureSize(mPenLightColors, mDefaultPenLightColors);
    ensureSize(mPenDarkColors, mDefaultPenDarkColors);
    ensureSize(mMarkerLightColors, mDefaultMarkerLightColors);
    ensureSize(mMarkerDarkColors, mDefaultMarkerDarkColors);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 18, 20, 18);
    mainLayout->setSpacing(16);

    const QString paletteSizeToolTip = tr("This setting applies to the pen and marker palettes.");

    QWidget* sizeWidget = new QWidget(this);
    sizeWidget->setObjectName(QStringLiteral("colorPreferencesHeader"));
    QHBoxLayout* sizeLayout = new QHBoxLayout(sizeWidget);
    sizeLayout->setContentsMargins(14, 12, 14, 12);
    sizeLayout->setSpacing(12);

    QLabel* sizeLabel = new QLabel(tr("Palette size"), sizeWidget);
    sizeLabel->setObjectName(QStringLiteral("colorPreferencesHeaderLabel"));
    sizeLabel->setToolTip(paletteSizeToolTip);
    mPaletteSizeSlider = new QSlider(Qt::Horizontal, sizeWidget);
    mPaletteSizeSlider->setObjectName(QStringLiteral("colorPreferencesPaletteSizeSlider"));
    mPaletteSizeSlider->setRange(mMinPaletteSize, mMaxPaletteSize);
    mPaletteSizeSlider->setSingleStep(1);
    mPaletteSizeSlider->setPageStep(1);
    mPaletteSizeSlider->setTickInterval(1);
    mPaletteSizeSlider->setTickPosition(QSlider::TicksBelow);
    mPaletteSizeSlider->setValue(boundedPaletteSize);
    mPaletteSizeSlider->setToolTip(paletteSizeToolTip);
    connect(mPaletteSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(paletteSizeChanged(int)));
    mPaletteSizeValue = new QLabel(QString::number(boundedPaletteSize), sizeWidget);
    mPaletteSizeValue->setObjectName(QStringLiteral("colorPreferencesPaletteSizeValue"));
    mPaletteSizeValue->setAlignment(Qt::AlignCenter);
    mPaletteSizeValue->setToolTip(paletteSizeToolTip);
    sizeLayout->addWidget(sizeLabel);
    sizeLayout->addWidget(mPaletteSizeSlider, 1);
    sizeLayout->addWidget(mPaletteSizeValue);
    mainLayout->addWidget(sizeWidget);

    mTabWidget = new QTabWidget(this);
    mTabWidget->setObjectName(QStringLiteral("colorPreferencesTabWidget"));
    mTabWidget->setIconSize(QSize(18, 18));

    QWidget* penTab = createToolTab(tr("Light background"),
                                    tr("Dark background"),
                                    mPenLightFrame,
                                    mPenDarkFrame,
                                    mPenLightShortcutLabels,
                                    mPenDarkShortcutLabels,
                                    mPenLightButtons,
                                    mPenDarkButtons,
                                    mPenLightColors,
                                    mPenDarkColors,
                                    QStringLiteral("penLight"),
                                    QStringLiteral("penDark"),
                                    mResetPenButton);
    penTab->setObjectName(QStringLiteral("colorPreferencesPenTab"));
    mResetPenButton->setText(tr("Reset pen"));
    mResetPenButton->setObjectName(QStringLiteral("colorPreferencesResetButton"));
    connect(mResetPenButton, SIGNAL(clicked()), this, SLOT(resetPenSettings()));
    mTabWidget->addTab(penTab, tr("Pen"));

    QWidget* markerTab = createToolTab(tr("Light background"),
                                       tr("Dark background"),
                                       mMarkerLightFrame,
                                       mMarkerDarkFrame,
                                       mMarkerLightShortcutLabels,
                                       mMarkerDarkShortcutLabels,
                                       mMarkerLightButtons,
                                       mMarkerDarkButtons,
                                       mMarkerLightColors,
                                       mMarkerDarkColors,
                                       QStringLiteral("markerLight"),
                                       QStringLiteral("markerDark"),
                                       mResetMarkerButton);
    markerTab->setObjectName(QStringLiteral("colorPreferencesMarkerTab"));
    mResetMarkerButton->setText(tr("Reset marker"));
    mResetMarkerButton->setObjectName(QStringLiteral("colorPreferencesResetButton"));
    connect(mResetMarkerButton, SIGNAL(clicked()), this, SLOT(resetMarkerSettings()));

    QVBoxLayout* markerLayout = qobject_cast<QVBoxLayout*>(markerTab->layout());
    if (markerLayout)
    {
        QHBoxLayout* opacityLayout = new QHBoxLayout();
        QLabel* opacityLabel = new QLabel(tr("Marker opacity"), markerTab);
        opacityLabel->setObjectName(QStringLiteral("colorPreferencesSectionTitle"));
        mMarkerOpacitySlider = new QSlider(Qt::Horizontal, markerTab);
        mMarkerOpacitySlider->setObjectName(QStringLiteral("colorPreferencesOpacitySlider"));
        mMarkerOpacitySlider->setRange(1, 100);
        mMarkerOpacitySlider->setValue(qBound(1, markerOpacityPercent, 100));
        connect(mMarkerOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(markerOpacityChanged(int)));
        mMarkerOpacityValue = new QLabel(markerTab);
        mMarkerOpacityValue->setObjectName(QStringLiteral("colorPreferencesOpacityValue"));
        mMarkerOpacityValue->setMinimumWidth(40);
        opacityLayout->addWidget(opacityLabel);
        opacityLayout->addWidget(mMarkerOpacitySlider);
        opacityLayout->addWidget(mMarkerOpacityValue);
        markerLayout->insertLayout(markerLayout->count() - 2, opacityLayout);
    }

    mTabWidget->addTab(markerTab, tr("Marker"));
    connect(mTabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateTabIcons(int)));
    updateTabIcons(mTabWidget->currentIndex());
    mainLayout->addWidget(mTabWidget);

    QWidget* hintWidget = new QWidget(this);
    hintWidget->setObjectName(QStringLiteral("colorPreferencesHint"));
    QHBoxLayout* hintLayout = new QHBoxLayout(hintWidget);
    hintLayout->setContentsMargins(14, 10, 14, 10);
    hintLayout->setSpacing(10);

    QLabel* hintIcon = new QLabel(hintWidget);
    hintIcon->setObjectName(QStringLiteral("colorPreferencesHintIcon"));
    hintIcon->setPixmap(QIcon(QStringLiteral(":/images/toolbar/tip.png")).pixmap(20, 20));
    hintIcon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel* hintLabel = new QLabel(
        tr("When you switch between light and dark background in OpenBoard, each color is replaced by its counterpart so your annotations stay readable."),
        hintWidget);
    hintLabel->setObjectName(QStringLiteral("colorPreferencesHintText"));
    hintLabel->setWordWrap(true);

    hintLayout->addWidget(hintIcon, 0, Qt::AlignTop);
    hintLayout->addWidget(hintLabel, 1);
    mainLayout->addWidget(hintWidget);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setObjectName(QStringLiteral("colorPreferencesButtonBox"));
    QPushButton* defaultSettingsButton = buttons->addButton(tr("Default settings"), QDialogButtonBox::ResetRole);
    defaultSettingsButton->setObjectName(QStringLiteral("colorPreferencesResetButton"));
    connect(defaultSettingsButton, SIGNAL(clicked()), this, SLOT(resetDefaultSettings()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttons);

    updateBackgroundFrames();
    paletteSizeChanged(boundedPaletteSize);
    markerOpacityChanged(mMarkerOpacitySlider->value());
}


int UBColorPreferencesDialog::paletteSize() const
{
    return mPaletteSizeSlider->value();
}


QList<QColor> UBColorPreferencesDialog::penLightColors() const
{
    return mPenLightColors;
}


QList<QColor> UBColorPreferencesDialog::penDarkColors() const
{
    return mPenDarkColors;
}


QList<QColor> UBColorPreferencesDialog::markerLightColors() const
{
    return mMarkerLightColors;
}


QList<QColor> UBColorPreferencesDialog::markerDarkColors() const
{
    return mMarkerDarkColors;
}


int UBColorPreferencesDialog::markerOpacity() const
{
    return mMarkerOpacitySlider->value();
}


void UBColorPreferencesDialog::paletteSizeChanged(int value)
{
    Q_UNUSED(value);

    ensureSize(mPenLightColors, mDefaultPenLightColors);
    ensureSize(mPenDarkColors, mDefaultPenDarkColors);
    ensureSize(mMarkerLightColors, mDefaultMarkerLightColors);
    ensureSize(mMarkerDarkColors, mDefaultMarkerDarkColors);

    const int visibleCount = mPaletteSizeSlider->value();
    if (mPaletteSizeValue)
        mPaletteSizeValue->setText(QString::number(visibleCount));

    auto setRowWidth = [visibleCount](QWidget* frame, const QList<QToolButton*>& buttons)
    {
        if (!frame)
            return;

        const int buttonWidth = buttons.isEmpty() ? 0 : buttons.first()->width();
        int rowWidth = visibleCount * buttonWidth;

        if (QGridLayout* layout = qobject_cast<QGridLayout*>(frame->layout()))
        {
            const QMargins margins = layout->contentsMargins();
            rowWidth += margins.left() + margins.right();
            rowWidth += qMax(0, visibleCount - 1) * layout->horizontalSpacing();
        }

        frame->setMinimumWidth(rowWidth);
        frame->setMaximumWidth(rowWidth);
    };

    refreshPenSwatches();
    refreshMarkerSwatches();

    setRowWidth(mPenLightFrame, mPenLightButtons);
    setRowWidth(mPenDarkFrame, mPenDarkButtons);
    setRowWidth(mMarkerLightFrame, mMarkerLightButtons);
    setRowWidth(mMarkerDarkFrame, mMarkerDarkButtons);
}


void UBColorPreferencesDialog::penSwatchClicked()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (!button)
        return;

    const QString role = button->property("role").toString();
    const int index = button->property("index").toInt();
    QList<QColor>* colors = role == QLatin1String("penDark") ? &mPenDarkColors : &mPenLightColors;

    if (!colors || index < 0 || index >= colors->size())
        return;

    const QColor chosen = QColorDialog::getColor(colors->at(index), this, tr("Select Pen Color"));
    if (!chosen.isValid())
        return;

    (*colors)[index] = chosen;
    refreshPenSwatches();
}


void UBColorPreferencesDialog::markerSwatchClicked()
{
    QToolButton* button = qobject_cast<QToolButton*>(sender());
    if (!button)
        return;

    const QString role = button->property("role").toString();
    const int index = button->property("index").toInt();
    QList<QColor>* colors = role == QLatin1String("markerDark") ? &mMarkerDarkColors : &mMarkerLightColors;

    if (!colors || index < 0 || index >= colors->size())
        return;

    QColor initialColor = colors->at(index);
    initialColor.setAlpha(255);

    const QColor chosen = QColorDialog::getColor(initialColor, this, tr("Select Marker Color"));
    if (!chosen.isValid())
        return;

    QColor opaqueChosen = chosen;
    opaqueChosen.setAlpha(255);
    (*colors)[index] = opaqueChosen;
    refreshMarkerSwatches();
}


void UBColorPreferencesDialog::markerOpacityChanged(int value)
{
    mMarkerOpacityValue->setText(QString::number(value) + QStringLiteral("%"));
    refreshMarkerSwatches();
}


void UBColorPreferencesDialog::resetPenSettings()
{
    mPenLightColors = mDefaultPenLightColors;
    mPenDarkColors = mDefaultPenDarkColors;

    ensureSize(mPenLightColors, mDefaultPenLightColors);
    ensureSize(mPenDarkColors, mDefaultPenDarkColors);
    refreshPenSwatches();
}


void UBColorPreferencesDialog::resetMarkerSettings()
{
    mMarkerLightColors = mDefaultMarkerLightColors;
    mMarkerDarkColors = mDefaultMarkerDarkColors;

    ensureSize(mMarkerLightColors, mDefaultMarkerLightColors);
    ensureSize(mMarkerDarkColors, mDefaultMarkerDarkColors);
    mMarkerOpacitySlider->setValue(mDefaultMarkerOpacityPercent);
    refreshMarkerSwatches();
}


void UBColorPreferencesDialog::resetDefaultSettings()
{
    mPaletteSizeSlider->setValue(UBSettings::defaultColorPaletteSize);
    resetPenSettings();
    resetMarkerSettings();
}


QWidget* UBColorPreferencesDialog::createToolTab(const QString& lightTitle,
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
                                                 QPushButton*& resetButton)
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* tabLayout = new QVBoxLayout(tab);
    tabLayout->setContentsMargins(8, 14, 8, 6);
    tabLayout->setSpacing(14);

    buildColorSection(tabLayout, lightTitle, lightFrame, lightShortcutLabels, lightButtons, lightColors, lightRole);
    buildColorSection(tabLayout, darkTitle, darkFrame, darkShortcutLabels, darkButtons, darkColors, darkRole);

    tabLayout->addStretch();

    QHBoxLayout* resetLayout = new QHBoxLayout();
    resetLayout->addStretch();
    resetButton = new QPushButton(tab);
    resetLayout->addWidget(resetButton);
    tabLayout->addLayout(resetLayout);

    return tab;
}


void UBColorPreferencesDialog::buildColorSection(QVBoxLayout* parentLayout,
                                                 const QString& title,
                                                 QWidget*& frame,
                                                 QList<QLabel*>& shortcutLabels,
                                                 QList<QToolButton*>& buttons,
                                                 const QList<QColor>& colors,
                                                 const QString& role)
{
    QWidget* parentWidget = parentLayout ? parentLayout->parentWidget() : this;
    QWidget* sectionWidget = new QWidget(parentWidget);
    sectionWidget->setObjectName(QStringLiteral("colorPreferencesSection"));
    QVBoxLayout* sectionLayout = new QVBoxLayout(sectionWidget);
    sectionLayout->setContentsMargins(14, 12, 14, 12);
    sectionLayout->setSpacing(8);

    QLabel* titleLabel = new QLabel(title, sectionWidget);
    titleLabel->setObjectName(QStringLiteral("colorPreferencesSectionTitle"));
    sectionLayout->addWidget(titleLabel);

    frame = new QFrame(sectionWidget);
    frame->setObjectName(QStringLiteral("colorPreviewFrame"));
    frame->setProperty("previewMode", role.endsWith(QLatin1String("Dark")) ? QStringLiteral("dark") : QStringLiteral("light"));
    frame->setAutoFillBackground(true);
    frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    QGridLayout* layout = new QGridLayout(frame);
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->setContentsMargins(10, 8, 10, 8);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(4);
    buildSwatches(layout, shortcutLabels, buttons, colors, role);
    sectionLayout->addWidget(frame, 0, Qt::AlignHCenter);
    parentLayout->addWidget(sectionWidget);
}


void UBColorPreferencesDialog::buildSwatches(QGridLayout* layout,
                                             QList<QLabel*>& shortcutLabels,
                                             QList<QToolButton*>& buttons,
                                             const QList<QColor>& colors,
                                             const QString& role)
{
    QWidget* parent = layout ? layout->parentWidget() : this;

    for (int i = 0; i < mMaxPaletteSize; ++i)
    {
        QLabel* shortcut = new QLabel(colorIndexLabel(i), parent);
        shortcut->setObjectName(QStringLiteral("colorShortcutLabel"));
        shortcut->setAlignment(Qt::AlignCenter);
        shortcut->setFixedSize(32, 14);
        shortcut->setToolTip(colorToolTip(i));
        shortcutLabels.append(shortcut);

        QToolButton* button = new QToolButton(parent);
        button->setAutoRaise(false);
        button->setIconSize(QSize(18, 18));
        button->setFixedSize(34, 30);
        button->setProperty("index", i);
        button->setProperty("role", role);
        button->setToolTip(colorToolTip(i));
        buttons.append(button);

        if (layout)
        {
            layout->addWidget(shortcut, 0, i);
            layout->addWidget(button, 1, i);
        }
    }

    const bool isMarkerRow = role.startsWith(QLatin1String("marker"));

    for (QToolButton* button : buttons)
    {
        if (isMarkerRow)
            connect(button, SIGNAL(clicked()), this, SLOT(markerSwatchClicked()));
        else
            connect(button, SIGNAL(clicked()), this, SLOT(penSwatchClicked()));
    }

    refreshSwatches(shortcutLabels, buttons, colors, mPaletteSizeSlider->value());
}


void UBColorPreferencesDialog::refreshSwatches(QList<QLabel*>& shortcutLabels,
                                               QList<QToolButton*>& buttons,
                                               const QList<QColor>& colors,
                                               int visibleCount)
{
    for (int i = 0; i < buttons.size(); ++i)
    {
        QLabel* shortcut = i < shortcutLabels.size() ? shortcutLabels.at(i) : nullptr;
        QToolButton* button = buttons.at(i);
        const bool visible = i < visibleCount;

        if (shortcut)
        {
            shortcut->setVisible(visible);
            shortcut->setText(colorIndexLabel(i));
            shortcut->setToolTip(colorToolTip(i));
        }

        button->setVisible(visible);
        button->setEnabled(visible);
        button->setToolTip(colorToolTip(i));

        if (visible)
        {
            QString objectName;
            if (i == 0)
                objectName = QStringLiteral("ubButtonGroupLeft");
            else if (i == visibleCount - 1)
                objectName = QStringLiteral("ubButtonGroupRight");
            else
                objectName = QStringLiteral("ubButtonGroupCenter");

            if (button->objectName() != objectName)
            {
                button->setObjectName(objectName);
                button->style()->unpolish(button);
                button->style()->polish(button);
            }
        }

        if (i < colors.size())
        {
            button->setIcon(swatchIcon(colors.at(i)));
        }
    }
}


void UBColorPreferencesDialog::refreshPenSwatches()
{
    const int visibleCount = mPaletteSizeSlider->value();
    refreshSwatches(mPenLightShortcutLabels, mPenLightButtons, mPenLightColors, visibleCount);
    refreshSwatches(mPenDarkShortcutLabels, mPenDarkButtons, mPenDarkColors, visibleCount);
}


void UBColorPreferencesDialog::refreshMarkerSwatches()
{
    const int visibleCount = mPaletteSizeSlider->value();
    const QList<QColor> markerLightWithOpacity = markerColorsWithOpacity(mMarkerLightColors);
    const QList<QColor> markerDarkWithOpacity = markerColorsWithOpacity(mMarkerDarkColors);
    refreshSwatches(mMarkerLightShortcutLabels, mMarkerLightButtons, markerLightWithOpacity, visibleCount);
    refreshSwatches(mMarkerDarkShortcutLabels, mMarkerDarkButtons, markerDarkWithOpacity, visibleCount);
}


void UBColorPreferencesDialog::ensureSize(QList<QColor>& colors, const QList<QColor>& defaults)
{
    if (defaults.isEmpty())
        return;

    for (int i = colors.size(); i < mMaxPaletteSize; ++i)
    {
        colors.append(defaults.value(i % defaults.size(), QColor(Qt::black)));
    }

    if (colors.size() > mMaxPaletteSize)
    {
        colors = colors.mid(0, mMaxPaletteSize);
    }
}


void UBColorPreferencesDialog::updateBackgroundFrames()
{
    auto applyBackground = [](QWidget* widget, const QList<QLabel*>& labels, const QColor& background, const QColor& foreground)
    {
        if (!widget)
            return;

        QPalette palette = widget->palette();
        palette.setColor(QPalette::Window, background);
        palette.setColor(QPalette::WindowText, foreground);
        widget->setPalette(palette);

        for (QLabel* label : labels)
        {
            if (!label)
                continue;

            QPalette labelPalette = label->palette();
            labelPalette.setColor(QPalette::WindowText, foreground);
            labelPalette.setColor(QPalette::Text, foreground);
            label->setPalette(labelPalette);
        }
    };

    applyBackground(mPenLightFrame, mPenLightShortcutLabels, QColor(Qt::white), QColor(Qt::black));
    applyBackground(mMarkerLightFrame, mMarkerLightShortcutLabels, QColor(Qt::white), QColor(Qt::black));
    applyBackground(mPenDarkFrame, mPenDarkShortcutLabels, QColor(Qt::black), QColor(Qt::white));
    applyBackground(mMarkerDarkFrame, mMarkerDarkShortcutLabels, QColor(Qt::black), QColor(Qt::white));
}


QList<QColor> UBColorPreferencesDialog::opaqueColors(const QList<QColor>& colors)
{
    QList<QColor> opaque;

    for (const auto& color : colors)
    {
        QColor opaqueColor = color;
        opaqueColor.setAlpha(255);
        opaque.append(opaqueColor);
    }

    return opaque;
}


QIcon UBColorPreferencesDialog::swatchIcon(const QColor& color)
{
    const QSize size(12, 12);
    QPixmap pixmap(size);
    pixmap.fill(color);
    return QIcon(pixmap);
}


QList<QColor> UBColorPreferencesDialog::markerColorsWithOpacity(const QList<QColor>& colors) const
{
    QList<QColor> adjusted;
    adjusted.reserve(colors.size());

    const qreal opacityFactor = mMarkerOpacitySlider ? (mMarkerOpacitySlider->value() / 100.0) : 1.0;

    for (const QColor& color : colors)
    {
        QColor c = color;
        c.setAlphaF(qBound<qreal>(0.0, c.alphaF() * opacityFactor, 1.0));
        adjusted.append(c);
    }

    return adjusted;
}


QString UBColorPreferencesDialog::colorIndexLabel(int index) const
{
    return QString::number(index + 1);
}


QAction* UBColorPreferencesDialog::colorActionForIndex(int index) const
{
    if (!UBApplication::mainWindow)
        return nullptr;

    switch (index)
    {
        case 0: return UBApplication::mainWindow->actionColor0;
        case 1: return UBApplication::mainWindow->actionColor1;
        case 2: return UBApplication::mainWindow->actionColor2;
        case 3: return UBApplication::mainWindow->actionColor3;
        case 4: return UBApplication::mainWindow->actionColor4;
        case 5: return UBApplication::mainWindow->actionColor5;
        case 6: return UBApplication::mainWindow->actionColor6;
        case 7: return UBApplication::mainWindow->actionColor7;
        case 8: return UBApplication::mainWindow->actionColor8;
        case 9: return UBApplication::mainWindow->actionColor9;
        default: return nullptr;
    }
}


QString UBColorPreferencesDialog::shortcutTextForIndex(int index) const
{
    QAction* action = colorActionForIndex(index);
    if (!action)
        return QString();

    QStringList shortcutTexts;
    for (const auto& shortcut : action->shortcuts())
    {
        const QString text = shortcut.toString(QKeySequence::NativeText);
        if (!text.isEmpty())
            shortcutTexts.append(text);
    }

    return shortcutTexts.join(QStringLiteral(", "));
}


QString UBColorPreferencesDialog::colorToolTip(int index) const
{
    const QString shortcutText = shortcutTextForIndex(index);
    if (shortcutText.isEmpty())
        return tr("Color %1").arg(index + 1);

    return tr("Shortcut: %1").arg(shortcutText);
}


void UBColorPreferencesDialog::updateTabIcons(int currentIndex)
{
    const int penIndex = 0;
    const int markerIndex = 1;

    if (!mTabWidget || mTabWidget->count() <= markerIndex)
        return;

    mTabWidget->setTabIcon(penIndex, QIcon(currentIndex == penIndex
        ? QStringLiteral(":/images/stylusPalette/penOn.svg")
        : QStringLiteral(":/images/stylusPalette/pen.svg")));

    mTabWidget->setTabIcon(markerIndex, QIcon(currentIndex == markerIndex
        ? QStringLiteral(":/images/stylusPalette/markerOn.svg")
        : QStringLiteral(":/images/stylusPalette/marker.svg")));
}
