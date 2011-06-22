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




