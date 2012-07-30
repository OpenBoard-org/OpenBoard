/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#include "UBStringUtils.h"

#include "core/memcheck.h"

QStringList UBStringUtils::sortByLastDigit(const QStringList& sourceList)
{
    // we look for a set of digit after non digits and before a .
    QRegExp rx("\\D(\\d+)\\.");

    QMultiMap<int, QString> elements;

    foreach(QString source, sourceList)
    {
        int pos = rx.lastIndexIn(source);

        int digit = -1;

        if (pos >= 0)
        {
            digit = rx.cap(1).toInt();
        }

        elements.insert(digit, source);
    }

    QStringList result;

    QList<int> keys = elements.keys();
    qSort(keys);

    foreach(int key, keys)
    {
        QList<QString> values = elements.values(key);
        qSort(values);
        foreach(QString val, values)
        {
            if (!result.contains(val))
                result << val;
        }
    }

    return result;
}


QString UBStringUtils::netxDigitizedName(const QString& source)
{

    // we look for a set of digit after non digits and at the end
    QRegExp rx("\\D(\\d+)");

    int pos = rx.lastIndexIn(source);

    int digit = -1;

    if (pos >= 0)
    {
        digit = rx.cap(1).toInt();
    }

    QString ret(source);

    if (digit == -1)
    {
        return ret + " 1";
    }
    else
    {
        QString s("%1");
        s = s.arg(digit + 1);
        return ret.replace(rx.cap(1), s);
    }
}

QString UBStringUtils::toCanonicalUuid(const QUuid& uuid)
{
    QString s = uuid.toString();

    if(s.startsWith("{"))
            s = s.right(s.length() - 1);

    if(s.endsWith("}"))
            s = s.left(s.length() - 1);


    return s;
}

QString UBStringUtils::toUtcIsoDateTime(const QDateTime& dateTime)
{
    return dateTime.toUTC().toString(Qt::ISODate) + "Z";
}

QDateTime UBStringUtils::fromUtcIsoDate(const QString& dateString)
{
    return QDateTime::fromString(dateString,Qt::ISODate).toLocalTime();
}




