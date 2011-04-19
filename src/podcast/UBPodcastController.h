/*
 * UBPodcastController.h
 *
 *  Created on: 3 sept. 2009
 *      Author: Luc
 */

#ifndef UBPODCASTCONTROLLER_H_
#define UBPODCASTCONTROLLER_H_

#include <QtGui>

#include "UBAbstractVideoEncoder.h"

#include "core/UBApplicationController.h"

class UBGraphicsScene;
class WBWebView;
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

        void processWidgetPaintEvent();

        void processScenePaintEvent();

        void sceneChanged(const QList<QRectF> & region);
        void sceneBackgroundChanged();

        void activeSceneChanged();

        void applicationMainModeChanged(UBApplicationController::MainMode pMode);

        void applicationDesktopMode(bool displayed);

        void webActiveWebPageChanged(WBWebView* pWebView);

        void encodingStatus(const QString& pStatus);

        void encodingFinished(bool ok);

        void applicationAboutToQuit();

        void groupActionTriggered(QAction*);
        void actionToggled(bool);

        void updateActionState();

    private:

        void setRecordingState(RecordingState pRecordingState);

        void sendLatestPixmapToEncoder();

        long elapsedRecordingMs();

        static UBPodcastController* sInstance;

        QPointer<UBAbstractVideoEncoder> mVideoEncoder;

        QTime mRecordStartTime;

        bool mIsGrabbing;

        QQueue<QRect> mWidgetRepaintRectQueue;
        QQueue<QRectF> mSceneRepaintRectQueue;

        bool mInitialized;

        QImage mLatestCapture;

        int mVideoFramesPerSecondAtStart;
        QSize mVideoFrameSizeAtStart;
        long mVideoBitsPerSecondAtStart;

        static unsigned int sBackgroundColor;

        QWidget* mSourceWidget;

        UBGraphicsScene* mSourceScene;

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
