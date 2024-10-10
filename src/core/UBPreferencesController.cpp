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




#include "UBPreferencesController.h"

#include "UBSettings.h"
#include "UBApplication.h"

#include "gui/UBCircleFrame.h"

#include "core/UBSetting.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

#include "frameworks/UBStringUtils.h"

#include "board/UBBoardController.h"
#include "document/UBDocumentController.h"
#include "domain/UBGraphicsScene.h"
#include "board/UBDrawingController.h"
#include "podcast/UBPodcastController.h"

#include "ui_preferences.h"

#include "core/memcheck.h"

qreal UBPreferencesController::sSliderRatio = 10.0;
qreal UBPreferencesController::sMinPenWidth = 0.5;
qreal UBPreferencesController::sMaxPenWidth = 50.0;


UBPreferencesDialog::UBPreferencesDialog(UBPreferencesController* prefController, QWidget* parent,Qt::WindowFlags f)
    :QDialog(parent,f)
    ,mPreferencesController(prefController)
{
}

UBPreferencesDialog::~UBPreferencesDialog()
{
}

void UBPreferencesDialog::closeEvent(QCloseEvent* e)
{
    e->accept();
}



UBPreferencesController::UBPreferencesController(QWidget *parent)
    : QObject(parent)
    , mPreferencesWindow(0)
    , mPreferencesUI(0)
    , mPenProperties(0)
    , mMarkerProperties(0)
    , mDarkBackgroundGridColorPicker(0)
    , mLightBackgroundGridColorPicker(0)
{
    mPreferencesWindow = new UBPreferencesDialog(this,parent, Qt::Dialog);
    mPreferencesUI = new Ui::preferencesDialog();  // deleted in destructor
    mPreferencesUI->setupUi(mPreferencesWindow);
    adjustScreensPreferences();

    connect(UBApplication::displayManager, &UBDisplayManager::availableScreenCountChanged, this, &UBPreferencesController::adjustScreensPreferences);
    wire();
}


UBPreferencesController::~UBPreferencesController()
{
    delete mPreferencesWindow;

    delete mPreferencesUI;

    delete mPenProperties;

    delete mMarkerProperties;
}

void UBPreferencesController::adjustScreensPreferences()
{
    bool enabled = UBApplication::displayManager->numScreens() > 1;
    mPreferencesUI->multiDisplayGroupBox->setEnabled(enabled);

    if (enabled)
    {
        mPreferencesUI->screenList->setPlaceholderText(tr("Use all available displays"));
    }
    else
    {
        mPreferencesUI->screenList->setPlaceholderText("");
    }

    auto availableScreens = UBApplication::displayManager->availableScreens();
    QStringList screenNames;
    static QRegularExpression specialChars("[^a-zA-Z0-9]+");

    for (QScreen* screen : availableScreens)
    {
        screenNames << screen->name().replace(specialChars, "-");
    }

    QString screenConfiguration = screenNames.join('_');

    QString path = UBSettings::settings()->appScreenList->path() + "-" + screenConfiguration;

    if (path != mScreenConfigurationPath)
    {
        mScreenConfigurationPath = path;
        QVariant value = UBSettings::settings()->value(path);
        UBSettings::settings()->appScreenList->set(value);
        mPreferencesUI->screenList->loadScreenList(value.toStringList());
    }
}

void UBPreferencesController::show()
{
    init();

    mPreferencesWindow->exec();
}

void UBPreferencesController::wire()
{
    UBSettings* settings = UBSettings::settings();

    // main tab
    mPreferencesUI->mainTabWidget->setCurrentWidget(mPreferencesUI->displayTab);
    mPreferencesUI->versionLabel->setText(tr("version: ") + UBApplication::applicationVersion());

    connect(mPreferencesUI->closeButton, SIGNAL(released()), this, SLOT(close()));
    connect(mPreferencesUI->defaultSettingsButton, SIGNAL(released()), this, SLOT(defaultSettings()));

    connect(mPreferencesUI->screenList, &UBScreenListLineEdit::screenListChanged, this, [this,settings](const QStringList screenList){
        settings->appScreenList->set(screenList);

        if (!mScreenConfigurationPath.isEmpty())
        {
            settings->setValue(mScreenConfigurationPath, screenList);
        }
    });

    // OSK preferences

    for(int i = 0; i < settings->supportedKeyboardSizes->size(); i++)
        mPreferencesUI->keyboardPaletteKeyButtonSize->addItem(settings->supportedKeyboardSizes->at(i));


    connect(mPreferencesUI->useSystemOSKCheckBox, SIGNAL(clicked(bool)), settings->useSystemOnScreenKeyboard, SLOT(setBool(bool)));
    connect(mPreferencesUI->useSystemOSKCheckBox, SIGNAL(clicked(bool)), this, SLOT(systemOSKCheckBoxToggled(bool)));

    // PDF preferences
    connect(mPreferencesUI->exportBackgroundGrid, SIGNAL(clicked(bool)), settings->exportBackgroundGrid, SLOT(setBool(bool)));
    connect(mPreferencesUI->exportBackgroundColor, SIGNAL(clicked(bool)), settings->exportBackgroundColor, SLOT(setBool(bool)));

    // Documents Mode preferences
    connect(mPreferencesUI->showDateColumnOnAlphabeticalSort, SIGNAL(clicked(bool)), settings->showDateColumnOnAlphabeticalSort, SLOT(setBool(bool)));
    connect(mPreferencesUI->showDateColumnOnAlphabeticalSort, SIGNAL(clicked(bool)), UBApplication::documentController, SLOT(refreshDateColumns()));
    connect(mPreferencesUI->emptyTrashForOlderDocuments, SIGNAL(clicked(bool)), settings->emptyTrashForOlderDocuments, SLOT(setBool(bool)));
    connect(mPreferencesUI->emptyTrashDaysValue, SIGNAL(valueChanged(int)), settings->emptyTrashDaysValue,  SLOT(setInt(int)));


    connect(mPreferencesUI->keyboardPaletteKeyButtonSize, qOverload<int>(&QComboBox::currentIndexChanged), settings->boardKeyboardPaletteKeyBtnSize, [=](int index) {
        settings->boardKeyboardPaletteKeyBtnSize->setString(mPreferencesUI->keyboardPaletteKeyButtonSize->itemText(index));
    });
    connect(mPreferencesUI->startModeComboBox, SIGNAL(currentIndexChanged(int)), settings->appStartMode, SLOT(setInt(int)));

    connect(mPreferencesUI->useExternalBrowserCheckBox, SIGNAL(clicked(bool)), settings->webUseExternalBrowser, SLOT(setBool(bool)));
    connect(mPreferencesUI->displayBrowserPageCheckBox, SIGNAL(clicked(bool)), settings->webShowPageImmediatelyOnMirroredScreen, SLOT(setBool(bool)));

    connect(mPreferencesUI->toolbarAtTopRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    connect(mPreferencesUI->toolbarAtBottomRadioButton, SIGNAL(clicked(bool)), this, SLOT(toolbarPositionChanged(bool)));
    connect(mPreferencesUI->horizontalChoice, SIGNAL(clicked(bool)), this, SLOT(toolbarOrientationHorizontal(bool)));
    connect(mPreferencesUI->verticalChoice, SIGNAL(clicked(bool)), this, SLOT(toolbarOrientationVertical(bool)));
    connect(mPreferencesUI->toolbarDisplayTextCheckBox, SIGNAL(clicked(bool)), settings->appToolBarDisplayText, SLOT(setBool(bool)));

    //grid tab
    //On light background
    QPalette lightBackgroundPalette = QApplication::palette();
    lightBackgroundPalette.setColor(QPalette::Window, Qt::white);

    mPreferencesUI->crossColorLightBackgroundFrame->setAutoFillBackground(true);
    mPreferencesUI->crossColorLightBackgroundFrame->setPalette(lightBackgroundPalette);

    QPalette darkBackgroundPalette = QApplication::palette();
    darkBackgroundPalette.setColor(QPalette::Window, Qt::black);
    darkBackgroundPalette.setColor(QPalette::ButtonText, Qt::white);
    darkBackgroundPalette.setColor(QPalette::WindowText, Qt::white);

    mPreferencesUI->crossColorDarkBackgroundFrame->setAutoFillBackground(true);
    mPreferencesUI->crossColorDarkBackgroundFrame->setPalette(darkBackgroundPalette);
    mPreferencesUI->crossColorDarkBackgroundLabel->setPalette(darkBackgroundPalette);

    QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();
    QColor selectedCrossColorLightBackground(settings->boardCrossColorLightBackground->get().toString());

    mLightBackgroundGridColorPicker = new UBColorPicker(mPreferencesUI->crossColorLightBackgroundFrame);
    mLightBackgroundGridColorPicker->setObjectName(QString::fromUtf8("crossColorLightBackgroundFrame"));
    mLightBackgroundGridColorPicker->setMinimumSize(QSize(32, 32));
    mLightBackgroundGridColorPicker->setFrameShape(QFrame::StyledPanel);
    mLightBackgroundGridColorPicker->setFrameShadow(QFrame::Raised);

    mPreferencesUI->crossColorLightBackgroundLayout->addWidget(mLightBackgroundGridColorPicker);
    mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);
    mLightBackgroundGridColorPicker->setSelectedColorIndex(gridLightBackgroundColors.indexOf(selectedCrossColorLightBackground));
    mPreferencesUI->lightBackgroundOpacitySlider->setValue(selectedCrossColorLightBackground.alpha()*100 / 255);

    QObject::connect(mLightBackgroundGridColorPicker, SIGNAL(colorSelected(const QColor&)), this, SLOT(setCrossColorOnLightBackground(const QColor&)));
    connect(mPreferencesUI->lightBackgroundOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(lightBackgroundCrossOpacityValueChanged(int)));

    //On dark background
    QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();
    QColor selectedCrossColorDarkBackground(settings->boardCrossColorDarkBackground->get().toString());

    mDarkBackgroundGridColorPicker = new UBColorPicker(mPreferencesUI->crossColorDarkBackgroundFrame);
    mDarkBackgroundGridColorPicker->setObjectName(QString::fromUtf8("crossColorDarkBackgroundFrame"));
    mDarkBackgroundGridColorPicker->setMinimumSize(QSize(32, 32));
    mDarkBackgroundGridColorPicker->setFrameShape(QFrame::StyledPanel);
    mDarkBackgroundGridColorPicker->setFrameShadow(QFrame::Raised);

    mPreferencesUI->crossColorDarkBackgroundLayout->addWidget(mDarkBackgroundGridColorPicker);
    mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);
    mDarkBackgroundGridColorPicker->setSelectedColorIndex(gridDarkBackgroundColors.indexOf(selectedCrossColorDarkBackground));
    mPreferencesUI->darkBackgroundOpacitySlider->setValue(selectedCrossColorDarkBackground.alpha()*100 / 255);

    QObject::connect(mDarkBackgroundGridColorPicker, SIGNAL(colorSelected(const QColor&)), this, SLOT(setCrossColorOnDarkBackground(const QColor&)));
    connect(mPreferencesUI->darkBackgroundOpacitySlider, SIGNAL(valueChanged(int)), this, SLOT(darkBackgroundCrossOpacityValueChanged(int)));

    // pen
    QList<QColor> penLightBackgroundColors = settings->boardPenLightBackgroundColors->colors();
    QList<QColor> penDarkBackgroundColors = settings->boardPenDarkBackgroundColors->colors();
    QList<QColor> penLightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();
    QList<QColor> penDarkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

    mPenProperties = new UBBrushPropertiesFrame(mPreferencesUI->penFrame,
                                                penLightBackgroundColors, penDarkBackgroundColors, penLightBackgroundSelectedColors,
                                                penDarkBackgroundSelectedColors, this);

    mPenProperties->opacityFrame->hide();

    connect(mPenProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mPenProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setPenPressureSensitive(bool)));
    connect(mPenProperties->circleCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setPenPreviewCircle(bool)));
    connect(mPenProperties->circleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(penPreviewFromSizeChanged(int)));

    // marker
    QList<QColor> markerLightBackgroundColors = settings->boardMarkerLightBackgroundColors->colors();
    QList<QColor> markerDarkBackgroundColors = settings->boardMarkerDarkBackgroundColors->colors();
    QList<QColor> markerLightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();
    QList<QColor> markerDarkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

    mMarkerProperties = new UBBrushPropertiesFrame(mPreferencesUI->markerFrame, markerLightBackgroundColors,
                                                   markerDarkBackgroundColors, markerLightBackgroundSelectedColors,
                                                   markerDarkBackgroundSelectedColors, this);

    mMarkerProperties->pressureSensitiveCheckBox->setText(tr("Marker is pressure sensitive"));

    mMarkerProperties->circleFrame->hide();

    connect(mMarkerProperties->fineSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->mediumSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->strongSlider, SIGNAL(valueChanged(int)), this, SLOT(widthSliderChanged(int)));
    connect(mMarkerProperties->pressureSensitiveCheckBox, SIGNAL(clicked(bool)), settings, SLOT(setMarkerPressureSensitive(bool)));
    connect(mMarkerProperties->opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacitySliderChanged(int)));

    // about tab
    connect(mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox, SIGNAL(clicked(bool)), settings->appEnableAutomaticSoftwareUpdates, SLOT(setBool(bool)));
}

void UBPreferencesController::init()
{
    UBSettings* settings = UBSettings::settings();

    // about tab
    mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(settings->appEnableAutomaticSoftwareUpdates->get().toBool());

    // display tab
    for(int i=0; i<mPreferencesUI->keyboardPaletteKeyButtonSize->count(); i++)
        if (mPreferencesUI->keyboardPaletteKeyButtonSize->itemText(i) == settings->boardKeyboardPaletteKeyBtnSize->get().toString()) {
            mPreferencesUI->keyboardPaletteKeyButtonSize->setCurrentIndex(i);
            break;
        }

    mPreferencesUI->useSystemOSKCheckBox->setChecked(settings->useSystemOnScreenKeyboard->get().toBool());
    this->systemOSKCheckBoxToggled(mPreferencesUI->useSystemOSKCheckBox->isChecked());

    mPreferencesUI->exportBackgroundGrid->setChecked(settings->exportBackgroundGrid->get().toBool());
    mPreferencesUI->exportBackgroundColor->setChecked(settings->exportBackgroundColor->get().toBool());

    mPreferencesUI->showDateColumnOnAlphabeticalSort->setChecked(settings->showDateColumnOnAlphabeticalSort->get().toBool());
    mPreferencesUI->emptyTrashForOlderDocuments->setChecked(settings->emptyTrashForOlderDocuments->get().toBool());
    mPreferencesUI->emptyTrashDaysValue->setValue(settings->emptyTrashDaysValue->get().toInt());

    mPreferencesUI->startModeComboBox->setCurrentIndex(settings->appStartMode->get().toInt());

    mPreferencesUI->useExternalBrowserCheckBox->setChecked(settings->webUseExternalBrowser->get().toBool());
    mPreferencesUI->displayBrowserPageCheckBox->setChecked(settings->webShowPageImmediatelyOnMirroredScreen->get().toBool());
    mPreferencesUI->screenList->loadScreenList(settings->appScreenList->get().toStringList());
    mPreferencesUI->webHomePage->setText(settings->webHomePage->get().toString());

    mPreferencesUI->proxyUsername->setText(settings->proxyUsername());
    mPreferencesUI->proxyPassword->setText(settings->proxyPassword());

    mPreferencesUI->toolbarAtTopRadioButton->setChecked(settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!settings->appToolBarPositionedAtTop->get().toBool());
    mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(settings->appToolBarDisplayText->get().toBool());
    mPreferencesUI->verticalChoice->setChecked(settings->appToolBarOrientationVertical->get().toBool());
    mPreferencesUI->horizontalChoice->setChecked(!settings->appToolBarOrientationVertical->get().toBool());

    // pen tab
    mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->get().toDouble() * sSliderRatio);
    mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->get().toDouble() * sSliderRatio);
    mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->get().toDouble() * sSliderRatio);
    mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->get().toBool());
    mPenProperties->circleCheckBox->setChecked(settings->showPenPreviewCircle->get().toBool());
    mPenProperties->circleSpinBox->setValue(settings->penPreviewFromSize->get().toInt());

    // marker tab
    mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->get().toDouble() * sSliderRatio);
    mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->get().toBool());

    mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->get().toDouble() * 100);

}

void UBPreferencesController::close()
{
    //web
    QString homePage = mPreferencesUI->webHomePage->text();

    UBSettings::settings()->webHomePage->set(homePage);
    UBSettings::settings()->setProxyUsername(mPreferencesUI->proxyUsername->text());
    UBSettings::settings()->setProxyPassword(mPreferencesUI->proxyPassword->text());

    mPreferencesWindow->accept();
}


void UBPreferencesController::defaultSettings()
{
    UBSettings* settings = UBSettings::settings();

    if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->displayTab)
    {
        mPreferencesUI->screenList->setDefault();

        bool defaultValue = settings->appToolBarPositionedAtTop->reset().toBool();
        mPreferencesUI->toolbarAtTopRadioButton->setChecked(defaultValue);
        mPreferencesUI->toolbarAtBottomRadioButton->setChecked(!defaultValue);

        defaultValue = settings->appToolBarDisplayText->reset().toBool();
        mPreferencesUI->toolbarDisplayTextCheckBox->setChecked(defaultValue);
        mPreferencesUI->verticalChoice->setChecked(settings->appToolBarOrientationVertical->reset().toBool());
        mPreferencesUI->horizontalChoice->setChecked(!settings->appToolBarOrientationVertical->reset().toBool());
        mPreferencesUI->startModeComboBox->setCurrentIndex(0);

        mPreferencesUI->useSystemOSKCheckBox->setChecked(settings->useSystemOnScreenKeyboard->reset().toBool());

        mPreferencesUI->showDateColumnOnAlphabeticalSort->setChecked(settings->showDateColumnOnAlphabeticalSort->reset().toBool());
        UBApplication::documentController->refreshDateColumns();

        mPreferencesUI->exportBackgroundGrid->setChecked(settings->exportBackgroundGrid->reset().toBool());
        mPreferencesUI->exportBackgroundColor->setChecked(settings->exportBackgroundColor->reset().toBool());

        mPreferencesUI->emptyTrashForOlderDocuments->setChecked(settings->emptyTrashForOlderDocuments->reset().toBool());
        mPreferencesUI->emptyTrashDaysValue->setValue(settings->emptyTrashDaysValue->reset().toInt());

    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->penTab)
    {

        mPenProperties->fineSlider->setValue(settings->boardPenFineWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->mediumSlider->setValue(settings->boardPenMediumWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->strongSlider->setValue(settings->boardPenStrongWidth->reset().toDouble() * sSliderRatio);
        mPenProperties->pressureSensitiveCheckBox->setChecked(settings->boardPenPressureSensitive->reset().toBool());
        mPenProperties->circleCheckBox->setChecked(settings->showPenPreviewCircle->reset().toBool());
        mPenProperties->circleSpinBox->setValue(settings->penPreviewFromSize->reset().toInt());

        settings->boardPenLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardPenLightBackgroundSelectedColors->colors();

        settings->boardPenDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardPenDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mPenProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->penColors(false).at(i)));
            mPenProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->penColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->markerTab)
    {
        mMarkerProperties->fineSlider->setValue(settings->boardMarkerFineWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->mediumSlider->setValue(settings->boardMarkerMediumWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->strongSlider->setValue(settings->boardMarkerStrongWidth->reset().toDouble() * sSliderRatio);
        mMarkerProperties->pressureSensitiveCheckBox->setChecked(settings->boardMarkerPressureSensitive->reset().toBool());

        mMarkerProperties->opacitySlider->setValue(settings->boardMarkerAlpha->reset().toDouble() * 100);

        settings->boardMarkerLightBackgroundSelectedColors->reset();
        QList<QColor> lightBackgroundSelectedColors = settings->boardMarkerLightBackgroundSelectedColors->colors();

        settings->boardMarkerDarkBackgroundSelectedColors->reset();
        QList<QColor> darkBackgroundSelectedColors = settings->boardMarkerDarkBackgroundSelectedColors->colors();

        for (int i = 0 ; i < settings->colorPaletteSize ; i++)
        {
            mMarkerProperties->lightBackgroundColorPickers[i]->setSelectedColorIndex(lightBackgroundSelectedColors.indexOf(settings->markerColors(false).at(i)));
            mMarkerProperties->darkBackgroundColorPickers[i]->setSelectedColorIndex(darkBackgroundSelectedColors.indexOf(settings->markerColors(true).at(i)));
        }
    }
    else if (mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->aboutTab)
    {
        bool defaultValue = settings->appEnableAutomaticSoftwareUpdates->reset().toBool();
        mPreferencesUI->checkSoftwareUpdateAtLaunchCheckBox->setChecked(defaultValue);

    }
    else if(mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->networkTab){
        bool defaultValue = settings->webUseExternalBrowser->reset().toBool();
        mPreferencesUI->useExternalBrowserCheckBox->setChecked(defaultValue);
        defaultValue = settings->webShowPageImmediatelyOnMirroredScreen->reset().toBool();
        mPreferencesUI->displayBrowserPageCheckBox->setChecked(defaultValue);

        mPreferencesUI->webHomePage->setText(settings->webHomePage->reset().toString());
    }
    else if(mPreferencesUI->mainTabWidget->currentWidget() == mPreferencesUI->gridTab)
    {
        settings->boardCrossColorDarkBackground->reset();
        settings->boardGridDarkBackgroundColors->reset();

        QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();
        QColor selectedCrossColorDarkBackground(settings->boardCrossColorDarkBackground->get().toString());

        mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);
        mDarkBackgroundGridColorPicker->setSelectedColorIndex(gridDarkBackgroundColors.indexOf(selectedCrossColorDarkBackground));
        int darkBackgroundOpacity = selectedCrossColorDarkBackground.alpha()*100 / 255;
        mPreferencesUI->darkBackgroundOpacitySlider->setValue(darkBackgroundOpacity);
        darkBackgroundCrossOpacityValueChanged(darkBackgroundOpacity);

        settings->boardCrossColorLightBackground->reset();
        settings->boardGridLightBackgroundColors->reset();

        QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();
        QColor selectedCrossColorLightBackground(settings->boardCrossColorLightBackground->get().toString());

        mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);
        mLightBackgroundGridColorPicker->setSelectedColorIndex(gridLightBackgroundColors.indexOf(selectedCrossColorLightBackground));
        int lightBackgroundOpacity = selectedCrossColorLightBackground.alpha()*100 / 255;
        mPreferencesUI->lightBackgroundOpacitySlider->setValue(lightBackgroundOpacity);
        lightBackgroundCrossOpacityValueChanged(lightBackgroundOpacity);

    }
}

void UBPreferencesController::darkBackgroundCrossOpacityValueChanged(int value)
{
    UBSettings* settings = UBSettings::settings();
    int opacity = value * 255 / 100;

    QList<QColor> gridDarkBackgroundColors = settings->boardGridDarkBackgroundColors->colors();

    int index = mDarkBackgroundGridColorPicker->selectedColorIndex();
    QColor currentColor = gridDarkBackgroundColors.at(index);
    currentColor.setAlpha(opacity);
    gridDarkBackgroundColors.replace(index, currentColor);
    mDarkBackgroundGridColorPicker->setColors(gridDarkBackgroundColors);

    settings->boardGridDarkBackgroundColors->setColor(index, currentColor);

    UBSettings::settings()->boardCrossColorDarkBackground->set(currentColor.name(QColor::HexArgb));

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::lightBackgroundCrossOpacityValueChanged(int value)
{
    UBSettings* settings = UBSettings::settings();
    int opacity = value * 255 / 100;

    QList<QColor> gridLightBackgroundColors = settings->boardGridLightBackgroundColors->colors();

    int index = mLightBackgroundGridColorPicker->selectedColorIndex();
    QColor currentColor = gridLightBackgroundColors.at(index);
    currentColor.setAlpha(opacity);
    gridLightBackgroundColors.replace(index, currentColor);
    mLightBackgroundGridColorPicker->setColors(gridLightBackgroundColors);

    settings->boardGridLightBackgroundColors->setColor(index, currentColor);

    UBSettings::settings()->boardCrossColorLightBackground->set(currentColor.name(QColor::HexArgb));

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::penPreviewFromSizeChanged(int value)
{
    UBSettings::settings()->setPenPreviewFromSize(value);
}

void UBPreferencesController::widthSliderChanged(int value)
{
    UBSettings* settings = UBSettings::settings();

    qreal width = value / sSliderRatio;

    QObject *slider = sender();
    UBCircleFrame* display = 0;

    if (slider == mPenProperties->fineSlider)
    {
        settings->boardPenFineWidth->set(width);
        display = mPenProperties->fineDisplayFrame;
    }
    else if (slider == mPenProperties->mediumSlider)
    {
        settings->boardPenMediumWidth->set(width);
        display = mPenProperties->mediumDisplayFrame;
    }
    else if (slider == mPenProperties->strongSlider)
    {
        settings->boardPenStrongWidth->set(width);
        display = mPenProperties->strongDisplayFrame;
    }
    else if (slider == mMarkerProperties->fineSlider)
    {
        settings->boardMarkerFineWidth->set(width);
        display = mMarkerProperties->fineDisplayFrame;
    }
    else if (slider == mMarkerProperties->mediumSlider)
    {
        settings->boardMarkerMediumWidth->set(width);
        display = mMarkerProperties->mediumDisplayFrame;
    }
    else if (slider == mMarkerProperties->strongSlider)
    {
        settings->boardMarkerStrongWidth->set(width);
        display = mMarkerProperties->strongDisplayFrame;
    }
    else
    {
        qDebug() << "widthSliderChanged : unknown sender ... ignoring event";
    }

    if (display)
    {
        display->currentPenWidth = width;
        display->maxPenWidth = sMaxPenWidth;
        display->repaint();
    }
}


void UBPreferencesController::opacitySliderChanged(int value)
{
    qreal opacity = ((qreal)value) / 100;

    QObject *slider = sender();

    if (slider == mMarkerProperties->opacitySlider)
    {
        UBDrawingController::drawingController()->setMarkerAlpha(opacity);
    }
}


void UBPreferencesController::colorSelected(const QColor& color)
{
    UBColorPicker *colorPicker = qobject_cast<UBColorPicker*>(sender());

    int index = mPenProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(false, color, index);
        return;
    }

    index = mPenProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setPenColor(true, color, index);
    }

    index = mMarkerProperties->lightBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(false, color, index);
    }

    index = mMarkerProperties->darkBackgroundColorPickers.indexOf(colorPicker);

    if (index >= 0)
    {
        UBDrawingController::drawingController()->setMarkerColor(true, color, index);
    }

}

void UBPreferencesController::setCrossColorOnDarkBackground(const QColor& color)
{
    UBSettings::settings()->boardCrossColorDarkBackground->set(color.name(QColor::HexArgb));

    mPreferencesUI->darkBackgroundOpacitySlider->setValue(color.alpha() * 100 / 255);

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::setCrossColorOnLightBackground(const QColor& color)
{
    UBSettings::settings()->boardCrossColorLightBackground->set(color.name(QColor::HexArgb));

    mPreferencesUI->lightBackgroundOpacitySlider->setValue(color.alpha() * 100 / 255);

    if (UBApplication::boardController && UBApplication::boardController->activeScene())
    {
        foreach(QGraphicsView* view, UBApplication::boardController->activeScene()->views())
            view->resetCachedContent();
    }
}

void UBPreferencesController::toolbarPositionChanged(bool checked)
{
    Q_UNUSED(checked);

    UBSettings* settings = UBSettings::settings();

    settings->appToolBarPositionedAtTop->set(mPreferencesUI->toolbarAtTopRadioButton->isChecked());

}

void UBPreferencesController::toolbarOrientationVertical(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(checked);
}

void UBPreferencesController::toolbarOrientationHorizontal(bool checked)
{
    UBSettings* settings = UBSettings::settings();
    settings->appToolBarOrientationVertical->set(!checked);
}

void UBPreferencesController::systemOSKCheckBoxToggled(bool checked)
{
    mPreferencesUI->keyboardPaletteKeyButtonSize->setVisible(!checked);
    mPreferencesUI->keyboardPaletteKeyButtonSize_Label->setVisible(!checked);
}

UBBrushPropertiesFrame::UBBrushPropertiesFrame(QFrame* owner, const QList<QColor>& lightBackgroundColors,
                                               const QList<QColor>& darkBackgroundColors, const QList<QColor>& lightBackgroundSelectedColors,
                                               const QList<QColor>& darkBackgroundSelectedColors, UBPreferencesController* controller)
{
    setupUi(owner);

    QPalette lightBackgroundPalette = QApplication::palette();
    lightBackgroundPalette.setColor(QPalette::Window, Qt::white);

    lightBackgroundFrame->setAutoFillBackground(true);
    lightBackgroundFrame->setPalette(lightBackgroundPalette);

    QPalette darkBackgroundPalette = QApplication::palette();
    darkBackgroundPalette.setColor(QPalette::Window, Qt::black);
    darkBackgroundPalette.setColor(QPalette::ButtonText, Qt::white);
    darkBackgroundPalette.setColor(QPalette::WindowText, Qt::white);

    darkBackgroundFrame->setAutoFillBackground(true);
    darkBackgroundFrame->setPalette(darkBackgroundPalette);
    darkBackgroundLabel->setPalette(darkBackgroundPalette);

    QList<QColor> firstLightBackgroundColor;
    firstLightBackgroundColor.append(lightBackgroundColors[0]);

    lightBackgroundColorPicker0->setColors(firstLightBackgroundColor);
    lightBackgroundColorPicker0->setSelectedColorIndex(0);
    lightBackgroundColorPickers.append(lightBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(lightBackgroundFrame);
        picker->setObjectName(QString("penLightBackgroundColor%1").arg(i));
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        lightBackgroundLayout->addWidget(picker);

        picker->setColors(lightBackgroundColors);

        picker->setSelectedColorIndex(lightBackgroundColors.indexOf(lightBackgroundSelectedColors.at(i)));

        lightBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }

    QList<QColor> firstDarkBackgroundColor;
    firstDarkBackgroundColor.append(darkBackgroundColors[0]);

    darkBackgroundColorPicker0->setColors(firstDarkBackgroundColor);
    darkBackgroundColorPicker0->setSelectedColorIndex(0);
    darkBackgroundColorPickers.append(darkBackgroundColorPicker0);

    for (int i = 1 ; i < UBSettings::settings()->colorPaletteSize ; i++)
    {
        UBColorPicker *picker = new UBColorPicker(darkBackgroundFrame);
        picker->setObjectName(QString("penDarkBackgroundColor%1").arg(i));
        picker->setMinimumSize(QSize(32, 32));
        picker->setFrameShape(QFrame::StyledPanel);
        picker->setFrameShadow(QFrame::Raised);

        darkBackgroundLayout->addWidget(picker);

        picker->setColors(darkBackgroundColors);
        picker->setSelectedColorIndex(darkBackgroundColors.indexOf(darkBackgroundSelectedColors.at(i)));

        darkBackgroundColorPickers.append(picker);

        QObject::connect(picker, SIGNAL(colorSelected(const QColor&)), controller, SLOT(colorSelected(const QColor&)));

    }
}

UBScreenListLineEdit::UBScreenListLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , mValidator(nullptr)
{
    connect(this, &QLineEdit::textChanged, this, &UBScreenListLineEdit::onTextChanged);
}

void UBScreenListLineEdit::setDefault()
{
    setText("");
}

void UBScreenListLineEdit::loadScreenList(const QStringList &screenList)
{
    setText(screenList.join(','));
}

void UBScreenListLineEdit::focusInEvent(QFocusEvent *focusEvent)
{
    QLineEdit::focusInEvent(focusEvent);

    if (mScreenLabels.empty())
    {
        QStringList screenList = UBStringUtils::trimmed(text().split(','));

        QList<QScreen*> screens = UBApplication::displayManager->availableScreens();
        QStringList availableScreenIndexes;
        int screenIndex = 1;
        QFont font;
        font.setPointSize(48);

        for (QScreen* screen : screens)
        {
            QString index = QString::number(screenIndex);
            availableScreenIndexes << index;

            QPushButton* button = new QPushButton(this);
            button->setWindowFlag(Qt::FramelessWindowHint, true);
            button->setWindowFlag(Qt::WindowStaysOnTopHint, true);
            button->setWindowFlag(Qt::X11BypassWindowManagerHint, true);
            button->setWindowFlag(Qt::Window, true);
            button->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);
            button->setAttribute(Qt::WA_ShowWithoutActivating, true);
            button->setProperty("screenIndex", index);
#ifdef QT_DEBUG
            button->setText(index + "(" + screen->name() + ")");
#else
            button->setText(index);
#endif
            button->setFont(font);
            button->move(screen->geometry().topLeft());
            button->setMinimumSize(300, 150);
            button->setDisabled(screenList.contains(index));
            button->setCursor(Qt::PointingHandCursor);
            button->show();

            connect(button, &QPushButton::pressed, this, &UBScreenListLineEdit::addScreen);

            mScreenLabels << button;
            ++screenIndex;
        }

        if (!mValidator)
        {
            mValidator = new UBStringListValidator(this);
            setValidator(mValidator);
        }

        mValidator->setValidationStringList(availableScreenIndexes);
    }
}

void UBScreenListLineEdit::focusOutEvent(QFocusEvent *focusEvent)
{
    QLineEdit::focusOutEvent(focusEvent);
    qDeleteAll(mScreenLabels);
    mScreenLabels.clear();
}

void UBScreenListLineEdit::addScreen()
{
    QPushButton* button = dynamic_cast<QPushButton*>(sender());

    if (button)
    {
        QString list = text();
        mValidator->fixup(list);
        QString screenIndex = button->property("screenIndex").toString();

        if (list.isEmpty())
        {
            setText(screenIndex);
        }
        else
        {
            setText(list + "," + screenIndex);
        }

        button->setEnabled(false);
    }
}

void UBScreenListLineEdit::onTextChanged(const QString &input)
{
    const QStringList screenList = UBStringUtils::trimmed(input.split(','));

    for (QPushButton* button : std::as_const(mScreenLabels))
    {
        button->setDisabled(screenList.contains(button->property("screenIndex").toString()));
    }

    if (input.isEmpty() || input.right(1) == ',')
    {
        // create and attach a new QCompleter
        QStringList model;

        for (QPushButton* button : std::as_const(mScreenLabels))
        {
            if (button->isEnabled())
            {
                model << input + button->property("screenIndex").toString();
            }
        }
    }

    // user indication of acceptable input
    if (hasAcceptableInput())
    {
        setStyleSheet("");
        emit screenListChanged(screenList);
    }
    else
    {
        setStyleSheet("QLineEdit { background-color: #FFB3C8; }");
    }
}

UBStringListValidator::UBStringListValidator(QObject *parent)
    : QValidator(parent)
{

}

void UBStringListValidator::fixup(QString &input) const
{
    // remove invalid tokens from list, trim tokens
    QStringList inputList = UBStringUtils::trimmed(input.split(','));
    QStringList outputList;

    for (const QString& token : inputList)
    {
        if (mList.contains(token))
        {
            outputList << token;
        }
    }

    input = outputList.join(',');
}

QValidator::State UBStringListValidator::validate(QString &input, int &) const
{
    bool ok = true;
    QStringList inputList = UBStringUtils::trimmed(input.split(','));
    // number of commas must match number of list items - 1
    int commas = input.count(',');

    if (commas && commas + 1 != inputList.size())
    {
        ok = false;
    }

    for (const QString& token : inputList)
    {
        ok &= mList.contains(token) && inputList.count(token) == 1;
    }

    return ok ? Acceptable : Intermediate;
}

void UBStringListValidator::setValidationStringList(const QStringList &list)
{
    mList = list;
}
