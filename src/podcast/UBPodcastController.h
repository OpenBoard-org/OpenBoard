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




#ifndef UBPODCASTCONTROLLER_H_
#define UBPODCASTCONTROLLER_H_

#include <QtGui>

#include "UBAbstractVideoEncoder.h"

#include "core/UBApplicationController.h"

class UBGraphicsScene;
class WebView;
class UBPodcastRecordingPalette;


class UBPodcastController : public QObject
{
    Q_OBJECT;

    private:
        UBPodcastController(QObject* pParent = 0);
        virtual ~UBPodcastController();

    public:
        static UBPodcastController* instance();

        virtual bool eventFilter(QObject *obj, QEvent *event);

        virtual QStringList audioRecordingDevices();

        QList<QAction*> audioRecordingDevicesActions();
        QList<QAction*> videoSizeActions();

        QList<QAction*> podcastPublicationActions();

        enum RecordingState
        {
            Stopped = 0, Recording, Paused, Stopping
        };

    signals:

        void recordingStateChanged(UBPodcastController::RecordingState);

        void recordingProgressChanged(qint64 ms);

    public slots:

        void start();
        void stop();

        void pause();
        void unpause();

        void toggleRecordingPalette(bool visible);

        void recordToggled(bool record);
        void pauseToggled(bool pause);

    protected:

        virtual void setSourceWidget(QWidget* pWidget);

        virtual void timerEvent(QTimerEvent *event);

    private slots:

        void processScreenGrabingTimerEvent();
        void encodeWidgetContent(QPixmap pixmap);

        void processScenePaintEvent();

        void sceneChanged(const QList<QRectF> & region);
        void sceneBackgroundChanged();

        void activeSceneChanged();

        void applicationMainModeChanged(UBApplicationController::MainMode pMode);

        void applicationDesktopMode(bool displayed);

        void webActiveWebPageChanged(WebView* pWebView);

        void encodingStatus(const QString& pStatus);

        void encodingFinished(bool ok);

        void applicationAboutToQuit();

        void groupActionTriggered(QAction*);
        void actionToggled(bool);

        void updateActionState();

    private:
        void widgetSizeChanged(const QSizeF size);

        void setRecordingState(RecordingState pRecordingState);

        void sendLatestPixmapToEncoder();

        long elapsedRecordingMs();

        static UBPodcastController* sInstance;

        QPointer<UBAbstractVideoEncoder> mVideoEncoder;

        QTime mRecordStartTime;

        QQueue<QRectF> mSceneRepaintRectQueue;

        bool mInitialized;
        bool mEmptyChapter;

        QImage mLatestCapture;

        int mVideoFramesPerSecondAtStart;
        QSize mVideoFrameSizeAtStart;
        long mVideoBitsPerSecondAtStart;

        static unsigned int sBackgroundColor;

        QWidget* mSourceWidget;
        bool mIsDesktopMode;

        std::shared_ptr<UBGraphicsScene> mSourceScene;

        QTransform mViewToVideoTransform;

        int mScreenGrabingTimerEventID;
        int mRecordingProgressTimerEventID;

        int mPartNumber;

        void startNextChapter();

        UBPodcastRecordingPalette *mRecordingPalette;

        RecordingState mRecordingState;

        bool mApplicationIsClosing;

        QTime mTimeAtPaused;
        long mRecordingTimestampOffset;

        QAction *mDefaultAudioInputDeviceAction;
        QAction *mNoAudioInputDeviceAction;

        QList<QAction*> mAudioInputDevicesActions;
        QList<QAction*> mVideoSizesActions;

        QAction* mSmallVideoSizeAction;
        QAction* mMediumVideoSizeAction;
        QAction* mFullVideoSizeAction;

        QList<QAction*> mPodcastPublicationActions;

        QAction *mYoutubePublicationAction;
        QAction *mIntranetPublicationAction;

        QString mPodcastRecordingPath;

};

#endif /* UBPODCASTCONTROLLER_H_ */
