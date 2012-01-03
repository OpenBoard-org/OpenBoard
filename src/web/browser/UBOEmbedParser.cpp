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

#include <QRegExp>

#include "UBOEmbedParser.h"

/**********************************************************************************
  ----------------------------------------------------------------
  Here is an example of an embed content in an XML representation
  ----------------------------------------------------------------
    <oembed>
    <provider_url>http://www.youtube.com/</provider_url>
    <title>EPISODE 36 Traditional Mediums</title>
    <html>
    <iframe width="480" height="270" src="http://www.youtube.com/embed/C3lApsNmdwM?fs=1&feature=oembed" frameborder="0" allowfullscreen></iframe>
    </html>
    <author_name>FZDSCHOOL</author_name>
    <height>270</height>
    <thumbnail_width>480</thumbnail_width>
    <width>480</width>
    <version>1.0</version>
    <author_url>http://www.youtube.com/user/FZDSCHOOL</author_url>
    <provider_name>YouTube</provider_name>
    <thumbnail_url>http://i4.ytimg.com/vi/C3lApsNmdwM/hqdefault.jpg</thumbnail_url>
    <type>video</type>
    <thumbnail_height>360</thumbnail_height>
    </oembed>
***********************************************************************************/
typedef struct{
    QString providerUrl;
    QString title;
    QString author;
    int height;
    int thumbWidth;
    float version;
    QString authorUrl;
    QString providerName;
    QString thumbUrl;
    QString type;
    QString thumbHeight;
    QString sourceUrl;
}sOEmbedContent;

UBOEmbedParser::UBOEmbedParser(QObject *parent, const char* name)
{
    setObjectName(name);
    mParsedTitles.clear();
}

UBOEmbedParser::~UBOEmbedParser()
{

}

void UBOEmbedParser::parse(const QString& html, QList<QUrl> *pList)
{
    // get all the oembed links and parse their informations
    QString query = "";

}

/**
  /brief Extract the oembed infos from the JSON
  @param jsonUrl as the url of the JSON file
  */
void UBOEmbedParser::getJSONInfos(const QString &jsonUrl)
{

}

/**
  /brief Extract the oembed infos from the XML
  @param xmlUrl as the url of the XML file
  */
void UBOEmbedParser::getXMLInfos(const QString &xmlUrl)
{

}
