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

#ifndef UBABSTRACTVOTINGSYSTEM_H_
#define UBABSTRACTVOTINGSYSTEM_H_

#include <QtCore>

#include "core/UBApplication.h"

class UBAbstractVotingSystem : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString lastError READ lastError);

    public:
        UBAbstractVotingSystem(QObject *parent = 0);
        virtual ~UBAbstractVotingSystem();

        virtual bool connectToVotingSystem() = 0;

        virtual bool startPolling() = 0;

        virtual bool stopPolling() = 0;

        virtual bool disconnectFromVotingSystem() = 0;

        virtual QMap<int, int> votingState() = 0;

        QString lastError() const
        {
            return mLastError;
        }

    signals:

        void voteReceived(int keypadID, int value);

        void errorReceived(const QString& error);

    protected:

        void setLastError(QString pLastError);

    private:

        QString mLastError;

};

class UBVotingSystemFactory : public QObject
{
    private:
        UBVotingSystemFactory()
        {
            // NOOP
        }

        ~UBVotingSystemFactory()
        {
            // NOOP
        }

    public:

        static UBAbstractVotingSystem* createVotingSystem();
};

#endif /* UBABSTRACTVOTINGSYSTEM_H_ */
