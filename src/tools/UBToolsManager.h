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
        UBToolDescriptor triangle;
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
