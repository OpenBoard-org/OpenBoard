#include "UBBackgroundPalette.h"

#include "gui/UBMainWindow.h"

UBBackgroundPalette::UBBackgroundPalette(QList<QAction*> actions, QWidget * parent)
    : UBActionPalette(parent)
{
    init();
    setActions(actions);
}


UBBackgroundPalette::UBBackgroundPalette(QWidget * parent)
     : UBActionPalette(parent)
{
    init();
}



void UBBackgroundPalette::init()
{
    UBActionPalette::clearLayout();
    delete layout();


    m_customCloseProcessing = false;

    mButtonSize = QSize(32, 32);
    mIsClosable = false;
    mAutoClose = false;
    mButtonGroup = 0;
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
    mIntermediateLinesLabel = new QLabel(tr("Draw intermediate grid lines"));

    mResetDefaultGridSizeButton = createPaletteButton(UBApplication::mainWindow->actionDefaultGridSize, this);
    mResetDefaultGridSizeButton->setFixedSize(24,24);
    mActions << UBApplication::mainWindow->actionDefaultGridSize;

    connect(UBApplication::mainWindow->actionDefaultGridSize, SIGNAL(triggered()), this, SLOT(defaultBackgroundGridSize()));

    bool enableIntermediateLines = UBSettings::settings()->enableIntermediateLines->get().toBool();

    if (enableIntermediateLines)
    {
        mDrawIntermediateLinesCheckBox = createPaletteButton(UBApplication::mainWindow->actionDrawIntermediateGridLines, this);
        mDrawIntermediateLinesCheckBox->setFixedSize(24,24);
        mDrawIntermediateLinesCheckBox->setCheckable(true);
        mActions << UBApplication::mainWindow->actionDrawIntermediateGridLines;
        mButtons.removeLast(); // don't add to button group

        connect(UBApplication::mainWindow->actionDrawIntermediateGridLines, SIGNAL(toggled(bool)), this, SLOT(toggleIntermediateLines(bool)));
    }

    mBottomLayout->addSpacing(16);
    mBottomLayout->addWidget(mSliderLabel);
    mBottomLayout->addWidget(mSlider);
    mBottomLayout->addWidget(mResetDefaultGridSizeButton);
    mBottomLayout->addSpacing(16);

    if (enableIntermediateLines)
    {
        mBottomLayout->addWidget(mIntermediateLinesLabel);
        mBottomLayout->addWidget(mDrawIntermediateLinesCheckBox);
        mBottomLayout->addSpacing(16);
    }

    updateLayout();
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

    if (UBSettings::settings()->enableIntermediateLines->get().toBool())
    {
        mDrawIntermediateLinesCheckBox->setChecked(UBApplication::boardController->activeScene()->intermediateLines());
    }

    QWidget::showEvent(event);
}

void UBBackgroundPalette::sliderValueChanged(int value)
{
    UBApplication::boardController->activeScene()->setBackgroundGridSize(value);
    UBSettings::settings()->crossSize = value; // since this function is called (indirectly, by refresh) when we switch scenes, the settings will always have the current scene's cross size.
}

void UBBackgroundPalette::defaultBackgroundGridSize()
{
    mSlider->setValue(UBSettings::settings()->defaultCrossSize);
    sliderValueChanged(UBSettings::settings()->defaultCrossSize);
}

void UBBackgroundPalette::toggleIntermediateLines(bool checked)
{
    UBApplication::boardController->activeScene()->setIntermediateLines(checked);
    UBSettings::settings()->intermediateLines = checked; // since this function is called (indirectly, by refresh) when we switch scenes, the settings will always have the current scene's value.
}

void UBBackgroundPalette::backgroundChanged()
{
    bool dark = UBApplication::boardController->activeScene()->isDarkBackground();

    if (dark)
    {
        mSliderLabel->setStyleSheet("QLabel { color : white; }");
        mIntermediateLinesLabel->setStyleSheet("QLabel { color : white; }");
    }
    else
    {
        mSliderLabel->setStyleSheet("QLabel { color : black; }");
        mIntermediateLinesLabel->setStyleSheet("QLabel { color : black; }");
    }
}

void UBBackgroundPalette::refresh()
{
    backgroundChanged();
    mSlider->setValue(UBApplication::boardController->activeScene()->backgroundGridSize());
}
