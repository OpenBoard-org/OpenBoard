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


/**
 * @brief The UBEmbedParser class provides a means for parsing HTML for embeddable content.
 *
 * HTML pages may provide embeddable content via several mechanisms:
 * - By using `<iframe>` tags.
 * - By providing information on how to embed content using `<iframe>`.
 * - By using `<link>` tags with `type="oembed"`.
 *
 * The UBEmbedParser scans HTML input for such constructs, retrieves oembed information if
 * necessary and builds a list of embeddable content. This list can be provided to an
 * UBEmbedController, which offers the option to create a web widget from a selected
 * list entry.
 *
 * ### Embeddable content from <iframe> tags
 *
 * A web page may contain embedded content referenced by an `<iframe>` tag. Such content is
 * in turn embeddable in other web pages. Such tags and their content are added to the list.
 *
 * Sometimes the web page contains instructions on how to embed some content, e.g. in a text
 * field containing the necessary `<iframe>` tag. In these cases the opening `<` is represented
 * as entity using `&lt;`. The UBEmbedParser also tries to identify and use such information.
 *
 * ### Embeddable content using oEmbed.
 *
 * Some web pages provide instructions for embedding using the [oEmbed](https://oembed.com/)
 * standard. Here special `<link>` tags are used to point to an external resource providing
 * information about embeddable content. The UBEmbedParser retrieves such information as XML
 * or JSON documents and includes the result in its list of embeddable content.
 *
 * Here is an example of oEmbed information in XML format:
 * ```xml
 * <oembed>
 *   <provider_url>http://www.youtube.com/</provider_url>
 *   <title>EPISODE 36 Traditional Mediums</title>
 *   <html>
 *     <iframe width="480" height="270" src="http://www.youtube.com/embed/C3lApsNmdwM?fs=1&feature=oembed" frameborder="0" allowfullscreen></iframe>
 *   </html>
 *   <author_name>FZDSCHOOL</author_name>
 *   <height>270</height>
 *   <thumbnail_width>480</thumbnail_width>
 *   <width>480</width>
 *   <version>1.0</version>
 *   <author_url>http://www.youtube.com/user/FZDSCHOOL</author_url>
 *   <provider_name>YouTube</provider_name>
 *   <thumbnail_url>http://i4.ytimg.com/vi/C3lApsNmdwM/hqdefault.jpg</thumbnail_url>
 *   <type>video</type>
 *   <thumbnail_height>360</thumbnail_height>
 * </oembed>
 * ```
 *
 * @sa UBEmbedContent, UBEmbedController
 */
class UBEmbedParser : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a UBEmbedParser object which is a child of parent and the given object name.
     * @param parent Parent object, default is `nullptr`.
     * @param name Object name, default is `"UBEmbedParser"`.
     */
    explicit UBEmbedParser(QObject* parent = nullptr, const char* name="UBEmbedParser");

    /**
     * @brief Destroys the UBEmbedParser object.
     */
    virtual ~UBEmbedParser();

    /**
     * @brief Returns `true` if the parser has found any embeddable content.
     *
     * To retrieve the final result, this function should be invoked after parseResult was emitted.
     *
     * @return `true` if the parser has found any embeddable content.
     * @sa parse, parseResult
     */
    bool hasEmbeddedContent() const;

    /**
     * @brief Retrieves the list of embeddable content.
     *
     * To retrieve the final result, this function should be invoked after parseResult was emitted.
     * The list remains valid until a new parse process is started.
     *
     * @return List of UBEmbedContent found on the HTML page.
     * @sa parse, parseResult
     */
    QList<UBEmbedContent> embeddedContent() const;

signals:
    /**
     * @brief This signal is emitted when the parsing is complete and the result may be retrieved.
     *
     * @param hasEmbeddedContent `true` if any embeddable content was found.
     */
    void parseResult(bool hasEmbeddedContent);

    /**
     * @brief This signal is emitted when the parsing was cancelled by invoking parse again while
     * the previous parse process was not completed.
     *
     * This signal is mainly for internal use. It cancels any pending network requests for oembed
     * information. A parseResult signal is not emitted for the aborted parse operation.
     */
    void cancelled();

public slots:
    /**
     * @brief Start parsing the given HTML input.
     *
     * Starts the parsing process for the given HTML input. The parsing is an asynchronous process,
     * as it may involve retrieving oEmbed information. The end of this proces is signaled by
     * emitting parseResult.
     *
     * @param html HTML document to parse.
     */
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
