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

#include "UBPodcastPreferencesDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "gui/UBMainWindow.h"


UBPodcastPreferencesDialog::UBPodcastPreferencesDialog(QWidget* parent,
                                                   const QStringList& podcastAudioRecordingDevices,
                                                   const QString podcastAudioRecordingOption,
                                                   const QStringList& podcastProfileNames,
                                                   const QString podcastProfile,
                                                   const int podcastFrameRate,
                                                   const int podcastBitRate,
                                                   const QList<int>& podcastBitRateDivisor,
                                                   const QList<int>& podcastVerticalResolution,
                                                   const QList<int>& podcastHorizontalResolution,
                                                   const bool podcastPublishToIntranet,
                                                   const bool podcastPublishToYoutube)
    : QDialog(parent)
    , mPodcastAudioRecordingDevices(podcastAudioRecordingDevices)
    , mPodcastProfileNames(podcastProfileNames)
    , mPodcastBitRateDivisor(podcastBitRateDivisor)
{
    setModal(true);
    setWindowTitle(tr("Podcast Preferences"));
    setMinimumWidth(520);
    setObjectName(QStringLiteral("podcastPreferencesDialog"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 18, 20, 18);
    mainLayout->setSpacing(16);

    QWidget* recordingWidget = new QWidget(this);
    QVBoxLayout* recordingLayout = new QVBoxLayout(recordingWidget);
    recordingLayout->setContentsMargins(20, 18, 20, 18);
    recordingLayout->setSpacing(16);

    const QString audioDeviceSelectorToolTip = tr("The selected audio device applies to all profiles.");

    QWidget* audioWidget = new QWidget(recordingWidget);
    audioWidget->setObjectName(QStringLiteral("podcastPreferencesHeader"));
    QHBoxLayout* audioLayout = new QHBoxLayout(audioWidget);
    audioLayout->setContentsMargins(14, 12, 14, 12);
    audioLayout->setSpacing(12);

    QLabel* audioLabel = new QLabel(tr("Audio Recording Device"), audioWidget);
    audioLabel->setObjectName(QStringLiteral("podcastPreferencesHeaderLabel"));
    audioLabel->setToolTip(audioDeviceSelectorToolTip);
    mNoAudioCheckBox = new QCheckBox(tr("Don't record audio"));
    mNoAudioCheckBox->setToolTip(audioDeviceSelectorToolTip);
    mDefaultAudioCheckBox = new QCheckBox(tr("Use default audio input"));
    mDefaultAudioCheckBox->setToolTip(audioDeviceSelectorToolTip);
    mAudioDeviceSelector = new QComboBox(audioWidget);
    mAudioDeviceSelector->addItems(mPodcastAudioRecordingDevices);
    mAudioDeviceSelector->setToolTip(audioDeviceSelectorToolTip);
    connect(mNoAudioCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleNoAudioDevice(bool)));
    connect(mDefaultAudioCheckBox, SIGNAL(toggled(bool)), this, SLOT(toggleDefaultAudioDevice(bool)));
    if (podcastAudioRecordingOption == "None")
    {
        setNoAudioDevice();
    }
    else if (podcastAudioRecordingOption == "Default")
    {
        setDefaultAudioDevice();
    }
    else
    {
        mAudioDeviceSelector->setCurrentIndex(mPodcastAudioRecordingDevices.indexOf(podcastAudioRecordingOption));
    }
    audioLayout->addWidget(audioLabel);
    audioLayout->addWidget(mNoAudioCheckBox);
    audioLayout->addWidget(mDefaultAudioCheckBox);
    audioLayout->addWidget(mAudioDeviceSelector);
    recordingLayout->addWidget(audioWidget);

    const QString frameRateFormToolTip = tr("The global frame rate applies to all profiles.");

    QWidget* frameRateFormWidget = new QWidget(recordingWidget);
    frameRateFormWidget->setObjectName(QStringLiteral("podcastPreferencesSpinBox"));
    QHBoxLayout* frameRateFormLayout = new QHBoxLayout(frameRateFormWidget);
    frameRateFormLayout->setContentsMargins(14, 12, 14, 12);
    frameRateFormLayout->setSpacing(12);
    mFrameRateSpinBox = new QSpinBox(frameRateFormWidget);
    mFrameRateSpinBox->setRange(0, 1000000);
    mFrameRateSpinBox->setValue(podcastFrameRate);
    mFrameRateSpinBox->setSingleStep(1);
    mFrameRateSpinBox->setToolTip(frameRateFormToolTip);
    connect(mFrameRateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(frameRateChanged(int)));
    QLabel* frameRateLabel = new QLabel(tr("Global frame rate"), frameRateFormWidget);
    frameRateLabel->setObjectName(QStringLiteral("podcastPreferencesHeaderLabel"));
    frameRateLabel->setToolTip(frameRateFormToolTip);
    frameRateFormLayout->addWidget(frameRateLabel);
    frameRateFormLayout->addWidget(mFrameRateSpinBox);
    recordingLayout->addWidget(frameRateFormWidget);

    const QString bitRateFormToolTip = tr("The base bit rate is scaled depending on the profile.");

    QWidget* bitRateFormWidget = new QWidget(recordingWidget);
    bitRateFormWidget->setObjectName(QStringLiteral("podcastPreferencesSpinBox"));
    QHBoxLayout* bitRateFormLayout = new QHBoxLayout(bitRateFormWidget);
    bitRateFormLayout->setContentsMargins(14, 12, 14, 12);
    bitRateFormLayout->setSpacing(12);
    mBitRateSpinBox = new QSpinBox(bitRateFormWidget);
    mBitRateSpinBox->setRange(0, 1000000000);
    mBitRateSpinBox->setValue(podcastBitRate);
    mBitRateSpinBox->setSingleStep(1000);
    mBitRateSpinBox->setToolTip(bitRateFormToolTip);
    connect(mBitRateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(bitRateChanged(int)));
    QLabel* bitRateLabel = new QLabel(tr("Base bit rate"), bitRateFormWidget);
    bitRateLabel->setObjectName(QStringLiteral("podcastPreferencesHeaderLabel"));
    bitRateLabel->setToolTip(bitRateFormToolTip);
    bitRateFormLayout->addWidget(bitRateLabel);
    bitRateFormLayout->addWidget(mBitRateSpinBox);
    recordingLayout->addWidget(bitRateFormWidget);

    mTabWidget = new QTabWidget(recordingWidget);
    mTabWidget->setObjectName(QStringLiteral("podcastPreferencesTabWidget"));
    mTabWidget->setIconSize(QSize(18, 18));

    for (int i = 0; i < mPodcastProfileNames.size(); ++i)
    {
        const QString tabLabel = mPodcastProfileNames[i] == QString("Full")   ? tr("Full")
                               : mPodcastProfileNames[i] == QString("Medium") ? tr("Medium")
                               : mPodcastProfileNames[i] == QString("Small")  ? tr("Small")
                               : mPodcastProfileNames[i];
        QWidget* tab = createProfileTab(mBitRateProfileValues.emplaceBack(),
                                        mFrameRateProfileValues.emplaceBack(),
                                        podcastBitRate / mPodcastBitRateDivisor[i],
                                        podcastFrameRate,
                                        podcastVerticalResolution[i],
                                        podcastHorizontalResolution[i]);
        tab->setObjectName(QStringLiteral("podcastPreferencesPenTab"));
        mTabWidget->addTab(tab, tabLabel);
    }
    mTabWidget->setCurrentIndex(mPodcastProfileNames.indexOf(podcastProfile));
    recordingLayout->addWidget(mTabWidget);

    QWidget* hintWidget = new QWidget(recordingWidget);
    hintWidget->setObjectName(QStringLiteral("podcastPreferencesHint"));
    QHBoxLayout* hintLayout = new QHBoxLayout(hintWidget);
    hintLayout->setContentsMargins(14, 10, 14, 10);
    hintLayout->setSpacing(10);

    QLabel* hintIcon = new QLabel(hintWidget);
    hintIcon->setObjectName(QStringLiteral("podcastPreferencesHintIcon"));
    hintIcon->setPixmap(QIcon(QStringLiteral(":/images/toolbar/tip.png")).pixmap(20, 20));
    hintIcon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel* hintLabel = new QLabel(
        tr("Frame rate and bit rate are dynamically adjusted during recording to record changes while keeping the file size small."),
        hintWidget);
    hintLabel->setObjectName(QStringLiteral("podcastPreferencesHintText"));
    hintLabel->setWordWrap(true);

    hintLayout->addWidget(hintIcon, 0, Qt::AlignTop);
    hintLayout->addWidget(hintLabel, 1);
    recordingLayout->addWidget(hintWidget);

    QWidget* publish = new QWidget(this);
    QVBoxLayout* publishLayout = new QVBoxLayout(publish);
    publishLayout->setContentsMargins(8, 14, 8, 6);
    publishLayout->setSpacing(14);

    mPublishToIntranet = new QCheckBox(tr("Publish to Intranet"));
    mPublishToIntranet->setChecked(podcastPublishToIntranet);
    mPublishToYoutube = new QCheckBox(tr("Publish to Youtube"));
    mPublishToYoutube->setChecked(podcastPublishToYoutube);
    publishLayout->addWidget(mPublishToIntranet);
    publishLayout->addWidget(mPublishToYoutube);

    publishLayout->addStretch();

    QTabWidget* mainTabs = new QTabWidget(this);
    mainTabs->addTab(recordingWidget, tr("Recording"));
    mainTabs->addTab(publish, tr("Upload"));
    mainLayout->addWidget(mainTabs);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->setObjectName(QStringLiteral("podcastPreferencesButtonBox"));
    QPushButton* defaultSettingsButton = buttons->addButton(tr("Default settings"), QDialogButtonBox::ResetRole);
    defaultSettingsButton->setObjectName(QStringLiteral("podcastPreferencesResetButton"));
    connect(defaultSettingsButton, SIGNAL(clicked()), this, SLOT(resetDefaultSettings()));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttons);
}


QString UBPodcastPreferencesDialog::podcastAudioRecordingOption() const
{
    if (mNoAudioCheckBox && mNoAudioCheckBox->isChecked())
    {
        return "None";
    }
    if (mDefaultAudioCheckBox && mDefaultAudioCheckBox->isChecked())
    {
        return "Default";
    }
    const int currentIndex = mAudioDeviceSelector->currentIndex();
    if (0 <= currentIndex && currentIndex < mPodcastAudioRecordingDevices.size())
    {
        return mPodcastAudioRecordingDevices[currentIndex];
    }
    return "";
}

QString UBPodcastPreferencesDialog::podcastProfile() const
{
    const int currentIndex = mTabWidget->currentIndex();
    if (0 <= currentIndex && currentIndex < mPodcastProfileNames.size())
    {
        return mPodcastProfileNames[currentIndex];
    }
    return "";
}

int UBPodcastPreferencesDialog::podcastBitRate() const
{
    return mBitRateSpinBox->value();
}

int UBPodcastPreferencesDialog::podcastFrameRate() const
{
    return mFrameRateSpinBox->value();
}

bool UBPodcastPreferencesDialog::podcastPublishToIntranet() const
{
    return mPublishToIntranet->isChecked();
}

bool UBPodcastPreferencesDialog::podcastPublishToYoutube() const
{
    return mPublishToYoutube->isChecked();
}

void UBPodcastPreferencesDialog::bitRateChanged(int bitRate)
{
    for (int i = 0; i < mBitRateProfileValues.size(); ++i)
    {
        mBitRateProfileValues[i]->setNum(bitRate / mPodcastBitRateDivisor[i]);
    }
}

void UBPodcastPreferencesDialog::frameRateChanged(int frameRate)
{
    for (QLabel* label : mFrameRateProfileValues)
    {
        label->setNum(frameRate);
    }
}

void UBPodcastPreferencesDialog::toggleNoAudioDevice(bool checked)
{
    if (checked)
    {
        setNoAudioDevice();
    }
    else
    {
        setDefaultAudioDevice();
    }
}

void UBPodcastPreferencesDialog::setNoAudioDevice()
{
    mNoAudioCheckBox->setChecked(true);
    mNoAudioCheckBox->setEnabled(true);
    mDefaultAudioCheckBox->setChecked(false);
    mDefaultAudioCheckBox->setEnabled(false);
    mAudioDeviceSelector->setEnabled(false);
}

void UBPodcastPreferencesDialog::toggleDefaultAudioDevice(bool checked)
{
    if (checked)
    {
        setDefaultAudioDevice();
    }
    else if (mNoAudioCheckBox && !mNoAudioCheckBox->isChecked())
    {
        mAudioDeviceSelector->setEnabled(true);
    }
}

void UBPodcastPreferencesDialog::setDefaultAudioDevice()
{
    mNoAudioCheckBox->setChecked(false);
    mNoAudioCheckBox->setEnabled(true);
    mDefaultAudioCheckBox->setChecked(true);
    mDefaultAudioCheckBox->setEnabled(true);
    mAudioDeviceSelector->setEnabled(false);
}

void UBPodcastPreferencesDialog::resetDefaultSettings()
{
    setDefaultAudioDevice();
    mTabWidget->setCurrentIndex(mPodcastProfileNames.indexOf("Medium"));
    mBitRateSpinBox->setValue(1700000);
    mFrameRateSpinBox->setValue(10);
    mPublishToIntranet->setChecked(false);
    mPublishToYoutube->setChecked(false);
}


QWidget* UBPodcastPreferencesDialog::createProfileTab(QLabel* &bitRateValue,
                                                 QLabel* &frameRateValue,
                                                 const int bitRate,
                                                 const int frameRate,
                                                 const int height,
                                                 const int width)
{
    QWidget* tab = new QWidget(this);
    QVBoxLayout* tabLayout = new QVBoxLayout(tab);
    tabLayout->setContentsMargins(8, 14, 8, 6);
    tabLayout->setSpacing(14);

    const QString bitRateToolTip = tr("The bit rate determines the quality of each frame.");
    QWidget* bitRateWidget = new QWidget(this);
    bitRateWidget->setObjectName(QStringLiteral("podcastPreferencesHeader"));
    QHBoxLayout* bitRateLayout = new QHBoxLayout(bitRateWidget);
    bitRateLayout->setContentsMargins(14, 12, 14, 12);
    bitRateLayout->setSpacing(12);
    QLabel* bitRateLabel = new QLabel(tr("Bit rate"), bitRateWidget);
    bitRateLabel->setAlignment(Qt::AlignCenter);
    bitRateLabel->setToolTip(bitRateToolTip);
    bitRateValue = new QLabel(QString::number(bitRate), bitRateWidget);
    bitRateValue->setObjectName(QStringLiteral("podcastPreferencesProfileTabValue"));
    bitRateValue->setAlignment(Qt::AlignCenter);
    bitRateValue->setToolTip(bitRateToolTip);
    bitRateLayout->addWidget(bitRateLabel);
    bitRateLayout->addWidget(bitRateValue);
    tabLayout->addWidget(bitRateWidget);

    const QString frameRateToolTip = tr("The frame rate determines how fast changes are recorded.");
    QWidget* frameRateWidget = new QWidget(this);
    frameRateWidget->setObjectName(QStringLiteral("podcastPreferencesHeader"));
    QHBoxLayout* frameRateLayout = new QHBoxLayout(frameRateWidget);
    frameRateLayout->setContentsMargins(14, 12, 14, 12);
    frameRateLayout->setSpacing(12);
    QLabel* frameRateLabel = new QLabel(tr("Frame rate"), frameRateWidget);
    frameRateLabel->setAlignment(Qt::AlignCenter);
    frameRateLabel->setToolTip(frameRateToolTip);
    frameRateValue = new QLabel(QString::number(frameRate), frameRateWidget);
    frameRateValue->setObjectName(QStringLiteral("podcastPreferencesProfileTabValue"));
    frameRateValue->setAlignment(Qt::AlignCenter);
    frameRateValue->setToolTip(frameRateToolTip);
    frameRateLayout->addWidget(frameRateLabel);
    frameRateLayout->addWidget(frameRateValue);
    tabLayout->addWidget(frameRateWidget);

    const QString widthToolTip = tr("The width determines the horizontal detail of each frame.");
    QWidget* widthWidget = new QWidget(this);
    widthWidget->setObjectName(QStringLiteral("podcastPreferencesHeader"));
    QHBoxLayout* widthLayout = new QHBoxLayout(widthWidget);
    widthLayout->setContentsMargins(14, 12, 14, 12);
    widthLayout->setSpacing(12);
    QLabel* widthLabel = new QLabel(tr("Horizontal Resolution"), widthWidget);
    widthLabel->setAlignment(Qt::AlignCenter);
    widthLabel->setToolTip(widthToolTip);
    QLabel* widthValue = new QLabel(QString::number(width), widthWidget);
    widthValue->setObjectName(QStringLiteral("podcastPreferencesProfileTabValue"));
    widthValue->setAlignment(Qt::AlignCenter);
    widthValue->setToolTip(widthToolTip);
    widthLayout->addWidget(widthLabel);
    widthLayout->addWidget(widthValue);
    tabLayout->addWidget(widthWidget);

    const QString heightToolTip = tr("The height determines the vertical detail of each frame.");
    QWidget* heightWidget = new QWidget(this);
    heightWidget->setObjectName(QStringLiteral("podcastPreferencesHeader"));
    QHBoxLayout* heightLayout = new QHBoxLayout(heightWidget);
    heightLayout->setContentsMargins(14, 12, 14, 12);
    heightLayout->setSpacing(12);
    QLabel* heightLabel = new QLabel(tr("Vertical Resolution"), heightWidget);
    heightLabel->setAlignment(Qt::AlignCenter);
    heightLabel->setToolTip(heightToolTip);
    QLabel* heightValue = new QLabel(QString::number(height), heightWidget);
    heightValue->setObjectName(QStringLiteral("podcastPreferencesProfileTabValue"));
    heightValue->setAlignment(Qt::AlignCenter);
    heightValue->setToolTip(heightToolTip);
    heightLayout->addWidget(heightLabel);
    heightLayout->addWidget(heightValue);
    tabLayout->addWidget(heightWidget);

    return tab;
}
