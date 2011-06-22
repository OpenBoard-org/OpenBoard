/*
 * UBReplyWRS970VotingSystem.cpp
 *
 *  Created on: 3 mars 2010
 *      Author: Luc
 */

#include "UBReplyWRS970VotingSystem.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"

#include "core/memcheck.h"

UBReplyWRS970VotingSystem::UBReplyWRS970VotingSystem(QWidget *parent)
    : UBAbstractVotingSystem(parent)
    , mParent(parent)
    , mReplyVotingSystem(0)
    , mMaxKeyValue(99)
    , mCurrentQuestionNumber(0)
{
    // NOOP
}


UBReplyWRS970VotingSystem::~UBReplyWRS970VotingSystem()
{
    if (mReplyVotingSystem)
        delete mReplyVotingSystem;
}


bool UBReplyWRS970VotingSystem::isVotingSystemAvailable()
{
    if (!mReplyVotingSystem)
    {
        mReplyVotingSystem = new WRS970::WRS970X(mParent);

        connect(mReplyVotingSystem, SIGNAL(OnKeypadData(QString, int, int, QString, int, QString, int, int)),
                         this, SLOT(keypadData(QString, int, int, QString, int, QString, int, int)));

        connect(mReplyVotingSystem, SIGNAL(OnError(QString)), this, SLOT(error(QString)));
        connect(mReplyVotingSystem, SIGNAL(OnDebug(QString)), this, SLOT(debug(QString)));

        mReplyVotingSystem->SetVersion3(true);
        mReplyVotingSystem->SetExceptionsEnabled(false);
        mReplyVotingSystem->SetErrorsEnabled(true);

        bool ok;
        int maxKeypadsAllowed = UBSettings::settings()->replyPlusMaxKeypads->get().toInt(&ok);

        if (ok)
        {
            mReplyVotingSystem->SetMaxKeypadsAllowed(maxKeypadsAllowed);

            for(int i = 0 ; i < maxKeypadsAllowed; i += 100)
            {
                mReplyVotingSystem->AddStaticKeypad("00000", i + 1, true);
            }
        }

        if (UBApplication::app()->isVerbose())
        {
            mReplyVotingSystem->SetDebugEnabled(true);
        }
    }

    return (!mReplyVotingSystem->isNull());
}


bool UBReplyWRS970VotingSystem::connectToVotingSystem()
{
    bool connected = false;

    if (isVotingSystemAvailable())
    {
        mCurrentQuestionNumber = 0;
        QString targetBase;

        QString connectionURL = UBSettings::settings()->replyPlusConnectionURL->get().toString();

        if (connectionURL.toLower() == "usb")
        {
            QString baseSerial = mReplyVotingSystem->GetAvailableBases(false);

            if (baseSerial == "No device found")
            {
                setLastError("No WRS970 device found via USB");
                return false;
            }

            QStringList bases = baseSerial.split(",");

            if (bases.count() == 0)
            {
                setLastError("No base found via USB");
                return false;
            }

            targetBase = bases.at(0);

            mReplyVotingSystem->Connect(targetBase);
        }
        else
        {
            QString ip;
            QString port;

            targetBase = connectionURL;

            QStringList addressTokens = connectionURL.split(":");

            if (addressTokens.length() > 0)
                ip = addressTokens.at(0);

            if (addressTokens.length() > 1)
                port = addressTokens.at(1);

            if (ip.length() > 0)
            {
                mReplyVotingSystem->SetCommType(WRS970::ctTCP);
                mReplyVotingSystem->SetTCPAddress(ip);

                bool ok;
                int iPort = port.toInt(&ok);

                if (port.length() > 0 && ok)
                    mReplyVotingSystem->SetTCPPort(iPort);
            }

            mReplyVotingSystem->Connect(NULL);

        }

        connected = mReplyVotingSystem->Connected();

        if (!connected)
            setLastError("Error connecting to base " + targetBase);

        qDebug() << "Reply WRS 970 connected:" << connected;

        if (connected)
        {
            QString addressingMode = UBSettings::settings()->replyPlusAddressingMode->get().toString();

            if (addressingMode.toLower() == "static")
                mReplyVotingSystem->SetAddressMode(WRS970::addrStatic);
            else
                mReplyVotingSystem->SetAddressMode(WRS970::addrDynamic);
        }
    }
    else
    {
        setLastError("WRS970 driver not available");
    }

    return connected;
}


void UBReplyWRS970VotingSystem::keypadData(QString serialNumber, int keypadID, int timeStamp, QString version
        , int batteryLevel, QString value, int questionNumber, int keypadType)
{
    Q_UNUSED(serialNumber);
    Q_UNUSED(timeStamp);
    Q_UNUSED(version);
    Q_UNUSED(batteryLevel);
    Q_UNUSED(questionNumber);
    Q_UNUSED(keypadType);

    qDebug() << "Received vote from " << keypadID << " : " << value;

    mVotes.insert(keypadID, value.toInt());

    emit voteReceived(keypadID, value.toInt());
}


void UBReplyWRS970VotingSystem::error(QString error)
{
    setLastError(error);

    emit errorReceived(error);

    //UBApplication::showMessage(tr("Voting System Error: %1").arg(error));
}


void UBReplyWRS970VotingSystem::debug(QString error)
{
    qDebug() << error;
}



bool UBReplyWRS970VotingSystem::startPolling()
{
    mVotes.clear();

    if (isVotingSystemAvailable())
    {
        ++mCurrentQuestionNumber;
        mReplyVotingSystem->BeginQuestion(mCurrentQuestionNumber, WRS970::atSingleDigit);
    }
    else
    {
        setLastError("WRS970 driver not available");
    }

    return true;
}


bool UBReplyWRS970VotingSystem::stopPolling()
{
    if (isVotingSystemAvailable())
    {
        //mReplyVotingSystem->
    }
    else
    {
        setLastError("WRS970 driver not available");
    }

    return true;
}


bool UBReplyWRS970VotingSystem::disconnectFromVotingSystem()
{
    if (isVotingSystemAvailable())
    {
        mReplyVotingSystem->Disconnect();
    }
    else
    {
        setLastError("WRS970 driver not available");
    }

    return true;
}

QMap<int, int> UBReplyWRS970VotingSystem::votingState()
{
    QMap<int, int> results;

    for(int i = 0; i < mMaxKeyValue; i++)
    {
        results.insert(i, 0);
    }

    foreach(int vote, mVotes.values())
    {
        if(results.keys().contains(vote))
        {
            int voteCount = results.value(vote);
            voteCount++;
            results.insert(vote, voteCount);
        }
    }

    foreach(int key, results.keys())
    {
        qDebug() << "Vote" << key << ":" << results.value(key);
    }

    return results;
}


