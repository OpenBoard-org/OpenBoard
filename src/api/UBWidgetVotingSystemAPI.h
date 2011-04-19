/*
 * UBWidgetVotingSystemAPI.h
 *
 *  Created on: 16 feb. 2010
 *      Author: Luc
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
