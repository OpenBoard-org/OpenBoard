/*
 * UBWindowsMediaFile.h
 *
 *  Created on: 8 sept. 2009
 *      Author: Luc
 */

#ifndef UBWINDOWSMEDIAFILE_H_
#define UBWINDOWSMEDIAFILE_H_

#include <QtCore>

#include <wmsdk.h>


class UBWindowsMediaFile : public QObject
{

    Q_OBJECT;

    public:
        UBWindowsMediaFile(QObject * pParent = 0);
        virtual ~UBWindowsMediaFile();

        bool init(const QString& videoFileName, const QString& profileData
                , int pFramesPerSecond, int pixelWidth, int pixelHeight, int bitsPerPixel);

        bool close();

        bool appendVideoFrame(const QImage& pPix, long msTimeStamp);

        void startNewChapter(const QString& pLabel, long timestamp);

        QString lastErrorMessage() const
        {
            return mLastErrorMessage;
        }

    public slots:

       void appendAudioBuffer(WAVEHDR*, long);

    private:

        bool initVideoStream(int pixelWidth, int pixelHeight, int bitsPerPixel);

        void setLastErrorMessage(const QString& error);

        void releaseWMObjects();

        QWORD msToSampleTime(long ms)
        {
            QWORD qwordMs = ms;
            return (qwordMs * 10000);
        }

        IWMProfile *mWMProfile;
        IWMWriter *mWMWriter;
        IWMWriterPushSink *mPushSink;

        IWMInputMediaProps *mWMInputVideoProps;
        IWMInputMediaProps *mWMInputAudioProps;
        IWMProfileManager *mWMProfileManager;

        HDC mWMhDC;
        DWORD mVideoInputIndex;
        DWORD mAudioInputIndex;
        DWORD mFramesPerSecond;

        QString mLastErrorMessage;

        struct MarkerInfo
        {
            QString label;
            long timestamp;
        };

        QList<MarkerInfo> mChapters;

        QString mVideoFileName;

};

#endif /* UBWINDOWSMEDIAFILE_H_ */
