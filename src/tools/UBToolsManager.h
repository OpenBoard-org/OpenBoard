/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
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

        void addTool(const UBToolDescriptor& tool)
        {
            mDescriptors << tool;
        }

        UBToolDescriptor ruler;
        UBToolDescriptor axes;
        UBToolDescriptor protractor;
        UBToolDescriptor compass;
        UBToolDescriptor mask;
        UBToolDescriptor triangle;
        UBToolDescriptor magnifier;
        UBToolDescriptor cache;

        QString iconFromToolId(QString id) { return mToolsIcon.value(id);}

    private:
        UBToolsManager(QObject *parent = 0);
        virtual ~UBToolsManager();

        static UBToolsManager* sManager;

        QList<UBToolDescriptor> mDescriptors;

        QMap<QString ,QString> mToolsIcon;

};

#endif /* UBTOOLSMANAGER_H_ */
