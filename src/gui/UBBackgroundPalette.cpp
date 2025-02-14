#include "UBBackgroundPalette.h"

#include "gui/UBBackgroundManager.h"
#include "gui/UBMainWindow.h"

UBBackgroundPalette::UBBackgroundPalette(QWidget * parent)
     : UBActionPalette(parent)
{
    init();
    createActions();

    connect(UBApplication::boardController->backgroundManager(), &UBBackgroundManager::preferredBackgroundChanged,
            this, &UBBackgroundPalette::createActions);
}

void UBBackgroundPalette::init()
{
    UBActionPalette::clearLayout();
    delete layout();


    m_customCloseProcessing = false;

    mButtonSize = QSize(32, 32);
    mIsClosable = false;
    mAutoClose = false;
    mToolButtonStyle = Qt::ToolButtonIconOnly;
    mButtons.clear();

    mVLayout = new QVBoxLayout(this);
    mTopLayout = new QHBoxLayout();
    mBottomLayout = new QHBoxLayout();

    mVLayout->addLayout(mTopLayout);
    mVLayout->addLayout(mBottomLayout);

    mSlider = new QSlider(Qt::Horizontal);

    mSlider->setMinimum(UBSettings::settings()->minCrossSize);
    mSlider->setMaximum(UBSettings::settings()->maxCrossSize);
    mSlider->setSingleStep(2);
    mSlider->setTracking(true); // valueChanged() is emitted during movement and not just upon releasing the slider

    mSliderLabel = new QLabel(tr("Grid size"));
    mLightDarkModeLabel = new QLabel(tr("Switch background color"));

    mResetDefaultGridSizeButton = createPaletteButton(UBApplication::mainWindow->actionDefaultGridSize, this);
    mResetDefaultGridSizeButton->setFixedSize(24,24);
    mActions << UBApplication::mainWindow->actionDefaultGridSize;
    UBApplication::mainWindow->actionDefaultGridSize->setProperty("ungrouped", true); // don't add to action group

    connect(UBApplication::mainWindow->actionDefaultGridSize, &QAction::triggered, this, &UBBackgroundPalette::defaultBackgroundGridSize);

    mLightDarkModeSwitch = createPaletteButton(UBApplication::mainWindow->actionLightDarkMode, this);
    mLightDarkModeSwitch->setFixedSize(44,26);
    mLightDarkModeSwitch->setCheckable(true);
    mActions << UBApplication::mainWindow->actionLightDarkMode;
    UBApplication::mainWindow->actionLightDarkMode->setProperty("ungrouped", true); // don't add to action group

    connect(UBApplication::mainWindow->actionLightDarkMode, &QAction::triggered, this, &UBBackgroundPalette::toggleBackgroundColor);

    mBottomLayout->addSpacing(16);
    mBottomLayout->addWidget(mSliderLabel);
    mBottomLayout->addWidget(mSlider);
    mBottomLayout->addWidget(mResetDefaultGridSizeButton);
    mBottomLayout->addSpacing(16);

    mBottomLayout->addWidget(mLightDarkModeLabel);
    mBottomLayout->addWidget(mLightDarkModeSwitch);
    mBottomLayout->addSpacing(16);

    updateLayout();
}

void UBBackgroundPalette::createActions()
{
    // first delete everything from the top layout
    while(!mTopLayout->isEmpty())
    {
        QLayoutItem* pItem = mTopLayout->itemAt(0);
        QWidget* pW = pItem->widget();
        mTopLayout->removeItem(pItem);
        delete pItem;
        mTopLayout->removeWidget(pW);

        QAction* action = removePaletteButton(dynamic_cast<UBActionPaletteButton*>(pW));

        delete pW;
        delete action;
    }

    // then add actions for the first 6 backgrounds from the background manager
    const QList<const UBBackgroundRuling*> backgrounds = UBApplication::boardController->backgroundManager()->backgrounds();
    const auto dark = false; //UBApplication::boardController->activeScene()->isDarkBackground();
    const auto bgManager = UBApplication::boardController->backgroundManager();

    for (int i = 0; i < backgrounds.size() && i < 6; ++i)
    {
        const auto background = backgrounds.at(i);
        auto action = bgManager->backgroundAction(*background, dark);
        UBBackgroundPalette::addAction(action);

        connect(action, &QAction::triggered, this, &UBBackgroundPalette::changeBackground);
    }

    actionChanged();
    updateActions();
}

void UBBackgroundPalette::updateActions()
{
    const auto scene = UBApplication::boardController->activeScene();

    if (!scene)
    {
        return;
    }

    const auto dark = scene->isDarkBackground();
    UBApplication::mainWindow->actionLightDarkMode->setChecked(dark);
    QUuid uuid;

    if (scene->background())
    {
        uuid = scene->background()->uuid();
    }

    for (auto& action : mActions)
    {
        QUuid actionUuid = action->property("uuid").toUuid();

        if (!actionUuid.isNull())
        {
            UBApplication::boardController->backgroundManager()->updateAction(action, dark);
            action->setChecked(uuid == actionUuid);
        }
    }
}

void UBBackgroundPalette::addAction(QAction* action)
{
    UBActionPaletteButton* button = createPaletteButton(action, this);

    mTopLayout->addWidget(button);
    mActions << action;
}

void UBBackgroundPalette::setActions(QList<QAction*> actions)
{
    mMapActionToButton.clear();

    foreach(QAction* action, actions)
    {
        addAction(action);
    }

    actionChanged();
}

void UBBackgroundPalette::updateLayout()
{
    if (mToolButtonStyle == Qt::ToolButtonIconOnly) {
        mVLayout->setContentsMargins (sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border()
                , sLayoutContentMargin / 2  + border(), sLayoutContentMargin / 2  + border());
    }
    else
    {
        mVLayout->setContentsMargins (sLayoutContentMargin  + border(), sLayoutContentMargin  + border()
                , sLayoutContentMargin  + border(), sLayoutContentMargin + border());

    }
   update();
}

void UBBackgroundPalette::clearLayout()
{
    while(!mTopLayout->isEmpty()) {
        QLayoutItem* pItem = mTopLayout->itemAt(0);
        QWidget* pW = pItem->widget();
        mTopLayout->removeItem(pItem);
        delete pItem;
        mTopLayout->removeWidget(pW);
        delete pW;
    }

    delete mTopLayout;

    while(!mBottomLayout->isEmpty()) {
        QLayoutItem* pItem = mBottomLayout->itemAt(0);
        QWidget* pW = pItem->widget();
        mBottomLayout->removeItem(pItem);
        delete pItem;
        mBottomLayout->removeWidget(pW);
        delete pW;
    }

    delete mBottomLayout;

    delete mVLayout;

    mActions.clear();
    mButtons.clear();
}

void UBBackgroundPalette::showEvent(QShowEvent* event)
{
    backgroundChanged();

    mSlider->setValue(UBApplication::boardController->activeScene()->backgroundGridSize());
    connect(mSlider, SIGNAL(valueChanged(int)),
            this, SLOT(sliderValueChanged(int)));

    const auto scene = UBApplication::boardController->activeScene();
    mIsDark = scene->isDarkBackground();
    mLightDarkModeSwitch->setChecked(mIsDark);
    mBackground = scene->background();

    QWidget::showEvent(event);
}

void UBBackgroundPalette::sliderValueChanged(int value)
{
    UBApplication::boardController->activeScene()->setBackgroundGridSize(value);
    UBSettings::settings()->crossSize = value; // since this function is called (indirectly, by refresh) when we switch scenes, the settings will always have the current scene's cross size.

    UBApplication::boardController->activeScene()->setModified(true);
}

void UBBackgroundPalette::defaultBackgroundGridSize()
{
    mSlider->setValue(UBSettings::settings()->defaultCrossSize);
    sliderValueChanged(UBSettings::settings()->defaultCrossSize);
}

void UBBackgroundPalette::toggleBackgroundColor(bool checked)
{
    mIsDark = checked;
    UBApplication::boardController->setBackground(mIsDark, mBackground);
    backgroundChanged();
}

void UBBackgroundPalette::changeBackground(bool checked)
{
    if (checked)
    {
        QAction* action = dynamic_cast<QAction*>(sender());

        if (action)
        {
            const auto uuid = action->property("uuid").toUuid();
            mBackground = UBApplication::boardController->backgroundManager()->background(uuid);
            UBApplication::boardController->setBackground(mIsDark, mBackground);
        }
    }
}

void UBBackgroundPalette::backgroundChanged()
{
    bool dark = UBApplication::boardController->activeScene()->isDarkBackground();

    if (dark)
    {
        mSliderLabel->setStyleSheet("QLabel { color : white; }");
        mLightDarkModeLabel->setStyleSheet("QLabel { color : white; }");
    }
    else
    {
        mSliderLabel->setStyleSheet("QLabel { color : black; }");
        mLightDarkModeLabel->setStyleSheet("QLabel { color : black; }");
    }

    updateActions();
}

void UBBackgroundPalette::refresh()
{
    backgroundChanged();
    mSlider->setValue(UBApplication::boardController->activeScene()->backgroundGridSize());
}
