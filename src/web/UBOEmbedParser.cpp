/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBOEmbedParser.h"

#include <QRegExp>
#include <QStringList>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebEngineView>

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"


UBOEmbedContent::UBOEmbedContent()
    : mType(UBOEmbedType::UNKNOWN), mThumbWidth(0), mThumbHeight(0), mWidth(0), mHeight(0)
{

}

UBOEmbedType UBOEmbedContent::type() const
{
    return mType;
}

QString UBOEmbedContent::title() const
{
    return mTitle;
}

QString UBOEmbedContent::authorName() const
{
    return mAuthorName;
}

QUrl UBOEmbedContent::authorUrl() const
{
    return mAuthorUrl;
}

QString UBOEmbedContent::providerName() const
{
    return mProviderName;
}

QUrl UBOEmbedContent::providerUrl() const
{
    return mProviderUrl;
}

QUrl UBOEmbedContent::thumbUrl() const
{
    return mThumbUrl;
}

int UBOEmbedContent::thumbWidth() const
{
    return mThumbWidth;
}

int UBOEmbedContent::thumbHeight() const
{
    return mThumbHeight;
}

int UBOEmbedContent::width() const
{
    return mWidth;
}

int UBOEmbedContent::height() const
{
    return mHeight;
}

QString UBOEmbedContent::html() const
{
    return mHtml;
}

QUrl UBOEmbedContent::url() const
{
    return mUrl;
}


UBOEmbedParser::UBOEmbedParser(QWebEngineView *parent, const char* name)
    : QObject(parent)
{
    setObjectName(name);
    mView = parent;
    mpNam = UBNetworkAccessManager::defaultAccessManager();
    mParsing = false;
    mPending = 0;

    connect(mView, &QWebEngineView::loadProgress, [this](int progress){
        // Note: The loadFinished signal is not always emitted, but progress = 100 is.
        if (progress == 100)
        {
            onLoadFinished();
        }
    });
// DEBUG
//    connect(mView, &QWebEngineView::loadStarted, [](){qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "loadStarted";});
//    connect(mView, &QWebEngineView::loadProgress, [](int p){qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "loadProgress" << p;});
//    connect(mView, &QWebEngineView::loadFinished, [](){qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "loadFinished";});
//    connect(mView, &QWebEngineView::renderProcessTerminated, [](){qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "renderProcessTerminated";});
//    connect(mView, &QWebEngineView::urlChanged, [](){qDebug() << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << "urlChanged";});
    qDebug() << "Created UBOEmbedParser";
}

UBOEmbedParser::~UBOEmbedParser()
{

}

bool UBOEmbedParser::hasEmbeddedContent()
{
    return !mContents.empty();
}

QVector<UBOEmbedContent> UBOEmbedParser::embeddedContent()
{
    return mContents;
}

void UBOEmbedParser::onLoadFinished()
{
    qDebug() << "loadFinished";
    if (!mParsing)
    {
        mParsing = true;
        mView->page()->toHtml([this](const QString &html) {
            parse(html);
        });
    }
}

void UBOEmbedParser::parse(const QString& html)
{
    qDebug() << "parse" << html.length();
    mContents.clear();
    mParsedTitles.clear();

    // extract all <link> tags
    QRegExp exp("<link([^>]*)>");
    QStringList results;
    int count = 0;
    int pos = 0;

    while ((pos = exp.indexIn(html, pos)) != -1)
    {
        ++count;
        pos += exp.matchedLength();
        QStringList res = exp.capturedTexts();

        if ("" != res.at(1)) {
            results << res.at(1);
        }
    }

    QVector<QString> oembedUrls;

    for (const QString& link : results)
    {
        QString qsNode = QString("<link%0>").arg(link);
        QDomDocument domDoc;
        domDoc.setContent(qsNode);
        QDomElement linkNode = domDoc.documentElement();

        //  At this point, we have a node that is the <link> element. Now we have to parse its attributes
        //  in order to check if it is a oEmbed node or not
        QDomAttr typeAttribute = linkNode.attributeNode("type");

        if (typeAttribute.value().contains("oembed"))
        {
            // The node is an oembed one! We have to get the url
            QDomAttr hrefAttribute = linkNode.attributeNode("href");
            QString url = hrefAttribute.value();
            oembedUrls.append(url);
        }
    }

    mPending = oembedUrls.size();
    qDebug() << "UBOEmbedParser.parse, pending =" << mPending;

    if (0 == mPending)
    {
        emit parseResult(mView, false);
        mParsing = false;
    }
    else
    {
        // Here we start the parsing (finally...)!
        for(const QString& url : oembedUrls)
        {
            fetchOEmbed(url);
        }
    }
}

void UBOEmbedParser::fetchOEmbed(const QString &url)
{
    QUrl qurl = QUrl::fromEncoded(url.toLatin1());

    QNetworkRequest req(qurl);
    QNetworkReply* reply = mpNam->get(req);
    connect(reply, &QNetworkReply::finished, [this,reply](){
        onFinished(reply);
    });
}

void UBOEmbedParser::onFinished(QNetworkReply *reply)
{
    if (QNetworkReply::NoError == reply->error())
    {
        QString receivedDatas = reply->readAll().constData();
        qDebug() << "Received oEmbed" << receivedDatas;
        UBOEmbedContent crntContent;
        // The received datas can be in two different formats: JSON or XML
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

        if (contentType.contains("xml")) {
            // XML !
            crntContent = getXMLInfos(receivedDatas);
        } else if (contentType.contains("json")) {
            // JSON !
            crntContent = getJSONInfos(receivedDatas);
        }

        //  As we don't want duplicates, we have to check if the content title has already
        //  been parsed.
        if ("" != crntContent.mTitle && !mParsedTitles.contains(crntContent.mTitle)) {
            qDebug() << "Found" << crntContent.mTitle;
            mParsedTitles << crntContent.mTitle;
            mContents << crntContent;
        }

    } else {
        //  We decided to not handle the error case here. If there is a problem with
        //  getting the oembed content information, we just don't handle it: the content
        //  will not be available for importation.
    }

    // Decrement the number of content to analyze
    mPending--;
    qDebug() << "Remaining pending" << mPending;

    if (0 == mPending) {
        //  All the oembed contents have been parsed. We notify it!
        emit parseResult(mView, hasEmbeddedContent());
        mParsing = false;
    }

    reply->deleteLater();
}

/**
  /brief Extract the oembed infos from the JSON
  @param jsonUrl as the url of the JSON file
  */
UBOEmbedContent UBOEmbedParser::getJSONInfos(const QString &json) const
{
    UBOEmbedContent content;

    QJsonObject jsonObject = QJsonDocument::fromJson(json.toUtf8()).object();
    QString version = jsonObject.value("version").toString();

    if (version != "1.0") {
        qDebug() << "Unknown oEmbed version" << version;
        return content;
    }

    QString type = jsonObject.value("type").toString();

    if (type == "photo")
    {
        content.mType = UBOEmbedType::PHOTO;
    }
    else if (type == "video")
    {
        content.mType = UBOEmbedType::VIDEO;
    }
    else if (type == "link")
    {
        content.mType = UBOEmbedType::LINK;
    }
    else if (type == "rich")
    {
        content.mType = UBOEmbedType::RICH;
    }

    content.mTitle = jsonObject.value("title").toString();
    content.mAuthorName = jsonObject.value("author_name").toString();
    content.mAuthorUrl = jsonObject.value("author_url").toString();
    content.mProviderName = jsonObject.value("provider_name").toString();
    content.mProviderUrl = jsonObject.value("provider_url").toString();
    content.mThumbUrl = jsonObject.value("thumbnail_url").toString();
    content.mThumbWidth = jsonObject.value("thumbnail_width").toInt();
    content.mThumbHeight = jsonObject.value("thumbnail_height").toInt();
    content.mWidth = jsonObject.value("width").toInt();
    content.mHeight = jsonObject.value("height").toInt();
    content.mHtml = jsonObject.value("html").toString();
    content.mUrl = jsonObject.value("url").toString();

    return content;
}

/**
  /brief Extract the oembed infos from the XML
  @param xmlUrl as the url of the XML file
  */
UBOEmbedContent UBOEmbedParser::getXMLInfos(const QString &xml) const
{
    UBOEmbedContent content;

    QDomDocument domDoc;
    domDoc.setContent(xml);
    QDomElement oembed = domDoc.documentElement();

    QString version = oembed.firstChildElement("version").text();

    if (version != "1.0") {
        qDebug() << "Unknown oEmbed version" << version;
        return content;
    }

    QString type = oembed.firstChildElement("type").text();

    if (type == "photo")
    {
        content.mType = UBOEmbedType::PHOTO;
    }
    else if (type == "video")
    {
        content.mType = UBOEmbedType::VIDEO;
    }
    else if (type == "link")
    {
        content.mType = UBOEmbedType::LINK;
    }
    else if (type == "rich")
    {
        content.mType = UBOEmbedType::RICH;
    }

    QDomNodeList children = oembed.childNodes();

    for (int i = 0; i < children.size(); ++i)
    {
        QDomNode child = children.at(i);
        QString tag = child.nodeName();
        QString value = child.toElement().text();

        if ("provider_url" == tag) {
            content.mProviderUrl = value;
        } else if ("title" == tag) {
            content.mTitle = value;
        } else if ("html" == tag) {
            content.mHtml = value;
        } else if ("author_name" == tag) {
            content.mAuthorName = value;
        } else if ("height" == tag) {
            content.mHeight = value.toInt();
        } else if ("thumbnail_width" == tag) {
            content.mThumbWidth = value.toInt();
        } else if ("width" == tag) {
            content.mWidth = value.toInt();
        } else if ("author_url" == tag) {
            content.mAuthorUrl = value;
        } else if ("provider_name" == tag) {
            content.mProviderName = value;
        } else if ("thumbnail_url" == tag) {
            content.mThumbUrl = value;
        } else if ("thumbnail_height" == tag) {
            content.mThumbHeight = value.toInt();
        } else if ("url" == tag) {
            content.mUrl = value; // This case appears only for type = photo
        }
    }

    return content;
}
