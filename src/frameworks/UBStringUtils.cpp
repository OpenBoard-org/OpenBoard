/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBStringUtils.h"

#include "core/memcheck.h"

bool UBStringUtils::containsPrefix(const QStringList &prefixes, const QString &string, Qt::CaseSensitivity cs)
{
    for (const QString& prefix : prefixes)
    {
        if (string.startsWith(prefix, cs))
        {
            return true;
        }
    }

    return false;
}

QStringList UBStringUtils::sortByLastDigit(const QStringList& sourceList)
{
    // we look for a set of digit after non digits and before a .
    static const QRegularExpression rx("\\D(\\d+)\\.");

    QMultiMap<int, QString> elements;

    foreach(QString source, sourceList)
    {
        int pos = source.lastIndexOf(rx);

        int digit = -1;

        if (pos >= 0)
        {
            QRegularExpressionMatch match = rx.match(source, pos);
            digit = match.captured(1).toInt();
        }

        elements.insert(digit, source);
    }

    QStringList result;

    QList<int> keys = elements.keys();
    std::sort(keys.begin(), keys.end());

    foreach(int key, keys)
    {
        QList<QString> values = elements.values(key);
        std::sort(values.begin(), values.end());
        foreach(QString val, values)
        {
            if (!result.contains(val))
                result << val;
        }
    }

    return result;
}


QString UBStringUtils::nextDigitizedName(const QString& source)
{

    // we look for a set of digit after non digits and at the end
    static const QRegularExpression rx("\\D(\\d+)");

    int pos = source.lastIndexOf(rx);

    int digit = -1;
    QRegularExpressionMatch match;

    if (pos >= 0)
    {
        match = rx.match(source, pos);
        digit = match.captured(1).toInt();
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
        return ret.replace(match.captured(1), s);
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
    // It seems that in some previous versions of Qt, the 'Z'
    // was not in the string inserted in the QString returned by QDateTime::toString(Qt::DateFormat)
    // when the date format was UTC.
    // With Qt 5.15 and Qt6, it is, so we don't need to add it manually anymore
    return dateTime.toUTC().toString(Qt::ISODate);
}

QString UBStringUtils::toLittleEndian(const QDateTime& dateTime)
{
    return dateTime.toUTC().toString("dd/MM/yyyy hh:mm");
}

QDateTime UBStringUtils::fromUtcIsoDate(const QString& dateString)
{
    // Because of some changes in the behavior of QDateTime to QString conversions (cf. UBStringUtils::toUtcIsoDateTime)
    // invalid format is stored in metadatas and can produce an invalid QDateTime here with Qt > 6
    QString nonConstDateString = dateString;
    nonConstDateString.replace("ZZ", "Z");
    QDateTime date = QDateTime::fromString(nonConstDateString, Qt::ISODate);
    date.setTimeZone(QTimeZone::utc());
    return date.toLocalTime();
}

/**
 * @brief Create a list of trimmed, non-empty strings
 * @param list input list of strings
 * @return list of trimmed, nonempty strings
 */
QStringList UBStringUtils::trimmed(const QStringList &list)
{
    QStringList output;

    for (const QString& entry : list)
    {
        QString trimmedEntry = entry.trimmed();

        if (!trimmedEntry.isEmpty())
        {
            output << trimmedEntry;
        }
    }

    return output;
}




