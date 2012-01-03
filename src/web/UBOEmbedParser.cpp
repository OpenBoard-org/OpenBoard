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
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QScriptValue>
#include <QScriptEngine>
#include <QDebug>

#include "UBOEmbedParser.h"

UBOEmbedParser::UBOEmbedParser(QObject *parent, const char* name)
{
    setObjectName(name);
    mParsedTitles.clear();
    connect(this, SIGNAL(parseContent(QString)), this, SLOT(onParseContent(QString)));
}

UBOEmbedParser::~UBOEmbedParser()
{

}

void UBOEmbedParser::setNetworkAccessManager(QNetworkAccessManager *nam)
{
    mpNam = nam;
    connect(mpNam, SIGNAL(finished(QNetworkReply*)), this, SLOT(onFinished(QNetworkReply*)));
}

void UBOEmbedParser::parse(const QString& html)
{
    mContents.clear();
    QString query = "<link([^>]*)>";
    QRegExp exp(query);
    QStringList results;
    int count = 0;
    int pos = 0;
    while ((pos = exp.indexIn(html, pos)) != -1) {
        ++count;
        pos += exp.matchedLength();
        QStringList res = exp.capturedTexts();
        if("" != res.at(1)){
            results << res.at(1);
        }
    }

    QVector<QString> oembedUrls;

    if(2 <= results.size()){
        for(int i=1; i<results.size(); i++){
            if("" != results.at(i)){
                QString qsNode = QString("<link%0>").arg(results.at(i));
                QDomDocument domDoc;
                domDoc.setContent(qsNode);
                QDomNode linkNode = domDoc.documentElement();

                //  At this point, we have a node that is the <link> element. Now we have to parse its attributes
                //  in order to check if it is a oEmbed node or not
                QDomAttr typeAttribute = linkNode.toElement().attributeNode("type");
                if(typeAttribute.value().contains("oembed")){
                    // The node is an oembed one! We have to get the url and the type of oembed encoding
                    QDomAttr hrefAttribute = linkNode.toElement().attributeNode("href");
                    QString url = hrefAttribute.value();
                    oembedUrls.append(url);
                }
            }
        }
    }

    mPending = oembedUrls.size();

    if(0 == mPending){
        emit oembedParsed(mContents);
    }else{
        // Here we start the parsing (finally...)!
        for(int i=0; i<oembedUrls.size(); i++){
            emit parseContent(oembedUrls.at(i));
        }
    }
}

/**
  /brief Extract the oembed infos from the JSON
  @param jsonUrl as the url of the JSON file
  */
sOEmbedContent UBOEmbedParser::getJSONInfos(const QString &json)
{
    sOEmbedContent content;

    QScriptValue scriptValue;
    QScriptEngine scriptEngine;
    scriptValue = scriptEngine.evaluate ("(" + json + ")");

    QString providerUrl = scriptValue.property("provider_url").toString();
    QString title = scriptValue.property("title").toString();
    QString html = scriptValue.property("html").toString();
    QString authorName = scriptValue.property("author_name").toString();
    int height = scriptValue.property("height").toInteger();
    int thumbnailWidth = scriptValue.property("thumbnail_width").toInteger();
    int width = scriptValue.property("width").toInteger();
    float version = scriptValue.property("version").toString().toFloat();
    QString authorUrl = scriptValue.property("author_url").toString();
    QString providerName = scriptValue.property("provider_name").toString();
    QString thumbnailUrl = scriptValue.property("thumbnail_url").toString();
    QString type = scriptValue.property("type").toString();
    int thumbnailHeight = scriptValue.property("thumbnail_height").toInteger();

    content.providerUrl = providerUrl;
    content.title = title;
    content.html = html;
    content.author = authorName;
    content.height = height;
    content.thumbWidth = thumbnailWidth;
    content.width = width;
    content.version = version;
    content.authorUrl = authorUrl;
    content.providerName = providerName;
    content.thumbUrl = thumbnailUrl;
    content.type = type;
    content.thumbHeight = thumbnailHeight;

    if("photo" == content.type){
        content.url = scriptValue.property("url").toString();
    }else if("video" == content.type){
        QStringList strl = content.html.split('\"');
        for(int i=0; i<strl.size(); i++){
            if(strl.at(i).endsWith("src=") && strl.size() > (i+1)){
                content.url = strl.at(i+1);
            }
        }
    }

    return content;
}

/**
  /brief Extract the oembed infos from the XML
  @param xmlUrl as the url of the XML file
  */
sOEmbedContent UBOEmbedParser::getXMLInfos(const QString &xml)
{
    sOEmbedContent content;

    QDomDocument domDoc;
    domDoc.setContent(xml);
    QDomNode oembed = domDoc.documentElement();

    QDomNodeList children = oembed.toElement().childNodes();

    for(int i=0; i<children.size(); i++){
        QDomNode node = children.at(i);
        QString tag = node.nodeName();
        QString value = node.toElement().text();
        if("provider_url" == tag){
            content.providerUrl = value;
        }else if("title" == tag){
            content.title = value;
        }else if("html" == tag){
            content.html = value;
        }else if("author_name" == tag){
            content.author = value;
        }else if("height" == tag){
            content.height = value.toInt();
        }else if("thumbnail_width" == tag){
            content.thumbWidth = value.toInt();
        }else if("width" == tag){
            content.width = value.toInt();
        }else if("version" == tag){
            content.version = value.toFloat();
        }else if("author_url" == tag){
            content.authorUrl = value;
        }else if("provider_name" == tag){
            content.providerName = value;
        }else if("thumbnail_url" == tag){
            content.thumbUrl = value;
        }else if("type" == tag){
            content.type = value;
        }else if("thumbnail_height" == tag){
            content.thumbHeight = value.toInt();
        }else if("url" == tag){
            content.url = value; // This case appears only for type = photo
        }
    }

    if("video" == content.type){
        QStringList strl = content.html.split('\"');
        for(int i=0; i<strl.size(); i++){
            if(strl.at(i).endsWith("src=") && strl.size() > (i+1)){
                content.url = strl.at(i+1);
            }
        }
    }

    return content;
}

void UBOEmbedParser::onParseContent(QString url)
{
    QUrl qurl;
    qurl.setEncodedUrl(url.toAscii());

    QNetworkRequest req;
    req.setUrl(qurl);
    if(NULL != mpNam){
        mpNam->get(req);
    }
}

void UBOEmbedParser::onFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError err = reply->error();
    if(QNetworkReply::NoError == reply->error()){
        QString receivedDatas = reply->readAll().constData();
        sOEmbedContent crntContent;
        // The received datas can be in two different formats: JSON or XML
        if(receivedDatas.contains("<oembed>")){
            // XML !
            crntContent = getXMLInfos(receivedDatas);
        }else if(receivedDatas.contains("{\"provider_url")){
            // JSON !
            crntContent = getJSONInfos(receivedDatas);
        }

        //  As we don't want duplicates, we have to check if the content title has already
        //  been parsed.
        if("" != crntContent.title && !mParsedTitles.contains(crntContent.title)){
            mParsedTitles << crntContent.title;
            mContents << crntContent;
        }

    }else{
        //  We decided to not handle the error case here. If there is a problem with
        //  getting the oembed content information, we just don't handle it: the content
        //  will not be available for importation.
    }

    // Decrement the number of content to analyze
    mPending--;
    if(0 == mPending){
        //  All the oembed contents have been parsed. We notify it!
        emit oembedParsed(mContents);
    }
}
