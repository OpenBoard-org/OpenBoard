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
