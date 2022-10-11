/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBEmbedParser.h"

#include <QRegularExpression>
#include <QStringList>
#include <QDomAttr>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWebEngineView>

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"


UBEmbedParser::UBEmbedParser(QWebEngineView *parent, const char* name)
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

    qDebug() << "Created UBOEmbedParser";
}

UBEmbedParser::~UBEmbedParser()
{

}

bool UBEmbedParser::hasEmbeddedContent() const
{
    return !mContents.empty();
}

QList<UBEmbedContent> UBEmbedParser::embeddedContent()
{
    return mContents;
}

void UBEmbedParser::onLoadFinished()
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

void UBEmbedParser::parse(const QString& html)
{
    qDebug() << "parse" << html.length();
    mContents.clear();
    mParsedTitles.clear();

    // extract all <link> and <iframe> tags upto but not including the final >
    static const QRegularExpression exp("(<|&lt;)(link|iframe)([^>]*)");
    QStringList results;
    int count = 0;
    QRegularExpressionMatchIterator matches = exp.globalMatch(html);

    while (matches.hasNext())
    {
        QRegularExpressionMatch match = matches.next();
        ++count;
        QStringList res = match.capturedTexts();

        if ("" != res.at(0))
        {
            results << res.at(0);
        }
    }

    QList<QString> oembedUrls;

    for (QString result : results)
    {
        // replace entities
        result = result.trimmed().replace("&lt;", "<").replace("&gt;", ">").replace("\\&quot;", "\"").replace("\\", "");

        // again cut at first ">"
        int greater = result.indexOf('>');

        if (greater >= 0)
        {
            result.truncate(greater);
        }

        // add trailing slash if necessary
        if (result.at(result.length() - 1) != '/')
        {
            result += "/";
        }

        QString qsNode = result + ">";

        if (qsNode.startsWith("<iframe"))
        {
            // here we can already create a UBEmbedContent from the iframe
            UBEmbedContent content = createIframeContent(qsNode);

            if (content.type() == UBEmbedType::IFRAME)
            {
                mContents << content;
            }
        }
        else
        {
            QDomDocument domDoc;
            domDoc.setContent(qsNode);
            QDomElement node = domDoc.documentElement();

            //  At this point, we have a node that is the <link> element.
            //  Now we have to parse its attributes in order to check if it is a oEmbed node or not
            QDomAttr typeAttribute = node.attributeNode("type");

            if (typeAttribute.value().contains("oembed"))
            {
                // The node is an oembed one! We have to get the url
                QDomAttr hrefAttribute = node.attributeNode("href");
                QString url = hrefAttribute.value();
                oembedUrls.append(url);
            }
        }
    }

    mPending = oembedUrls.size();
    qDebug() << "UBOEmbedParser.parse, pending =" << mPending;

    if (0 == mPending)
    {
        emit parseResult(mView, !mContents.empty());
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

void UBEmbedParser::fetchOEmbed(const QString &url)
{
    QUrl qurl = QUrl::fromEncoded(url.toLatin1());

    QNetworkRequest req(qurl);
    QNetworkReply* reply = mpNam->get(req);
    connect(reply, &QNetworkReply::finished, [this,reply](){
        onFinished(reply);
    });
}

void UBEmbedParser::onFinished(QNetworkReply *reply)
{
    if (QNetworkReply::NoError == reply->error())
    {
        QString receivedDatas = reply->readAll().constData();
        qDebug() << "Received oEmbed" << receivedDatas;
        UBEmbedContent crntContent;
        // The received datas can be in two different formats: JSON or XML
        QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();

        if (contentType.contains("xml"))
        {
            // XML !
            crntContent = getXMLInfos(receivedDatas);
        }
        else if (contentType.contains("json"))
        {
            // JSON !
            crntContent = getJSONInfos(receivedDatas);
        }

        //  As we don't want duplicates, we have to check if the content title has already
        //  been parsed.
        if ("" != crntContent.mTitle && !mParsedTitles.contains(crntContent.mTitle))
        {
            qDebug() << "Found" << crntContent.mTitle;
            mParsedTitles << crntContent.mTitle;
            mContents << crntContent;
        }

    }
    else
    {
        //  We decided to not handle the error case here. If there is a problem with
        //  getting the oembed content information, we just don't handle it: the content
        //  will not be available for importation.
    }

    // Decrement the number of content to analyze
    mPending--;
    qDebug() << "Remaining pending" << mPending;

    if (0 == mPending)
    {
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
UBEmbedContent UBEmbedParser::getJSONInfos(const QString &json) const
{
    UBEmbedContent content;

    QJsonObject jsonObject = QJsonDocument::fromJson(json.toUtf8()).object();
    QString version = jsonObject.value("version").toString();

    if (version != "1.0")
    {
        qDebug() << "Unknown oEmbed version" << version;
        return content;
    }

    QString type = jsonObject.value("type").toString();
    content.mType = UBEmbedContent::typeFromString(type);
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
UBEmbedContent UBEmbedParser::getXMLInfos(const QString &xml) const
{
    UBEmbedContent content;

    QDomDocument domDoc;
    domDoc.setContent(xml);
    QDomElement oembed = domDoc.documentElement();

    QString version = oembed.firstChildElement("version").text();

    if (version != "1.0")
    {
        qDebug() << "Unknown oEmbed version" << version;
        return content;
    }

    QString type = oembed.firstChildElement("type").text();
    content.mType = UBEmbedContent::typeFromString(type);

    QDomNodeList children = oembed.childNodes();

    for (int i = 0; i < children.size(); ++i)
    {
        QDomNode child = children.at(i);
        QString tag = child.nodeName();
        QString value = child.toElement().text();

        if ("provider_url" == tag)
        {
            content.mProviderUrl = value;
        }
        else if ("title" == tag)
        {
            content.mTitle = value;
        }
        else if ("html" == tag)
        {
            content.mHtml = value;
        }
        else if ("author_name" == tag)
        {
            content.mAuthorName = value;
        }
        else if ("height" == tag)
        {
            content.mHeight = value.toInt();
        }
        else if ("thumbnail_width" == tag)
        {
            content.mThumbWidth = value.toInt();
        }
        else if ("width" == tag)
        {
            content.mWidth = value.toInt();
        }
        else if ("author_url" == tag)
        {
            content.mAuthorUrl = value;
        }
        else if ("provider_name" == tag)
        {
            content.mProviderName = value;
        }
        else if ("thumbnail_url" == tag)
        {
            content.mThumbUrl = value;
        }
        else if ("thumbnail_height" == tag)
        {
            content.mThumbHeight = value.toInt();
        }
        else if ("url" == tag)
        {
            content.mUrl = value; // This case appears only for type = photo
        }
    }

    return content;
}

UBEmbedContent UBEmbedParser::createIframeContent(const QString &html) const
{
    qDebug() << "Found iframe" << html;
    UBEmbedContent content;
    content.mType = UBEmbedType::IFRAME;

    // DOM parsing is not possible, as iframes contain boolean attributes
    // eg "allowfullscreen", which are not XML conformant
    static const QRegularExpression matchAttribute("(\\w+)(=\"([^\"]*)\")?");

    int pos = 7;    // size of initial <iframe
    QRegularExpressionMatchIterator matches = matchAttribute.globalMatch(html, pos);

    while (matches.hasNext())
    {
        QRegularExpressionMatch match = matches.next();
        QStringList res = match.capturedTexts();

        if (res.size() >= 4)
        {
            QString tag = res.at(1);
            QString value = res.at(3);

            if ("title" == tag || "subject" == tag)
            {
                content.mTitle = value;
            }
            else if ("height" == tag)
            {
                bool ok;
                content.mHeight = value.toInt(&ok);

                if (ok && content.mHeight <= 1)
                {
                    // hidden iframe, skip
                    content.mType = UBEmbedType::UNKNOWN;
                }
            }
            else if ("width" == tag)
            {
                bool ok;
                content.mWidth = value.toInt(&ok);

                if (ok && content.mWidth <= 1)
                {
                    // hidden iframe, skip
                    content.mType = UBEmbedType::UNKNOWN;
                }
            }
            else if ("src" == tag)
            {
                // check valid src
                if (value.left(4) == "http")
                {
                    content.mUrl = value;
                }
                else
                {
                    // invalid src, skip
                    content.mType = UBEmbedType::UNKNOWN;
                }
            }
            else if ("aria-hidden" == tag && "true" == value)
            {
                // hidden iframe, skip
                content.mType = UBEmbedType::UNKNOWN;
            }
            else if ("style" == tag)
            {
                // check for display: none
                value.replace(" ", "");


                if (value.contains("display:none"))
                {
                    // hidden iframe, skip
                    content.mType = UBEmbedType::UNKNOWN;
                }
            }
        }
    }

    content.mHtml = html;

    return content;
}
