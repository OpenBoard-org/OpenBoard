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




#include "UBPodcastController.h"

#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDisplayManager.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBMainWindow.h"

#include "web/UBWebController.h"
#include "web/simplebrowser/webview.h"

#include "domain/UBGraphicsScene.h"

#include "UBAbstractVideoEncoder.h"

#include "podcast/youtube/UBYouTubePublisher.h"
#include "podcast/intranet/UBIntranetPodcastPublisher.h"
#include "UBPodcastRecordingPalette.h"




#ifdef Q_OS_WIN
    #include "windowsmedia/UBWindowsMediaVideoEncoder.h"
    #include "windowsmedia/UBWaveRecorder.h"
#elif defined(Q_OS_OSX)
    #include "ffmpeg/UBFFmpegVideoEncoder.h"
    #include "ffmpeg/UBMicrophoneInput.h"
#elif defined(Q_OS_LINUX)
    #include "ffmpeg/UBFFmpegVideoEncoder.h"
    #include "ffmpeg/UBMicrophoneInput.h"
#endif

#include "core/memcheck.h"

UBPodcastController* UBPodcastController::sInstance = 0;

unsigned int UBPodcastController::sBackgroundColor = 0x00000000;  // BBGGRRAA


UBPodcastController::UBPodcastController(QObject* pParent)
    : QObject(pParent)
    , mVideoEncoder(0)
    , mInitialized(false)
    , mEmptyChapter(true)
    , mVideoFramesPerSecondAtStart(10)
    , mVideoFrameSizeAtStart(1024, 768)
    , mVideoBitsPerSecondAtStart(1700000)
    , mSourceWidget(0)
    , mIsDesktopMode(false)
    , mSourceScene(0)
    , mScreenGrabingTimerEventID(0)
    , mRecordingProgressTimerEventID(0)
    , mRecordingPalette(0)
    , mRecordingState(Stopped)
    , mApplicationIsClosing(false)
    , mRecordingTimestampOffset(0)
    , mDefaultAudioInputDeviceAction(0)
    , mNoAudioInputDeviceAction(0)
    , mSmallVideoSizeAction(0)
    , mMediumVideoSizeAction(0)
    , mFullVideoSizeAction(0)
    , mYoutubePublicationAction(0)
    , mIntranetPublicationAction(0)
{
    connect(UBApplication::applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode)),
            this, SLOT(applicationMainModeChanged(UBApplicationController::MainMode)));

    connect(UBApplication::applicationController, SIGNAL(desktopMode(bool)),
            this, SLOT(applicationDesktopMode(bool)));

    connect(UBApplication::webController, SIGNAL(activeWebPageChanged(WebView*)),
            this, SLOT(webActiveWebPageChanged(WebView*)));

    connect(UBApplication::app(), SIGNAL(lastWindowClosed()),
            this, SLOT(applicationAboutToQuit()));

}


UBPodcastController::~UBPodcastController()
{
    // NOOP
}


void UBPodcastController::applicationAboutToQuit()
{
    mApplicationIsClosing = true;

    if(mRecordingState == Recording || mRecordingState == Paused)
    {
        stop();
    }
}


void UBPodcastController::groupActionTriggered(QAction* action)
{
    Q_UNUSED(action);
    updateActionState();
}


void UBPodcastController::actionToggled(bool checked)
{
    Q_UNUSED(checked);
    updateActionState();
}


void UBPodcastController::updateActionState()
{
    if (mSmallVideoSizeAction && mSmallVideoSizeAction->isChecked())
        UBSettings::settings()->podcastVideoSize->set("Small");
    else if (mFullVideoSizeAction && mFullVideoSizeAction->isChecked())
        UBSettings::settings()->podcastVideoSize->set("Full");
    else
        UBSettings::settings()->podcastVideoSize->reset();

    UBSettings::settings()->podcastAudioRecordingDevice->reset();

    if (mDefaultAudioInputDeviceAction && mDefaultAudioInputDeviceAction->isChecked())
         UBSettings::settings()->podcastAudioRecordingDevice->set("Default");
    else if (mNoAudioInputDeviceAction && mNoAudioInputDeviceAction->isChecked())
         UBSettings::settings()->podcastAudioRecordingDevice->set("None");
    else
    {
        foreach(QAction* action, mAudioInputDevicesActions)
        {
            if (action->isChecked())
            {
                UBSettings::settings()->podcastAudioRecordingDevice->set(action->text());
                break;
            }
        }
    }

    UBSettings::settings()->podcastPublishToYoutube->set(mYoutubePublicationAction && mYoutubePublicationAction->isChecked());
    UBSettings::settings()->podcastPublishToIntranet->set(mIntranetPublicationAction && mIntranetPublicationAction->isChecked());

}

void UBPodcastController::widgetSizeChanged(const QSizeF size)
{
    qDebug() << "widgetSizeChanged to" << size << "video" << mVideoFrameSizeAtStart;
    mInitialized = false;
    mViewToVideoTransform.reset();

    QSizeF videoFrameSize(mVideoFrameSizeAtStart);
    qreal scaleHorizontal = videoFrameSize.width() / size.width();
    qreal scaleVertical = videoFrameSize.height() / size.height();
    qreal scale = qMin(scaleHorizontal, scaleVertical);

    mViewToVideoTransform.scale(scale, scale);

    QSizeF scaledWidgetSize = size * scale;
    int offsetX = (videoFrameSize.width() - scaledWidgetSize.width()) / 2;
    int offsetY = (videoFrameSize.height() - scaledWidgetSize.height()) / 2;

    mViewToVideoTransform.translate(offsetX / scale, offsetY / scale);
}


void UBPodcastController::setSourceWidget(QWidget* pWidget)
{
    if (mSourceWidget != pWidget)
    {
        // cleanup timer and event filter
        if (mScreenGrabingTimerEventID)
        {
            killTimer(mScreenGrabingTimerEventID);
            mScreenGrabingTimerEventID = 0;
        }

        if (mSourceWidget)
        {
            mSourceWidget->removeEventFilter(this);
        }

        // setup new source widget
        mSourceWidget = pWidget;
        mInitialized = false;
        mViewToVideoTransform.reset();
        mLatestCapture.fill(sBackgroundColor);

        if (mSourceWidget)
        {
            widgetSizeChanged(mSourceWidget->size());

            UBBoardView *bv = qobject_cast<UBBoardView *>(mSourceWidget);

            if (bv && !mIsDesktopMode)
            {
                connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(activeSceneChanged()));
                connect(UBApplication::boardController, SIGNAL(backgroundChanged()), this, SLOT(sceneBackgroundChanged()));
                connect(UBApplication::boardController, SIGNAL(controlViewportChanged()), this, SLOT(activeSceneChanged()));

                activeSceneChanged();
            }
            else
            {
                disconnect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(activeSceneChanged()));
                disconnect(UBApplication::boardController, SIGNAL(backgroundChanged()), this, SLOT(sceneBackgroundChanged()));
                disconnect(UBApplication::boardController, SIGNAL(controlViewportChanged()), this, SLOT(activeSceneChanged()));

                mSourceScene = nullptr;

                startNextChapter();

                if (mIsDesktopMode || UBApplication::applicationController->displayMode() == UBApplicationController::Internet)
                {
                    mScreenGrabingTimerEventID  = startTimer(1000 / mVideoFramesPerSecondAtStart);
                }
            }

            mSourceWidget->installEventFilter(this);
        }
    }
}


UBPodcastController* UBPodcastController::instance()
{
    if(!sInstance)
        sInstance = new UBPodcastController(UBApplication::staticMemoryCleaner);

    return sInstance;
}


void UBPodcastController::start()
{
    if (mRecordingState == Stopped)
    {
        mInitialized = false;

        QSize recommendedSize(1024, 768);

        int fullBitRate = UBSettings::settings()->podcastWindowsMediaBitsPerSecond->get().toInt();

        if (mSmallVideoSizeAction && mSmallVideoSizeAction->isChecked())
        {
            recommendedSize = QSize(640, 480);
            mVideoBitsPerSecondAtStart = fullBitRate / 4;
        }
        else if (mMediumVideoSizeAction && mMediumVideoSizeAction->isChecked())
        {
            recommendedSize = QSize(1024, 768);
            mVideoBitsPerSecondAtStart = fullBitRate / 2;
        }
        else if (mFullVideoSizeAction && mFullVideoSizeAction->isChecked())
        {
            recommendedSize = UBApplication::boardController->controlView()->size();
            mVideoBitsPerSecondAtStart = fullBitRate;
        }

        QSize scaledboardSize = UBApplication::boardController->controlView()->size();
        scaledboardSize.scale(recommendedSize, Qt::KeepAspectRatio);

        // Video width/height should be a multiple of 4

        int width = scaledboardSize.width();
        int height = scaledboardSize.height();

        if (width % 4 != 0)
                width = ((width / 4) * 4);

        if (height % 4 != 0)
                height = ((height / 4) * 4);

        mVideoFrameSizeAtStart = QSize(width, height);

        applicationMainModeChanged(UBApplication::applicationController->displayMode());

#ifdef Q_OS_WIN
        mVideoEncoder = new UBWindowsMediaVideoEncoder(this);  //deleted on stop
#elif defined(Q_OS_OSX)
        mVideoEncoder = new UBFFmpegVideoEncoder(this);
#elif defined(Q_OS_LINUX)
        mVideoEncoder = new UBFFmpegVideoEncoder(this);
#endif

        if (mVideoEncoder)
        {
            connect(mVideoEncoder, SIGNAL(encodingStatus(const QString&)), this, SLOT(encodingStatus(const QString&)));
            connect(mVideoEncoder, SIGNAL(encodingFinished(bool)), this, SLOT(encodingFinished(bool)));

            if(mRecordingPalette)
            {
                connect(mVideoEncoder, SIGNAL(audioLevelChanged(quint8))
                        , mRecordingPalette, SLOT(audioLevelChanged(quint8)));
            }

            mVideoEncoder->setRecordAudio(!mNoAudioInputDeviceAction->isChecked());

            QString recordingDevice = "";

            if (!mNoAudioInputDeviceAction->isChecked() && !mDefaultAudioInputDeviceAction->isChecked())
            {
                foreach(QAction* audioDevice, mAudioInputDevicesActions)
                {
                    if (audioDevice->isChecked())
                    {
                        recordingDevice = audioDevice->text();
                        break;
                    }
                }
            }

            mVideoEncoder->setAudioRecordingDevice(recordingDevice);

            mVideoEncoder->setFramesPerSecond(mVideoFramesPerSecondAtStart);
            mVideoEncoder->setVideoSize(mVideoFrameSizeAtStart);
            mVideoEncoder->setVideoBitsPerSecond(mVideoBitsPerSecondAtStart);

            mPartNumber = 0;

            mPodcastRecordingPath = UBSettings::settings()->userPodcastRecordingDirectory();

            qDebug() << "mPodcastRecordingPath: " << mPodcastRecordingPath;

            QString videoFileName;

            if (mIntranetPublicationAction && mIntranetPublicationAction->isChecked())
            {
                videoFileName = mPodcastRecordingPath + "/" + "Podcast-"
                        + QDateTime::currentDateTime().toString("yyyyMMddhhmmss")
                        + "-" + UBPlatformUtils::computerName() + "." + mVideoEncoder->videoFileExtension();
            }
            else
            {
                videoFileName = mPodcastRecordingPath + "/" + tr("OpenBoard Cast") + "." + mVideoEncoder->videoFileExtension();
            }

            videoFileName = UBFileSystemUtils::nextAvailableFileName(videoFileName, " ");

            mVideoEncoder->setVideoFileName(videoFileName);

            mLatestCapture = QImage(mVideoFrameSizeAtStart, QImage::Format_RGB32); //0xffRRGGBB

            mRecordStartTime = QTime::currentTime();

            mRecordingProgressTimerEventID = startTimer(100);

            if(mVideoEncoder->start())
            {
                setRecordingState(Recording);

                if (mSourceScene)
                {
                    processScenePaintEvent();
                }
                else
                {
                    processScreenGrabingTimerEvent();
                }
            }
            else
            {
                UBApplication::showMessage(tr("Failed to start encoder ..."), false);
            }
        }
        else
        {
            UBApplication::showMessage(tr("No Podcast encoder available ..."), false);
        }
    }
}

void UBPodcastController::pause()
{
    if (mVideoEncoder && mRecordingState == Recording && mVideoEncoder->canPause())
    {
        sendLatestPixmapToEncoder();

        mTimeAtPaused = QTime::currentTime();

        if (mVideoEncoder->pause())
        {
            setRecordingState(Paused);
        }
    }
}


void UBPodcastController::unpause()
{
    if (mVideoEncoder && mRecordingState == Paused && mVideoEncoder->canPause())
    {
        if (mVideoEncoder->unpause())
        {
             mRecordingTimestampOffset += mTimeAtPaused.msecsTo(QTime::currentTime());
             sendLatestPixmapToEncoder();

             setRecordingState(Recording);
        }
    }
}


void UBPodcastController::stop()
{
    if ((mRecordingState == Recording || mRecordingState == Paused) && mVideoEncoder)
    {
        if (mScreenGrabingTimerEventID != 0)
        {
            killTimer(mScreenGrabingTimerEventID);
            mScreenGrabingTimerEventID = 0;
        }

        if (mRecordingProgressTimerEventID != 0)
            killTimer(mRecordingProgressTimerEventID);

        sendLatestPixmapToEncoder();

        setRecordingState(Stopping);

        mVideoEncoder->stop();
    }

    mSourceScene = nullptr;
}


bool UBPodcastController::eventFilter(QObject *obj, QEvent *event)
{
    if (mRecordingState == Recording && event->type() == QEvent::Resize)
    {
        QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
        widgetSizeChanged(resizeEvent->size());
    }

    return QObject::eventFilter(obj, event);
}


void UBPodcastController::activeSceneChanged()
{
    if (mSourceScene)
    {
        disconnect(mSourceScene.get(), SIGNAL(changed(const QList<QRectF>&)),
                this, SLOT(sceneChanged(const QList<QRectF> &)));
    }

    mSourceScene = UBApplication::boardController->activeScene();

    connect(mSourceScene.get(), SIGNAL(changed(const QList<QRectF>&)),
        this, SLOT(sceneChanged(const QList<QRectF> &)));

    mInitialized = false;

    startNextChapter();

    UBBoardView *bv = qobject_cast<UBBoardView*>(mSourceWidget);
    if (bv)
    {
        QRectF viewportRect = bv->mapToScene(bv->geometry()).boundingRect();
        mSceneRepaintRectQueue.enqueue(viewportRect);
    }

    processScenePaintEvent();
}

void UBPodcastController::sceneBackgroundChanged()
{
    UBBoardView *bv = qobject_cast<UBBoardView*>(mSourceWidget);

    if (bv)
    {
        mInitialized = false;
    }

    processScenePaintEvent();
}


long UBPodcastController::elapsedRecordingMs()
{
    QTime now = QTime::currentTime();
    long msFromStart = mRecordStartTime.msecsTo(now);

    return msFromStart - mRecordingTimestampOffset;
}


void UBPodcastController::startNextChapter()
{
    if (mVideoEncoder && !mEmptyChapter)
    {
        //punch chapter in
        ++mPartNumber;
        mVideoEncoder->newChapter(tr("Part %1").arg(mPartNumber), elapsedRecordingMs());
        mEmptyChapter = true;
        qDebug() << "Start chapter" << mPartNumber;
    }
}


void UBPodcastController::sceneChanged(const QList<QRectF> & region)
{
    if(mRecordingState != Recording)
        return;

    bool shouldRepaint = (mSceneRepaintRectQueue.length() == 0);

    UBBoardView *bv = qobject_cast<UBBoardView *>(mSourceWidget);
    if (bv)
    {
        QRectF viewportRect = bv->mapToScene(QRect(0, 0, bv->width(), bv->height())).boundingRect();
        foreach(const QRectF rect, region)
        {
            QRectF maxRect = rect.intersected(viewportRect);
            mSceneRepaintRectQueue.enqueue(maxRect);
        }

        if (shouldRepaint)
            QTimer::singleShot(1000.0 / mVideoFramesPerSecondAtStart, this, SLOT(processScenePaintEvent()));

    }
}


void UBPodcastController::processScenePaintEvent()
{
    if(mRecordingState != Recording)
        return;

    UBBoardView *bv = qobject_cast<UBBoardView *>(mSourceWidget);

    if(!bv)
        return;

    QRectF repaintRect;

    if (!mInitialized)
    {
        mSceneRepaintRectQueue.clear();
        repaintRect = bv->mapToScene(QRect(0, 0, bv->width(), bv->height())).boundingRect();

        if (bv->scene()->isDarkBackground())
            mLatestCapture.fill(Qt::black);
        else
            mLatestCapture.fill(Qt::white);

        mInitialized = true;
    }
    else
    {
        while(mSceneRepaintRectQueue.size() > 0)
        {
            repaintRect = repaintRect.united(mSceneRepaintRectQueue.dequeue());
        }
    }

    if (!repaintRect.isNull())
    {
        std::shared_ptr<UBGraphicsScene> scene = bv->scene();

        QPainter p(&mLatestCapture);

        p.setTransform(mViewToVideoTransform);
        p.setTransform(bv->viewportTransform(), true);

        p.setRenderHints(QPainter::Antialiasing);
        p.setRenderHints(QPainter::SmoothPixmapTransform);

        repaintRect.adjust(-1, -1, 1, 1);

        p.setClipRect(repaintRect);

        if (scene->isDarkBackground())
            p.fillRect(repaintRect, Qt::black);
        else
            p.fillRect(repaintRect, Qt::white);

        scene->setRenderingContext(UBGraphicsScene::Podcast);

        scene->render(&p, repaintRect, repaintRect);

        scene->setRenderingContext(UBGraphicsScene::Screen);

        sendLatestPixmapToEncoder();
    }
}


void UBPodcastController::applicationMainModeChanged(UBApplicationController::MainMode pMode)
{
    mIsDesktopMode = false;

    if (pMode == UBApplicationController::Internet)
    {
        setSourceWidget(UBApplication::webController->controlView());
    }
    else
    {
        setSourceWidget(UBApplication::boardController->controlView());
    }
}


void UBPodcastController::applicationDesktopMode(bool displayed)
{
    mIsDesktopMode = displayed;

    if (displayed)
    {
        setSourceWidget(UBApplication::displayManager->widget(ScreenRole::Desktop));
    }
    else
    {
        applicationMainModeChanged(UBApplication::applicationController->displayMode());
    }
}


void UBPodcastController::webActiveWebPageChanged(WebView* pWebView)
{
    if(UBApplication::applicationController->displayMode() == UBApplicationController::Internet)
    {
        setSourceWidget(pWebView);
    }
}


void UBPodcastController::encodingStatus(const QString& pStatus)
{
    UBApplication::showMessage(pStatus, true);
}


void UBPodcastController::encodingFinished(bool ok)
{
    if (mVideoEncoder)
    {
        if (ok)
        {
            if (!mApplicationIsClosing)
            {
                QString location;

                if (mPodcastRecordingPath == QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                    location = tr("on your desktop ...");
                else
                {
                    QDir dir(mPodcastRecordingPath);
                    location = tr("in folder %1").arg(mPodcastRecordingPath);
                }

                UBApplication::showMessage(tr("Podcast created %1").arg(location), false);

                if (mIntranetPublicationAction && mIntranetPublicationAction->isChecked())
                {
                    UBIntranetPodcastPublisher* intranet = new UBIntranetPodcastPublisher(this); // Self destroyed
                    intranet->publishVideo(mVideoEncoder->videoFileName(), elapsedRecordingMs());
                }

                if (mYoutubePublicationAction && mYoutubePublicationAction->isChecked())
                {
                    UBYouTubePublisher* youTube = new UBYouTubePublisher(this); // Self destroyed
                    youTube->uploadVideo(mVideoEncoder->videoFileName());
                }
            }
        }
        else
        {
            qWarning() << mVideoEncoder->lastErrorMessage();

            UBApplication::showMessage(tr("Podcast recording error (%1)").arg(mVideoEncoder->lastErrorMessage()), false);
        }

        mVideoEncoder->deleteLater();

        setRecordingState(Stopped);
    }
}


void UBPodcastController::sendLatestPixmapToEncoder()
{
    if (mVideoEncoder)
        mVideoEncoder->newPixmap(mLatestCapture, elapsedRecordingMs());

    mEmptyChapter = false;
}

void UBPodcastController::timerEvent(QTimerEvent *event)
{
    if (mRecordingState == Recording)
    {
        if (event->timerId() == mScreenGrabingTimerEventID)
        {
            processScreenGrabingTimerEvent();
        }
        else if (event->timerId() == mRecordingProgressTimerEventID)
        {
            emit recordingProgressChanged(elapsedRecordingMs());
        }
    }
}

void UBPodcastController::processScreenGrabingTimerEvent()
{
    QPixmap widgetContent;

    if (mIsDesktopMode)
    {
        // TODO implement screencast based on screencast portal
        UBApplication::displayManager->grab(ScreenRole::Control, [this](QPixmap pixmap){
            encodeWidgetContent(pixmap);
        });
    }
    else
    {
        // render web view
        widgetContent = QPixmap(mSourceWidget->size());
        QPainter p(&widgetContent);
        mSourceWidget->render(&p);
        encodeWidgetContent(widgetContent);
    }
}

void UBPodcastController::encodeWidgetContent(QPixmap pixmap)
{
    QPainter p(&mLatestCapture);

    if (!mInitialized)
    {
        mLatestCapture.fill(sBackgroundColor);
        mInitialized = true;
    }

    QRectF targetRect = mViewToVideoTransform.mapRect(QRectF(0, 0, pixmap.width(), pixmap.height()));

    p.setRenderHints(QPainter::Antialiasing);
    p.setRenderHints(QPainter::SmoothPixmapTransform);
    p.drawPixmap(targetRect.left(), targetRect.top(), pixmap.scaled(targetRect.width(), targetRect.height(),  Qt::KeepAspectRatio, Qt::SmoothTransformation));

    sendLatestPixmapToEncoder();
}


QStringList UBPodcastController::audioRecordingDevices()
{
    QStringList devices;

#ifdef Q_OS_WIN
    devices = UBWaveRecorder::waveInDevices();
#elif defined(Q_OS_OSX)
    devices = UBMicrophoneInput::availableDevicesNames();
#elif defined(Q_OS_LINUX)
    devices = UBMicrophoneInput::availableDevicesNames();
#endif

    return devices;
}


void UBPodcastController::recordToggled(bool record)
{
    if ((mRecordingState == Stopped) && record)
        start();
    else
        stop();
}

void UBPodcastController::pauseToggled(bool paused)
{
    if ((mRecordingState == Recording) && paused)
        pause();
    else
        unpause();
}


void UBPodcastController::toggleRecordingPalette(bool visible)
{
    if(!mRecordingPalette)
    {
        mRecordingPalette = new UBPodcastRecordingPalette(UBApplication::mainWindow);

        mRecordingPalette->adjustSizeAndPosition();
        mRecordingPalette->setCustomPosition(true);

        int left = UBApplication::boardController->controlView()->width() * 0.75
                   - mRecordingPalette->width() / 2;

        int top = UBApplication::boardController->controlView()->height()
                   - mRecordingPalette->height() - UBSettings::boardMargin;

        QPoint controlViewPoint(left, top);
        QPoint mainWindowsPoint = UBApplication::boardController->controlView()->mapTo(UBApplication::mainWindow, controlViewPoint);

        mRecordingPalette->move(mainWindowsPoint);

        connect(UBApplication::mainWindow->actionPodcastRecord, SIGNAL(triggered(bool))
             , this, SLOT(recordToggled(bool)));

        connect(UBApplication::mainWindow->actionPodcastPause, SIGNAL(toggled(bool))
             , this, SLOT(pauseToggled(bool)));

        connect(this, SIGNAL(recordingStateChanged(UBPodcastController::RecordingState))
                , mRecordingPalette, SLOT(recordingStateChanged(UBPodcastController::RecordingState)));
        connect(this, SIGNAL(recordingProgressChanged(qint64))
                , mRecordingPalette, SLOT(recordingProgressChanged(qint64)));
    }

    mRecordingPalette->setVisible(visible);
}


void UBPodcastController::setRecordingState(RecordingState pRecordingState)
{
    if(mRecordingState != pRecordingState)
    {
        mRecordingState = pRecordingState;
        emit recordingStateChanged(mRecordingState);
    }
}


QList<QAction*> UBPodcastController::audioRecordingDevicesActions()
{
    if (mAudioInputDevicesActions.length() == 0)
    {
        QString settingsDevice = UBSettings::settings()->podcastAudioRecordingDevice->get().toString();

        mDefaultAudioInputDeviceAction = new QAction(tr("Default Audio Input"), this);
        QAction *checkedAction = mDefaultAudioInputDeviceAction;

        mNoAudioInputDeviceAction = new QAction(tr("No Audio Recording"), this);

        if (settingsDevice == "None")
            checkedAction = mNoAudioInputDeviceAction;

        mAudioInputDevicesActions << mNoAudioInputDeviceAction;
        mAudioInputDevicesActions << mDefaultAudioInputDeviceAction;

        foreach(QString audioDevice, audioRecordingDevices())
        {
            QAction* act = new QAction(audioDevice, this);
            act->setCheckable(true);
            mAudioInputDevicesActions << act;
            if (settingsDevice == audioDevice)
                checkedAction = act;
        }

        QActionGroup* audioInputActionGroup = new QActionGroup(this);
        audioInputActionGroup->setExclusive(true);

        foreach(QAction* action, mAudioInputDevicesActions)
        {
            audioInputActionGroup->addAction(action);
            action->setCheckable(true);
        }
        checkedAction->setChecked(true);

        connect(audioInputActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(groupActionTriggered(QAction*)));
    }

    return mAudioInputDevicesActions;

}


QList<QAction*> UBPodcastController::videoSizeActions()
{
    if (mVideoSizesActions.length() == 0)
    {
        mSmallVideoSizeAction = new QAction(tr("Small"), this);
        mMediumVideoSizeAction = new QAction(tr("Medium"), this);
        mFullVideoSizeAction = new QAction(tr("Full"), this);

        mVideoSizesActions << mSmallVideoSizeAction;
        mVideoSizesActions << mMediumVideoSizeAction;
        mVideoSizesActions << mFullVideoSizeAction;

        QActionGroup* videoSizeActionGroup = new QActionGroup(this);
        videoSizeActionGroup->setExclusive(true);

        foreach(QAction* videoSizeAction, mVideoSizesActions)
        {
            videoSizeAction->setCheckable(true);
            videoSizeActionGroup->addAction(videoSizeAction);
        }

        QString videoSize = UBSettings::settings()->podcastVideoSize->get().toString();

        if (videoSize == "Small")
            mSmallVideoSizeAction->setChecked(true);
        else if (videoSize == "Full")
            mFullVideoSizeAction->setChecked(true);
        else
            mMediumVideoSizeAction->setChecked(true);

        connect(videoSizeActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(groupActionTriggered(QAction*)));
    }

    return mVideoSizesActions;
}


QList<QAction*> UBPodcastController::podcastPublicationActions()
{
    if (mPodcastPublicationActions.length() == 0)
    {
        mIntranetPublicationAction = new QAction(tr("Publish to Intranet"), this);

        mIntranetPublicationAction->setCheckable(true);
        mIntranetPublicationAction->setChecked(UBSettings::settings()->podcastPublishToIntranet->get().toBool());

        mPodcastPublicationActions << mIntranetPublicationAction;

        mYoutubePublicationAction = new QAction(tr("Publish to Youtube"), this);
        mYoutubePublicationAction->setCheckable(true);
        mYoutubePublicationAction->setChecked(UBSettings::settings()->podcastPublishToYoutube->get().toBool());

        mPodcastPublicationActions << mYoutubePublicationAction;

        foreach(QAction* publicationAction, mPodcastPublicationActions)
        {
            connect(publicationAction, SIGNAL(toggled(bool)), this, SLOT(actionToggled(bool)));
        }
    }

    return mPodcastPublicationActions;
}



