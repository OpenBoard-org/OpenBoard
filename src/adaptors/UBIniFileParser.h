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
