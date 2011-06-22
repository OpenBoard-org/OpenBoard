/*
 * UBReply2005VotingSystem.cpp
 *
 *  Created on: 12 feb. 2010
 *      Author: Luc
 */

#include "UBReply2005VotingSystem.h"

#include "core/UBSettings.h"

#include "core/memcheck.h"

UBReply2005VotingSystem::UBReply2005VotingSystem(QWidget *parent)
    : UBAbstractVotingSystem(parent)
    , mParent(parent)
    , mReplyVotingSystem(0)
    , mMaxKeyValue(99)
{
    // NOOP
}

UBReply2005VotingSystem::~UBReply2005VotingSystem()
{
    if (mReplyVotingSystem)
        delete mReplyVotingSystem;
}

bool UBReply2005VotingSystem::isVotingSystemAvailable()
{
    if (!mReplyVotingSystem)
        mReplyVotingSystem = new ReplyXControl1::ReplyX(mParent);

    return (!mReplyVotingSystem->isNull());
}


bool UBReply2005VotingSystem::connectToVotingSystem()
{
    bool result = false;

    if (isVotingSystemAvailable())
    {
        connect(mReplyVotingSystem, SIGNAL(OnKeypadDataReceived(int, int)),
                 this, SLOT(keypadDataReceived(int, int)));

        mReplyVotingSystem->SetSerialPort(UBSettings::settings()->replyWWSerialPort->get().toInt());
        mReplyVotingSystem->SetReplyModel(ReplyXControl1::mReply2005);

        result = mReplyVotingSystem->Connect();

        if (!result)
            setLastError("Cannot connect to voting system");
    }
    else
    {
        setLastError("Reply2005 driver not available");
    }

    return result;
}

void UBReply2005VotingSystem::keypadDataReceived(int keyPadID, int val)
{
    qDebug() << "received vote from " << keyPadID << " : " << val;

    mVotes.insert(keyPadID, val);

    emit voteReceived(keyPadID, val);
}


bool UBReply2005VotingSystem::startPolling()
{
    mVotes.clear();

    if (isVotingSystemAvailable())
    {
        mReplyVotingSystem->StartPolling();
    }
    else
    {
        setLastError("Reply2005 driver not available");
    }

    return true;
}


bool UBReply2005VotingSystem::stopPolling()
{
    if (isVotingSystemAvailable())
    {
        mReplyVotingSystem->StopPolling();
    }
    else
    {
        setLastError("Reply2005 driver not available");
    }

    return true;
}


bool UBReply2005VotingSystem::disconnectFromVotingSystem()
{
    if (isVotingSystemAvailable())
    {
        mReplyVotingSystem->Disconnect();
    }
    else
    {
        setLastError("Reply2005 driver not available");
    }

    return true;
}

QMap<int, int> UBReply2005VotingSystem::votingState()
{
    QMap<int, int> results;

    for(int i = 0; i < mMaxKeyValue; i++)
    {
        results.insert(i, 0);
    }

    foreach(int vote, mVotes.values())
    {
        int voteCount = 0;

        if(results.keys().contains(vote))
        {
            voteCount = results.value(vote);
            voteCount++;
            results.insert(vote, voteCount);
        }
    }

    return results;
}


