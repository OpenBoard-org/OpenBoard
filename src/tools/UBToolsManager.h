/*
 * UBToolsManager.h
 *
 *  Created on: 11 nov. 2009
 *      Author: Luc
 */

#ifndef UBTOOLSMANAGER_H_
#define UBTOOLSMANAGER_H_

#include <QtGui>

#include "core/UBApplication.h"

class UBToolsManager : public QObject
{
    Q_OBJECT;

    public:

        class UBToolDescriptor
        {
            public:

                QString id;
                QPixmap icon;
                QString label;
                QString version;

        };


        static UBToolsManager* manager()
        {
            if (!sManager)
                sManager = new UBToolsManager(UBApplication::staticMemoryCleaner);
            return sManager;
        }

        QList<UBToolDescriptor> allTools()
        {
             return mDescriptors;
        }

        QStringList allToolIDs()
        {
            QStringList ids;

            foreach(UBToolDescriptor tool, allTools())
            {
                ids << tool.id;
            }

            return ids;
        }

        UBToolDescriptor toolByID(const QString& id)
        {
            foreach(UBToolDescriptor tool, allTools())
            {
                if (tool.id == id)
                    return tool;
            }

            return UBToolDescriptor();
        }

        UBToolDescriptor ruler;
        UBToolDescriptor protractor;
        UBToolDescriptor compass;
        UBToolDescriptor mask;

        QPixmap iconFromToolId(QString id) { return mToolsIcon.value(id);}

    private:
        UBToolsManager(QObject *parent = 0);
        virtual ~UBToolsManager();

        static UBToolsManager* sManager;

        QList<UBToolDescriptor> mDescriptors;

        QMap<QString ,QPixmap> mToolsIcon;

};

#endif /* UBTOOLSMANAGER_H_ */
