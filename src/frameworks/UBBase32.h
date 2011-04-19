/*
 * UBBase32.h
 *
 *  Created on: 6 janv. 2010
 *      Author: Luc
 */

#ifndef UBBASE32_H_
#define UBBASE32_H_

#include <QtCore>

class UBBase32
{
    public:
        static QByteArray decode(const QString& base32String);

    protected:
        UBBase32() {}
        virtual ~UBBase32() {}

    private:
        static QString sBase32Chars;
        static int sBase32Lookup[];
};

#endif /* UBBASE32_H_ */
