/*
 * UBReplyWRS970VotingSystem.h
 *
 *  Created on: 3 mars 2010
 *      Author: Luc
 */

#ifndef UBREPLYWRS970VOTINGSYSTEM_H_
#define UBREPLYWRS970VOTINGSYSTEM_H_

#include "UBAbstractVotingSystem.h"

#include "wrs970.h"

class UBReplyWRS970VotingSystem : public UBAbstractVotingSystem
{
    Q_OBJECT;

    public:
        UBReplyWRS970VotingSystem(QWidget *parent = 0);
        virtual ~UBReplyWRS970VotingSystem();

    public slots:

        virtual bool connectToVotingSystem();
        virtual bool disconnectFromVotingSystem();

        virtual bool startPolling();
        virtual bool stopPolling();

        virtual QMap<int, int> votingState();

        bool isVotingSystemAvailable();

    protected:
        WRS970::WRS970X *mReplyVotingSystem;

    private:

        QWidget* mParent;
        QMap <int, int> mVotes;
        int mMaxKeyValue;
        int mCurrentQuestionNumber;

    private slots:

        void keypadData(QString, int, int, QString, int, QString, int, int);

        void error(QString error);

        void debug(QString error);

};

#endif /* UBREPLYWRS970VOTINGSYSTEM_H_ */
