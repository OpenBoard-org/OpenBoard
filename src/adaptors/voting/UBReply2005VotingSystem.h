/*
 * UBReply2005VotingSystem.h
 *
 *  Created on: 12 feb. 2010
 *      Author: Luc
 */

#ifndef UBREPLY2005VOTINGSYSTEM_H_
#define UBREPLY2005VOTINGSYSTEM_H_

#include "UBAbstractVotingSystem.h"

#include <QtGui>

#include "replyxcontrol1.h"

class UBReply2005VotingSystem: public UBAbstractVotingSystem
{
    Q_OBJECT;

    public:
        UBReply2005VotingSystem(QWidget *parent = 0);
        virtual ~UBReply2005VotingSystem();

    public slots:

        virtual bool connectToVotingSystem();
        virtual bool disconnectFromVotingSystem();

        virtual bool startPolling();
        virtual bool stopPolling();

        virtual QMap<int, int> votingState();

        bool isVotingSystemAvailable();

    protected:
        ReplyXControl1::ReplyX *mReplyVotingSystem;

    private:

        QWidget* mParent;

        QMap <int, int> mVotes;

        int mMaxKeyValue;

    private slots:
         void keypadDataReceived(int, int);
};

#endif /* UBREPLY2005VOTINGSYSTEM_H_ */
