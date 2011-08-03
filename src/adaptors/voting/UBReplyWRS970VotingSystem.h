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
