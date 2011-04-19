#ifndef UBSTRINGUTILS_H
#define UBSTRINGUTILS_H

#include <QtCore>


class UBStringUtils
{

    private:
        UBStringUtils() {}
        ~UBStringUtils() {}

    public:
        static QStringList sortByLastDigit(const QStringList& source);

        static QString netxDigitizedName(const QString& source);

        static QString toCanonicalUuid(const QUuid& uuid);

        static QString toUtcIsoDateTime(const QDateTime& dateTime);


};

#endif // UBSTRINGUTILS_H
