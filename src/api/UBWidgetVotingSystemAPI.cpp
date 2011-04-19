/*
 * UBWidgetVotingSystemAPI.cpp
 *
 *  Created on: 16 feb 2010
 *      Author: Luc
 */

#include "UBWidgetVotingSystemAPI.h"

#include "adaptors/voting/UBAbstractVotingSystem.h"


UBWidgetVotingSystemAPI::UBWidgetVotingSystemAPI(QObject* parent)
    : QObject(parent)
    , mVotingSystem(0)
{
    // NOOP
}


UBWidgetVotingSystemAPI::~UBWidgetVotingSystemAPI()
{
    // NOOP
}


QString UBWidgetVotingSystemAPI::lastError()
{
    return mLastError;
}


void UBWidgetVotingSystemAPI::setLastError(const QString& pLastError)
{
    mLastError = pLastError;
    qDebug() << mLastError;

    emit error(mLastError);
}


void UBWidgetVotingSystemAPI::errorReceived(const QString& pLastError)
{
    setLastError(pLastError);
}


bool UBWidgetVotingSystemAPI::startPoll()
{
    if (!mVotingSystem)
    {
        mVotingSystem = UBVotingSystemFactory::createVotingSystem();
    }

    if (!mVotingSystem)
    {
        setLastError("No voting system available");
        return false;
    }

    connect(mVotingSystem, SIGNAL(errorReceived(const QString&)), this, SLOT(errorReceived(const QString&)));

    bool connected = mVotingSystem->connectToVotingSystem();

    if (!connected)
    {
        return false;
    }

    bool started = mVotingSystem->startPolling();

    if (!started)
    {
        return false;
    }

    return true;
}


QVariantMap UBWidgetVotingSystemAPI::closePoll()
{
    if (!mVotingSystem)
        return QVariantMap();

    mVotingSystem->stopPolling();

    QMap<int, int> results = mVotingSystem->votingState();

    mVotingSystem->disconnectFromVotingSystem();

    delete mVotingSystem;
    mVotingSystem = 0;

    QVariantMap scriptResults;

    foreach(int key, results.keys())
    {
        scriptResults.insert(QString("%1").arg(key), QVariant(results.value(key)));
    }

    return scriptResults;
}
