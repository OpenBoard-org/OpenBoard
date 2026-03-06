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


#include "UBStylePalette.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBStyledItem.h"
#include "gui/UBBackgroundManager.h"
#include "gui/UBMainWindow.h"
#include "gui/UBToolbarButtonGroup.h"


UBStylePalette::UBStylePalette(QToolBar* toolBar, UBToolbarButtonGroup* lineColorChoice,
                               UBToolbarButtonGroup* lineWidthChoice, QWidget* parent)
    : UBToolbarExtensionPalette(toolBar, parent)
    , mLineColorChoice{lineColorChoice}
    , mLineWidthChoice{lineWidthChoice}
    , mLineColorActions{lineColorChoice->buttonActions()}
    , mLineWidthActions{lineWidthChoice->buttonActions()}
{
    init();

    connect(UBApplication::boardController, &UBBoardController::backgroundChanged, this,
            &UBStylePalette::updateColorPalette);
    connect(UBApplication::boardController, &UBBoardController::activeSceneChanged, this,
            &UBStylePalette::updateColorPalette);
    connect(UBDrawingController::drawingController(), &UBDrawingController::stylusToolChanged, this,
            &UBStylePalette::switchMode);

    connect(UBSettings::settings(), &UBSettings::colorContextChanged, this, &UBStylePalette::colorContextChanged);

    connect(this, &UBStylePalette::styleChanged, this, &UBStylePalette::updatePreview);
    connect(this, &UBStylePalette::styleChanged, this,
            [](UBItemStyle style)
            {
                if (UBApplication::boardController->activeScene())
                {
                    UBApplication::boardController->activeScene()->applyStyle(style);
                }
            });

    auto settings = UBSettings::settings();

    const auto lineColorOnLight =
        QColor::fromString(settings->value("Board/StyleLineColorOnLight", "black").toString());
    const auto lineColorOnDark = QColor::fromString(settings->value("Board/StyleLineColorOnDark", "white").toString());
    const auto lineWidth = settings->value("Board/StyleLineWidth", 3.).toDouble();
    const auto lineStyle =
        settings->value("Board/StyleLineStyle", static_cast<int>(Qt::SolidLine)).value<Qt::PenStyle>();
    const auto fillColorOnLight =
        QColor::fromString(settings->value("Board/StyleFillColorOnLight", "transparent").toString());
    const auto fillColorOnDark =
        QColor::fromString(settings->value("Board/StyleFillColorOnDark", "transparent").toString());

    mStyle = UBItemStyle{lineColorOnLight, lineColorOnDark, lineWidth, lineStyle, fillColorOnLight, fillColorOnDark};
    updateChoice(mStyle);
}

UBStylePalette::~UBStylePalette()
{
    auto settings = UBSettings::settings();

    const auto colorToString = [](const QColor& color)
    {
        if (color.isValid())
        {
            return color.name(QColor::HexArgb);
        }
        else
        {
            return QString{"invalid"};
        }
    };

    settings->setValue("Board/StyleLineColorOnLight", colorToString(mStyle.lineColor(false)));
    settings->setValue("Board/StyleLineColorOnDark", colorToString(mStyle.lineColor(true)));
    settings->setValue("Board/StyleLineWidth", mStyle.lineWidth());
    settings->setValue("Board/StyleLineStyle", static_cast<int>(mStyle.lineStyle()));
    settings->setValue("Board/StyleFillColorOnLight", colorToString(mStyle.fillColor(false)));
    settings->setValue("Board/StyleFillColorOnDark", colorToString(mStyle.fillColor(true)));

    settings->save();
}

UBItemStyle UBStylePalette::selectedStyle()
{
    return mMode == UBStylusTool::Selector ? mCommonStyle : mStyle;
}

void UBStylePalette::updateSelection()
{
    if (mUpdateTriggered || mMode != UBStylusTool::Selector)
    {
        return;
    }

    mUpdateTriggered = true;

    QTimer::singleShot(
        0, this,
        [this]()
        {
            // compute common style of selected styled items
            const auto selectedItems = UBApplication::boardController->activeScene()->selectedStyledItems();
            bool selectionContainsShape{false};
            bool selectionOnlyContainsMarker{true};

            if (!selectedItems.isEmpty())
            {
                mCommonStyle = (*selectedItems.begin())->itemStyle();

                for (const auto item : selectedItems)
                {
                    mCommonStyle = mCommonStyle.intersected(item->itemStyle());
                    selectionContainsShape |= item->isShape();
                    selectionOnlyContainsMarker &= item->isMarker();
                }

                mLineColorChoice->colorPaletteChanged(selectionOnlyContainsMarker ? UBStylusTool::Marker
                                                                                  : UBStylusTool::Pen);
                updateChoice(mCommonStyle);

                if (selectionOnlyContainsMarker)
                {
                    mLineColorChoice->setLabel(tr("Marker Color"));
                }
                else
                {
                    mLineColorChoice->setLabel(tr("Line Color"));
                }

                mLineColorChoice->update();
            }

            setVisible(selectionContainsShape || mMode == UBStylusTool::Drawing);
            mLineWidthChoice->setEnabled(selectionContainsShape || mLineWidthChoice->currentIndex() >= 0);
            mUpdateTriggered = false;
        });
}

void UBStylePalette::switchMode(int tool)
{
    mMode = static_cast<UBStylusTool::Enum>(tool);

    switch (mMode)
    {
    case UBStylusTool::Pen:
        mLineColorChoice->setLabel(tr("Pen Color"));
        mLineColorChoice->update();
        hide();
        break;

    case UBStylusTool::Marker:
        mLineColorChoice->setLabel(tr("Marker Color"));
        mLineColorChoice->update();
        hide();
        break;

    case UBStylusTool::Drawing:
        mLineColorChoice->setLabel(tr("Shape Color"));
        mLineColorChoice->update();
        show();
        mLineColorChoice->colorPaletteChanged(UBStylusTool::Drawing);
        updateChoice(mStyle);
        break;

    case UBStylusTool::Selector:
        updateSelection();
        break;

    default:
        hide();
        break;
    }
}

void UBStylePalette::updateChoice(const UBItemStyle& style)
{
    // search for matching line color
    mLineColorChoice->setCurrentIndex(-1);

    for (const auto action : mLineColorActions)
    {
        if (action->isVisible() && action->property("color").isValid())
        {
            const auto color = action->property("color").value<QVariantList>();
            const auto match = color.at(0).value<QColor>() == style.lineColor(false) &&
                               color.at(1).value<QColor>() == style.lineColor(true);

            if (match)
            {
                mLineColorChoice->setCurrentIndex(mLineColorActions.indexOf(action));
                break;
            }
        }
    }

    // search for matching line width
    const auto width = style.lineWidth();
    const auto settings = UBSettings::settings();

    QList<bool> matchWidth;

    matchWidth << qFuzzyCompare(width, settings->boardPenFineWidth->get().toDouble())
               << qFuzzyCompare(width, settings->boardPenMediumWidth->get().toDouble())
               << qFuzzyCompare(width, settings->boardPenStrongWidth->get().toDouble());

    mLineWidthChoice->setCurrentIndex(matchWidth.indexOf(true));

    // search for matching line style
    mLineStyleActions.at(0)->setChecked(style.lineStyle() == Qt::SolidLine);
    mLineStyleActions.at(1)->setChecked(style.lineStyle() == Qt::DashLine);
    mLineStyleActions.at(2)->setChecked(style.lineStyle() == Qt::DotLine);

    // search for matching fill color
    for (const auto action : mSolidFillColorActions + mTransparentFillColorActions)
    {
        if (action->isVisible() && action->property("color").isValid())
        {
            const auto color = action->property("color").value<QVariantList>();
            // convert to strings to avoid mismatch caused by rounding errors
            const auto match =
                color.at(0).value<QColor>().name(QColor::HexArgb) == style.fillColor(false).name(QColor::HexArgb) &&
                color.at(1).value<QColor>().name(QColor::HexArgb) == style.fillColor(true).name(QColor::HexArgb);
            action->setChecked(match);
        }
    }

    updatePreview(style);
}

void UBStylePalette::init()
{
    mGridLayout = new QGridLayout{this};
    mGridLayout->setVerticalSpacing(0);

    // Setup line color choice actions
    for (const auto action : mLineColorActions)
    {
        if (action->objectName() != "actionColorPreferences")
        {
            connect(action, &QAction::triggered, this,
                    [this]()
                    {
                        const auto color = sender()->property("color").value<QVariantList>();

                        auto newStyle = selectedStyle();
                        newStyle.setLineColor(color.at(0).value<QColor>(), color.at(1).value<QColor>());
                        applyStyle(newStyle);
                    });
        }
    }

    // Setup line width choice actions
    connect(mLineWidthActions.at(0), &QAction::triggered, this,
            [this]()
            {
                const auto width = UBSettings::settings()->boardPenFineWidth->get().toDouble();
                auto newStyle = selectedStyle();
                newStyle.setLineWidth(width);
                applyStyle(newStyle);
            });

    connect(mLineWidthActions.at(1), &QAction::triggered, this,
            [this]()
            {
                const auto width = UBSettings::settings()->boardPenMediumWidth->get().toDouble();
                auto newStyle = selectedStyle();
                newStyle.setLineWidth(width);
                applyStyle(newStyle);
            });

    connect(mLineWidthActions.at(2), &QAction::triggered, this,
            [this]()
            {
                const auto width = UBSettings::settings()->boardPenStrongWidth->get().toDouble();
                auto newStyle = selectedStyle();
                newStyle.setLineWidth(width);
                applyStyle(newStyle);
            });

    // Setup line style choice widget
    for (int i = 0; i < 3; ++i)
    {
        auto action = new QAction{this};
        action->setCheckable(true);
        mLineStyleActions << action;
    }

    mLineStyleChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, mLineStyleActions, "");
    mLineStyleChoice->displayText(false);
    mLineStyleChoice->layout()->setContentsMargins({});

    setLineStyleIconAndConnect(mLineStyleChoice, 0, Qt::SolidLine);
    setLineStyleIconAndConnect(mLineStyleChoice, 1, Qt::DashLine);
    setLineStyleIconAndConnect(mLineStyleChoice, 2, Qt::DotLine);

    QLabel* lineStyleLabel = new QLabel{tr("Line style")};
    mGridLayout->addWidget(mLineStyleChoice, 0, 0, Qt::AlignLeft);
    mGridLayout->addWidget(lineStyleLabel, 1, 0, Qt::AlignHCenter);

    // Setup fill color choice widget
    for (int i = 0; i < UBSettings::maxColorPaletteSize; ++i)
    {
        auto action = new QAction{this};
        action->setCheckable(true);
        mSolidFillColorActions << action;

        action = new QAction{this};
        action->setCheckable(true);
        mTransparentFillColorActions << action;
    }

    // append transparent background button to transparent fill colors
    auto action = new QAction;
    action->setCheckable(true);
    mTransparentFillColorActions << action;

    mSolidFillColorChoice = new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, mSolidFillColorActions,
                                                     "", UBSettings::settings()->colorPaletteSize);
    mSolidFillColorChoice->displayText(false);
    mSolidFillColorChoice->layout()->setContentsMargins({});

    mTransparentFillColorChoice =
        new UBToolbarButtonGroup(UBApplication::mainWindow->boardToolBar, mTransparentFillColorActions, "",
                                 UBSettings::settings()->colorPaletteSize + 1);
    mTransparentFillColorChoice->displayText(false);
    mTransparentFillColorChoice->layout()->setContentsMargins({});

    // use a common action group for solid and transparent fill colors
    auto fillColorActionGroup = mSolidFillColorActions.at(0)->actionGroup();

    for (const auto action : mSolidFillColorActions + mTransparentFillColorActions)
    {
        action->setActionGroup(fillColorActionGroup);
        connect(action, &QAction::triggered, this,
                [this]()
                {
                    auto color = sender()->property("color").value<QVariantList>();
                    auto newStyle = selectedStyle();
                    newStyle.setFillColor(color.at(0).value<QColor>(), color.at(1).value<QColor>());
                    applyStyle(newStyle);
                });
    }

    QLabel* fillColorLabel = new QLabel{tr("Fill color")};
    mGridLayout->addWidget(mSolidFillColorChoice, 2, 0, Qt::AlignLeft);
    mGridLayout->addWidget(mTransparentFillColorChoice, 3, 0, Qt::AlignLeft);
    mGridLayout->addWidget(fillColorLabel, 4, 0, Qt::AlignHCenter);

    updateButtonColors();

    // setup preview
    QWidget* preview = new QWidget{this};
    QHBoxLayout* hbox = new QHBoxLayout{preview};

    mCurrentPreviewLabel = new QLabel;
    hbox->addWidget(mCurrentPreviewLabel);

    mSave = new QPushButton{">"};
    mRecall = new QPushButton{"<"};

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(mSave);
    vbox->addWidget(mRecall);

    hbox->addLayout(vbox);

    mSavedPreviewLabel = new QLabel;
    hbox->addWidget(mSavedPreviewLabel);

    connect(mSave, &QPushButton::clicked, this, &UBStylePalette::saveStyle);
    connect(mRecall, &QPushButton::clicked, this, &UBStylePalette::recallStyle);

    QLabel* previewLabel = new QLabel{tr("Preview")};
    mGridLayout->addWidget(preview, 5, 0, Qt::AlignHCenter);
    mGridLayout->addWidget(previewLabel, 6, 0, Qt::AlignHCenter);
    mGridLayout->setRowMinimumHeight(5, hbox->sizeHint().height());

    preview->resize(hbox->sizeHint());

    resize(mGridLayout->sizeHint());
}

void UBStylePalette::setLineStyleIconAndConnect(UBToolbarButtonGroup* buttonGroup, int index, Qt::PenStyle style)
{
    QPixmap pixmap{54, 8};
    pixmap.fill(Qt::transparent);

    QPainter painter{&pixmap};
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPen pen{Qt::black, 3, style, Qt::RoundCap};
    painter.setPen(pen);
    painter.drawLine(4, 4, 50, 4);

    QIcon icon{pixmap};

    const auto buttonList = buttonGroup->findChildren<QToolButton*>(Qt::FindDirectChildrenOnly);

    if (buttonList.size() > index)
    {
        buttonList.at(index)->setStyleSheet("width: 64");
        buttonList.at(index)->defaultAction()->setIcon(icon);
        buttonList.at(index)->setIconSize(pixmap.size());
    }

    connect(buttonList.at(index)->defaultAction(), &QAction::triggered, this,
            [this, style]()
            {
                auto newStyle = selectedStyle();
                newStyle.setLineStyle(style);
                applyStyle(newStyle);
            });
}

void UBStylePalette::setTransparentIcon(UBToolbarButtonGroup* buttonGroup, int index)
{
    const QSize iconSize{24, 24};
    const QSize patternSize{8, 8};
    const uint patternColor{0xff808080};

    QImage pattern{patternSize, QImage::Format_ARGB32};
    pattern.fill(Qt::white);

    const auto blockSize = patternSize.width() / 2;

    for (int x = 0; x < blockSize; ++x)
    {
        for (int y = 0; y < blockSize; ++y)
        {
            pattern.setPixel(x, y, patternColor);
            pattern.setPixel(x + blockSize, y + blockSize, patternColor);
        }
    }

    QBrush brush;
    brush.setTextureImage(pattern);

    QPixmap pixmap{iconSize};
    pixmap.fill(Qt::white);

    QPainter painter{&pixmap};
    painter.fillRect(pixmap.rect(), brush);

    QIcon icon{pixmap};

    const auto buttonList = buttonGroup->findChildren<QToolButton*>(Qt::FindDirectChildrenOnly);

    if (buttonList.size() > index)
    {
        buttonList.at(index)->defaultAction()->setIcon(icon);
        buttonList.at(index)->setIconSize(pixmap.size());
    }
}

QPixmap UBStylePalette::createPreview(const UBItemStyle& style) const
{
    // get the current background
    const auto scene = UBApplication::boardController->activeScene();
    bool isDark{false};
    const UBBackgroundRuling* background{nullptr};

    if (scene)
    {
        isDark = scene->isDarkBackground();
        background = scene->background();
    }
    else
    {
        isDark = UBSettings::settings()->isDarkBackground();
        const auto backgroundUuid = UBSettings::settings()->pageBackgroundUuid();
        const auto bgManager = UBApplication::boardController->backgroundManager();
        background = bgManager->background(backgroundUuid);
    }

    QPixmap pixmap{70, 70};
    pixmap.fill(isDark ? Qt::black : Qt::white);

    QPainter painter{&pixmap};
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (background)
    {
        background->draw(&painter, pixmap.rect(), 20., pixmap.rect(), isDark);
    }

    const QPen borderPen{Qt::darkGray, 1};
    painter.setPen(borderPen);
    painter.drawRect(QRect{{0, 0}, pixmap.size()});

    const Qt::PenStyle penStyle =
        style.lineColor(isDark).isValid() && style.lineWidth() > 0 ? style.lineStyle() : Qt::NoPen;
    QPen pen{style.lineColor(isDark), style.lineWidth(), penStyle, Qt::RoundCap};

    if (penStyle != Qt::SolidLine && penStyle != Qt::NoPen)
    {
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern(UBShapeFactory::dashPattern(penStyle));
    }

    painter.setPen(pen);

    if (style.fillColor(isDark).isValid())
    {
        const QBrush brush{style.fillColor(isDark)};
        painter.setBrush(brush);
    }
    else
    {
        painter.setBrush(QBrush{});
    }

    painter.drawLine(5, 50, 30, 10);
    painter.drawEllipse(25, 25, 40, 40);

    return pixmap;
}

void UBStylePalette::updateColorPalette()
{
    updateButtonColors();
    updatePreview(mStyle);
    mSavedPreviewLabel->setPixmap(createPreview(mSavedStyle));
}

void UBStylePalette::updateButtonColors()
{
    const auto settings = UBSettings::settings();

    const auto isDarkBackground = settings->isDarkBackground();
    const auto solidColors = settings->penColors(isDarkBackground);
    const auto solidColorsOnLight = settings->penColors(false);
    const auto solidColorsOnDark = settings->penColors(true);

    mLineColorChoice->colorPaletteChanged(
        static_cast<UBStylusTool::Enum>(UBDrawingController::drawingController()->stylusTool()));
    mSolidFillColorChoice->colorPaletteChanged(UBStylusTool::Pen);
    mTransparentFillColorChoice->colorPaletteChanged(UBStylusTool::Marker);

    const auto transparentIndex = settings->colorPaletteSize;
    setTransparentIcon(mTransparentFillColorChoice, transparentIndex);
    mTransparentFillColorActions.at(transparentIndex)
        ->setProperty("color", QVariantList{QColor{Qt::transparent}, QColor{Qt::transparent}});
    mTransparentFillColorChoice->setSelectableCount(transparentIndex + 1); // + 1 for transparent fill
}

void UBStylePalette::updatePreview(const UBItemStyle& style)
{
    const auto previewPixmap = createPreview(style);
    mCurrentPreviewLabel->setPixmap(previewPixmap);
}

void UBStylePalette::colorContextChanged()
{
    const auto selectableColors = UBSettings::settings()->boardColorPaletteSize->get().toInt();
    mLineColorChoice->setSelectableCount(selectableColors);
    mSolidFillColorChoice->setSelectableCount(selectableColors);
    updateButtonColors();

    // defer resizing so that sizeHint is already updated
    QTimer::singleShot(0, [this]() { resize(mGridLayout->sizeHint()); });
}

void UBStylePalette::applyStyle(const UBItemStyle& style)
{
    if (!(style == selectedStyle()) && (mMode == UBStylusTool::Selector || mMode == UBStylusTool::Drawing))
    {
        if (mMode == UBStylusTool::Selector)
        {
            mCommonStyle = style;
        }
        else
        {
            mStyle = style;
        }

        emit styleChanged(style);
    }
}

void UBStylePalette::saveStyle()
{
    mSavedStyle = selectedStyle();
    mSavedPreviewLabel->setPixmap(createPreview(mSavedStyle));
}

void UBStylePalette::recallStyle()
{
    updateChoice(mSavedStyle);
    applyStyle(mSavedStyle);
}
