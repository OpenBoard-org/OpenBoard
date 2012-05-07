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

#ifndef IDATASTORAGE_H
#define IDATASTORAGE_H

#include <QString>
#include <QMap>

typedef enum
{
    eElementType_START,
    eElementType_END,
    eElementType_UNIQUE
}eElementType;

typedef struct
{
    QString name;
    QMap<QString,QString> attributes;
    eElementType type;
}tIDataStorage;

class IDataStorage
{
public:
    virtual void load(QString element) = 0;
    virtual QVector<tIDataStorage*>save(int pageIndex) = 0 ;
};
#endif // IDATASTORAGE_H
