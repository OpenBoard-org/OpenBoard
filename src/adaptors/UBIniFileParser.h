/*
 * UBIniFileParser.h
 *
 *  Created on: 23 mars 2009
 *      Author: Julien
 */

#ifndef UBINIFILEPARSER_H_
#define UBINIFILEPARSER_H_

#include <QtGui>

/**
 * This class can parse a ini file format.
 */
class UBIniFileParser
{
    public:
        UBIniFileParser(const QString& pFile);
        virtual ~UBIniFileParser();

        /**
         * return the value for pKey in pSection. pSection is the group in which the key must be search (section are defined in [] in the ini file)
         */
        QString getStringValue(const QString& pSection, const QString& pKey);

    private:
        QString readLine(const QString& pLine, const QString& catName);
        QHash<QString, QHash<QString, QString>*> mIniDico;
};

#endif /* UBINIFILEPARSER_H_ */
