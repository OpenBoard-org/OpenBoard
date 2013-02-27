/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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


        static UBToolsManager* manager();
        static void destroy();

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
        UBToolDescriptor triangle;
        UBToolDescriptor magnifier;
        UBToolDescriptor cache;
        UBToolDescriptor aristo;

        QString iconFromToolId(QString id) { return mToolsIcon.value(id);}

    private:
        UBToolsManager(QObject *parent = 0);
        virtual ~UBToolsManager();

        static UBToolsManager* sManager;

        QList<UBToolDescriptor> mDescriptors;

        QMap<QString ,QString> mToolsIcon;

};

#endif /* UBTOOLSMANAGER_H_ */
