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

#include "UBWidgetVotingSystemAPI.h"

#include "adaptors/voting/UBAbstractVotingSystem.h"

#include "core/memcheck.h"

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
