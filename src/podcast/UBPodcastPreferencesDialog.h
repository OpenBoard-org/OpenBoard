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




#ifndef UBPODCASTPREFERENCESDIALOG_H
#define UBPODCASTPREFERENCESDIALOG_H

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <QSpinBox>

class QWidget;
class QLabel;
class QTabWidget;
class QPushButton;

class UBPodcastPreferencesDialog : public QDialog
{
    Q_OBJECT

    public:
        UBPodcastPreferencesDialog(QWidget* parent,
                                 const QStringList& podcastAudioRecordingOptions,
                                 const QString podcastAudioRecordingDevice,
                                 const QStringList& podcastProfileNames,
                                 const QString podcastProfile,
                                 const int podcastFrameRate,
                                 const int podcastBitRate,
                                 const QList<int>& podcastBitRateDivisors,
                                 const QList<int>& podcastVerticalResolution,
                                 const QList<int>& podcastHorizontalResolution);

        QString podcastAudioRecordingOption() const;
        QString podcastProfile() const;
        int podcastBitRate() const;
        int podcastFrameRate() const;

    private slots:
        void bitRateChanged(int bitRate);
        void frameRateChanged(int frameRate);
        void toggleNoAudioDevice(bool checked);
        void toggleDefaultAudioDevice(bool checked);
        void resetDefaultSettings();

    private:
        QWidget* createProfileTab(QLabel* &bitRateValue,
                                  QLabel* &frameRateValue,
                                  const int bitRate,
                                  const int frameRate,
                                  const int height,
                                  const int width);
        void setNoAudioDevice();
        void setDefaultAudioDevice();

        QCheckBox* mDefaultAudioCheckBox{nullptr};
        QCheckBox* mNoAudioCheckBox{nullptr};
        QComboBox* mAudioDeviceSelector{nullptr};
        QSpinBox* mBitRateSpinBox{nullptr};
        QSpinBox* mFrameRateSpinBox{nullptr};
        QTabWidget* mTabWidget{nullptr};
        QPushButton* mResetPenButton{nullptr};
        QPushButton* mResetMarkerButton{nullptr};
        QList<QLabel*> mBitRateProfileValues;
        QList<QLabel*> mFrameRateProfileValues;
        QStringList mPodcastAudioRecordingDevices;
        QStringList mPodcastProfileNames;
        QList<int> mPodcastBitRateDivisor;
};



#endif // UBPODCASTPREFERENCESDIALOG_H
