/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "UBAbstractVotingSystem.h"

#ifdef Q_WS_WIN

#include "UBReply2005VotingSystem.h"
#include "UBReplyWRS970VotingSystem.h"

#endif

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

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
