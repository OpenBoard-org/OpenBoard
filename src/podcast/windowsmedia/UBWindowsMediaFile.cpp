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




#include "UBWindowsMediaFile.h"

#include <QtGui>

#include "core/UBApplication.h"

#include "core/memcheck.h"

UBWindowsMediaFile::UBWindowsMediaFile(QObject * pParent)
    : QObject(pParent)
    , mWMhDC(0)
    , mWMWriter(0)
    , mPushSink(0)
    , mVideoInputIndex(0)
    , mWMProfile(0)
    , mWMInputVideoProps(0)
    , mWMInputAudioProps(0)
    , mWMProfileManager(0)
    , mFramesPerSecond(-1)
{
    // NOOP
}


bool UBWindowsMediaFile::init(const QString& videoFileName, const QString& profileData, int pFramesPerSecond
                , int pixelWidth, int pixelHeight, int bitsPerPixel)
{
    mFramesPerSecond = pFramesPerSecond;
    mVideoFileName = videoFileName;

    CoInitialize(0);

    if (FAILED(WMCreateProfileManager(&mWMProfileManager)))
    {
        setLastErrorMessage("Unable to create a WMProfileManager");
        close();
        return false;
    }

    IWMProfileManager2 *wmProfileManager = 0;

    if (FAILED(mWMProfileManager->QueryInterface(IID_IWMProfileManager2, (void**) &wmProfileManager)))
    {
        setLastErrorMessage("Unable to query the WMProfileManager for interface WMProfileManager2");
        close();
        return false;
    }

    HRESULT hr = wmProfileManager->SetSystemProfileVersion(WMT_VER_9_0);
    wmProfileManager->Release();

    if (FAILED(hr))
    {
        setLastErrorMessage("Unable to set WMProfileManager SystemProfileVersion");
        close();
        return false;
    }

    if (FAILED(mWMProfileManager->LoadProfileByData((LPCTSTR) profileData.utf16(), &mWMProfile)))
    {
        setLastErrorMessage("Unable to load WMProfileManager custom profile");
        close();
        return false;
    }

    DWORD streamCount = -1;

    if (FAILED(mWMProfile->GetStreamCount(&streamCount)))
    {
        setLastErrorMessage("Unable to read mWMProfile stream count");
        close();
        return false;
    }

    if (FAILED(WMCreateWriter(NULL, &mWMWriter)))
    {
        setLastErrorMessage("Unable to create WMMediaWriter Object");
        close();
        return false;
    }

    if (FAILED(mWMWriter->SetProfile(mWMProfile)))
    {
        setLastErrorMessage("Unable to set WMWriter system profile");
        close();
        return false;
    }

    DWORD mediaInputCount = 0;

    if (FAILED(mWMWriter->GetInputCount(&mediaInputCount)))
    {
        setLastErrorMessage("Unable to get input count for profile");
        close();
        return false;
    }

    for (DWORD i = 0; i < mediaInputCount; i++)
    {
        IWMInputMediaProps* wmInoutMediaProps = 0;

        if (FAILED(mWMWriter->GetInputProps(i, &wmInoutMediaProps)))
        {
            setLastErrorMessage("Unable to get WMWriter input properties");
            close();
            return false;
        }

        GUID guidInputType;

        if (FAILED(wmInoutMediaProps->GetType(&guidInputType)))
        {
            setLastErrorMessage("Unable to get WMWriter input property type");
            close();
            return false;
        }

        if (guidInputType == WMMEDIATYPE_Video)
        {
            mWMInputVideoProps = wmInoutMediaProps;
            mVideoInputIndex = i;
        }
        else if (guidInputType == WMMEDIATYPE_Audio)
        {
            mWMInputAudioProps = wmInoutMediaProps;
            mAudioInputIndex = i;
        }
        else
        {
            wmInoutMediaProps->Release();
            wmInoutMediaProps = 0;
        }
    }

    if (mWMInputVideoProps == 0)
    {
        setLastErrorMessage("Profile does not accept video input");
        close();
        return false;
    }

    if (mWMInputAudioProps == 0)
    {
        setLastErrorMessage("Profile does not accept audio input");
        close();
        return false;
    }

    if (FAILED(mWMWriter->SetOutputFilename((LPCTSTR) videoFileName.utf16())))
    {
        setLastErrorMessage("Unable to set the output filename");
        close();
        return false;
    }

    if(!initVideoStream(pixelWidth, pixelHeight, bitsPerPixel))
    {
        close();
        return false;
    }

    if (FAILED(mWMWriter->BeginWriting()))
    {
        setLastErrorMessage("Unable to initialize video frame writing");
        return false;
    }

    return true;
}


bool UBWindowsMediaFile::close()
{
    bool result = true;

    if (mWMWriter)
    {
        if (FAILED(mWMWriter->EndWriting()))
        {
            result = false;
        }
    }

    if (mPushSink)
    {
        mPushSink->Disconnect();
        mPushSink->EndSession();
        mPushSink->Release();
    }

    if(mChapters.length() > 0)
    {
        IWMMetadataEditor *editor = 0;
        IWMHeaderInfo     *headerInfo = 0;

        WMCreateEditor(&editor);
        editor->Open((LPCTSTR) mVideoFileName.utf16());
        editor->QueryInterface(IID_IWMHeaderInfo, (void**)&headerInfo);

        foreach(MarkerInfo mi, mChapters)
        {
            headerInfo->AddMarker((LPWSTR)mi.label.utf16(),  msToSampleTime(mi.timestamp));
        }

        editor->Flush();
        editor->Close();
        headerInfo->Release();
        editor->Release();
    }

    releaseWMObjects();

    CoUninitialize();

    return result;
}


UBWindowsMediaFile::~UBWindowsMediaFile()
{
    // NOOP
}


void UBWindowsMediaFile::releaseWMObjects()
{

    if (mWMInputVideoProps)
    {
        mWMInputVideoProps->Release();
        mWMInputVideoProps = 0;
    }

    if (mWMInputAudioProps)
    {
        mWMInputAudioProps->Release();
        mWMInputAudioProps = 0;
    }

    if (mWMWriter)
    {
        mWMWriter->Release();
        mWMWriter = 0;
    }

    if (mWMProfile)
    {
        mWMProfile->Release();
        mWMProfile = 0;
    }

    if (mWMProfileManager)
    {
        mWMProfileManager->Release();
        mWMProfileManager = 0;
    }

    if (mWMhDC)
    {
        DeleteDC(mWMhDC);
        mWMhDC = 0;
    }
}


bool UBWindowsMediaFile::initVideoStream(int nFrameWidth, int nFrameHeight, int nBitsPerPixel)
{
    BITMAPINFO bmpInfo;

    mWMhDC = CreateCompatibleDC(NULL);

    if (mWMhDC == NULL)
    {
        setLastErrorMessage("Unable to create a device context");
        return false;
    }

    ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biBitCount = nBitsPerPixel;
    bmpInfo.bmiHeader.biWidth = nFrameWidth;
    bmpInfo.bmiHeader.biHeight = -nFrameHeight;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biSizeImage = nFrameWidth * nFrameHeight * nBitsPerPixel / 8;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    WMVIDEOINFOHEADER videoInfo;
    videoInfo.rcSource.left = 0;
    videoInfo.rcSource.top = 0;
    videoInfo.rcSource.right = nFrameWidth;
    videoInfo.rcSource.bottom = nFrameHeight;
    videoInfo.rcTarget.left = 0;
    videoInfo.rcTarget.top = 0;
    videoInfo.rcTarget.right = nFrameWidth;
    videoInfo.rcTarget.bottom = nFrameHeight;
    videoInfo.dwBitRate = (nFrameWidth * nFrameHeight * bmpInfo.bmiHeader.biBitCount * mFramesPerSecond);

    videoInfo.dwBitErrorRate = 0;
    videoInfo.AvgTimePerFrame = 10000000 / mFramesPerSecond;
    memcpy(&(videoInfo.bmiHeader), &bmpInfo.bmiHeader, sizeof(BITMAPINFOHEADER));

    WM_MEDIA_TYPE mt;
    mt.majortype = WMMEDIATYPE_Video;

    if (bmpInfo.bmiHeader.biCompression == BI_RGB)
    {
        if (bmpInfo.bmiHeader.biBitCount == 32)
        {
            mt.subtype = WMMEDIASUBTYPE_RGB32;
        }
        else if (bmpInfo.bmiHeader.biBitCount == 24)
        {
            mt.subtype = WMMEDIASUBTYPE_RGB24;
        }
        else if (bmpInfo.bmiHeader.biBitCount == 16)
        {
            mt.subtype = WMMEDIASUBTYPE_RGB555;
        }
        else if (bmpInfo.bmiHeader.biBitCount == 8)
        {
            mt.subtype = WMMEDIASUBTYPE_RGB8;
        }
        else
        {
            mt.subtype = GUID_NULL;
        }
    }

    mt.bFixedSizeSamples = false;
    mt.bTemporalCompression = false;
    mt.lSampleSize = 0;
    mt.formattype = WMFORMAT_VideoInfo;
    mt.pUnk = NULL;
    mt.cbFormat = sizeof(WMVIDEOINFOHEADER);
    mt.pbFormat = (BYTE*) &videoInfo;

    if (FAILED(mWMInputVideoProps->SetMediaType(&mt)))
    {
        setLastErrorMessage("Unable to set the  video media type");
        return false;
    }

    if (FAILED(mWMWriter->SetInputProps(mVideoInputIndex, mWMInputVideoProps)))
    {
        setLastErrorMessage("Unable to set the input properties for WMMediaWriter");
        return false;
    }

    return true;
}


bool UBWindowsMediaFile::appendVideoFrame(const QImage& pImage, long mstimestamp)
{
    Q_ASSERT(pImage.format() == QImage::Format_RGB32);

    INSSBuffer *sampleBuffer = 0;

    if (FAILED(mWMWriter->AllocateSample(pImage.byteCount(), &sampleBuffer)))
    {
        setLastErrorMessage("Unable to allocate memory for new video frame");
        return false;
    }

    BYTE *rawBuffer = 0;
    DWORD rawBufferLength = 0;

    if (FAILED(sampleBuffer->GetBufferAndLength(&rawBuffer, &rawBufferLength)))
    {
        setLastErrorMessage("Unable to lock video frame buffer");
        sampleBuffer->Release();
        return false;
    }

    const uchar * imageBuffer = pImage.bits();
    memcpy((void*) rawBuffer, imageBuffer, pImage.byteCount());

    HRESULT hrWriteSample = mWMWriter->WriteSample(mVideoInputIndex, msToSampleTime(mstimestamp), 0, sampleBuffer);

    sampleBuffer->Release();

    if (FAILED(hrWriteSample))
    {
        setLastErrorMessage("Unable to write video frame");
        return false;
    }

    mLastErrorMessage = "";

    return true;

}


void UBWindowsMediaFile::appendAudioBuffer(WAVEHDR *buffer, long mstimestamp)
{
    INSSBuffer *sampleBuffer = 0;

    if (FAILED(mWMWriter->AllocateSample(buffer->dwBytesRecorded, &sampleBuffer)))
    {
        setLastErrorMessage("Unable to allocate memory for new audio frame");
        return ;
    }

    BYTE *rawBuffer = 0;
    DWORD rawBufferLength = 0;

    if (FAILED(sampleBuffer->GetBufferAndLength(&rawBuffer, &rawBufferLength)))
    {
        setLastErrorMessage("Unable to lock audio frame buffer");
        sampleBuffer->Release();
        return;
    }

    memcpy((void*) rawBuffer, buffer->lpData, rawBufferLength);

    HRESULT hrWriteSample = mWMWriter->WriteSample(mAudioInputIndex, msToSampleTime(mstimestamp), 0, sampleBuffer);

    sampleBuffer->Release();

    if (FAILED(hrWriteSample))
    {
        setLastErrorMessage("Unable to write audio buffer");
        return;
    }

    mLastErrorMessage = "";

    
    return;

}


void UBWindowsMediaFile::startNewChapter(const QString& pLabel, long timestamp)
{
    MarkerInfo mi;
    mi.label = pLabel;
    mi.timestamp = timestamp;

    mChapters << mi;
}


void UBWindowsMediaFile::setLastErrorMessage(const QString& error)
{
    mLastErrorMessage = error;
    qWarning() << error;
}


