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




#pragma once

#include <QList>
#include <QObject>
#include <QString>

#include "web/UBEmbedContent.h"


// forward
class QNetworkAccessManager;
class QNetworkReply;

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


class UBEmbedParser : public QObject
{
    Q_OBJECT

public:
    explicit UBEmbedParser(QObject* parent = nullptr, const char* name="UBEmbedParser");

    virtual ~UBEmbedParser();
    bool hasEmbeddedContent() const;
    QList<UBEmbedContent> embeddedContent() const;

signals:
    void parseResult(bool hasEmbeddedContent);
    void cancelled();

public slots:
    void parse(const QString& html);

private slots:
    void fetchOEmbed(const QString& url);
    void onFinished(QNetworkReply* reply);

private:
    UBEmbedContent getJSONInfo(const QString& json) const;
    UBEmbedContent getXMLInfo(const QString& xml) const;
    UBEmbedContent createIframeContent(const QString& html) const;

private:
    QList<UBEmbedContent> mContents;
    QList<QString> mParsedTitles;
    QNetworkAccessManager* mpNam;
    bool mParsing;
    int mPending;
};
