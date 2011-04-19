/*
 * UBAbstractVotingSystem.h
 *
 *  Created on: 12 feb. 2010
 *      Author: Luc
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
