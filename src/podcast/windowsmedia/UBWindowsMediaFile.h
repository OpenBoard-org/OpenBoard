/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
