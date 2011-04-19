/*
 * UBAbstractVideoEncoder.cpp
 *
 *  Created on: 3 sept. 2009
 *      Author: Luc
 */

#include "UBAbstractVideoEncoder.h"

UBAbstractVideoEncoder::UBAbstractVideoEncoder(QObject *pParent)
    : QObject(pParent)
    , mFramesPerSecond(10)
    , mVideoSize(640, 480)
    , mVideoBitsPerSecond(1700000) // 1.7 Mbps
{
    // NOOP

}

UBAbstractVideoEncoder::~UBAbstractVideoEncoder()
{
    // NOOP
}


void UBAbstractVideoEncoder::newChapter(const QString& pLabel, long timestamp)
{
    Q_UNUSED(pLabel);
    Q_UNUSED(timestamp);
}
