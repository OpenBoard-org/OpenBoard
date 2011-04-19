/*
 * UBAbstractVotingSystem.cpp
 *
 *  Created on: 12 feb. 2010
 *      Author: Luc
 */

#include "UBAbstractVotingSystem.h"

#ifdef Q_WS_WIN

#include "UBReply2005VotingSystem.h"
#include "UBReplyWRS970VotingSystem.h"

#endif

#include "gui/UBMainWindow.h"

UBAbstractVotingSystem::UBAbstractVotingSystem(QObject *parent)
    : QObject(parent)
{
    // NOOP

}


UBAbstractVotingSystem::~UBAbstractVotingSystem()
{
    // NOOP
}


void UBAbstractVotingSystem::setLastError(QString pLastError)
{
    mLastError = pLastError;
    qDebug() << "Voting System error: " <<  pLastError;
}


UBAbstractVotingSystem* UBVotingSystemFactory::createVotingSystem()
{

#ifdef Q_WS_WIN

    // TODO UB 4.x .. be smarter
    UBReplyWRS970VotingSystem* wrs970 = new UBReplyWRS970VotingSystem(UBApplication::mainWindow);

    if (wrs970->isVotingSystemAvailable())
        return wrs970;
    else
    {
        delete wrs970;

        UBReply2005VotingSystem* reply2005 = new UBReply2005VotingSystem(UBApplication::mainWindow);

        if (reply2005->isVotingSystemAvailable())
            return reply2005;
        else
            delete reply2005;

        return 0;
    }

#else
    return 0;
#endif

}
