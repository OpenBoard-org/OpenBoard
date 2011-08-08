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

#ifndef UBWIDGETVOTINGSYSTEMAPI_H_
#define UBWIDGETVOTINGSYSTEMAPI_H_

#include <QtCore>

class UBAbstractVotingSystem;

class UBWidgetVotingSystemAPI : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString lastError READ lastError);

    public:
        UBWidgetVotingSystemAPI(QObject* parent = 0);
        virtual ~UBWidgetVotingSystemAPI();

        QString lastError();
        void setLastError(const QString& pLastError);

    signals:
        void error(const QString&);

    public slots:

        bool startPoll();
        QVariantMap closePoll();

    private slots:

       void errorReceived(const QString& error);

    private:
        UBAbstractVotingSystem* mVotingSystem;
        QString mLastError;

};

#endif /* UBWIDGETVOTINGSYSTEMAPI_H_ */
