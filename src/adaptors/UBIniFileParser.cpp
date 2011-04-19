/*
 * UBIniFileParser.cpp
 *
 *  Created on: 23 mars 2009
 *      Author: Julien
 */

#include "UBIniFileParser.h"

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
