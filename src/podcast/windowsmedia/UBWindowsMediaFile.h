/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBWINDOWSMEDIAFILE_H_
#define UBWINDOWSMEDIAFILE_H_

#include <QtCore>

#ifndef inteface
#define interface struct
#endif
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
