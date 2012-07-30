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

#include "UBIniFileParser.h"

#include "core/memcheck.h"

UBIniFileParser::UBIniFileParser(const QString& pFileName)
{
    QFile file(pFileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream reader(&file);

        QString catName = "";
        while (!reader.atEnd())
        {
            QString line = reader.readLine();
            catName = readLine(line, catName);
        }

        file.close();
    }

}

QString UBIniFileParser::readLine(const QString& pLine, const QString& pCatName)
{
    if (pLine.indexOf(QChar('[')) == 0)
    {
        QString cat = pLine.mid(1,pLine.indexOf(QChar(']')) -1);
        mIniDico[cat] = new QHash<QString, QString>;
        return cat;
    }
    else
    {
        int equalIndex = pLine.indexOf(QChar('='));
        if (equalIndex > 0)
        {
            QString key = pLine.left(equalIndex);
            QString value = pLine.mid(equalIndex+1, pLine.length()-1);
            mIniDico[pCatName]->insert(key, value);
        }
    }
    return pCatName;
}

UBIniFileParser::~UBIniFileParser()
{
    foreach(QString key, mIniDico.keys())
    {
        delete mIniDico[key];
    }
}

QString UBIniFileParser::getStringValue(const QString& pSection, const QString& pKey)
{
    if (mIniDico[pSection])
    {
        return mIniDico[pSection]->value(pKey);
    }
    return "";
}
