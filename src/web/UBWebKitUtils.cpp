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


#include <QDebug>
#include <QWebFrame>

#include "UBWebKitUtils.h"

#include "core/memcheck.h"

UBWebKitUtils::UBWebKitUtils()
{
    // NOOP
}

UBWebKitUtils::~UBWebKitUtils()
{
    // NOOP
}

QList<UBWebKitUtils::HtmlObject> UBWebKitUtils::objectsInFrame(QWebFrame* frame)
{
    QList<UBWebKitUtils::HtmlObject> htmlObjects;

    if (frame)
    {
        QVariant res = frame->evaluateJavaScript("window.document.getElementsByTagName('embed').length");

        bool ok;

        int count = res.toInt(&ok);
        if (ok)
        {
            for (int i = 0; i < count; i++)
            {
                QString queryWidth = QString("window.document.getElementsByTagName('embed')[%1].width").arg(i);

                QString queryHeigth = QString("window.document.getElementsByTagName('embed')[%1].height").arg(i);

                QString querySource = QString("window.document.getElementsByTagName('embed')[%1].src").arg(i);

                res = frame->evaluateJavaScript(queryWidth);

                int width = res.toInt(&ok);
                if (width == 0 || !ok)
                {
                    qDebug() << "Width is not defined in pixel. 640 will be used";
                    width = 640;
                }

                res = frame->evaluateJavaScript(queryHeigth);

                int heigth = res.toInt(&ok);
                if (heigth == 0 || !ok)
                {
                    qDebug() << "Height is not defined in pixel. 480 will be used";
                    heigth = 480;
                }
                res = frame->evaluateJavaScript(querySource);

                QUrl baseUrl = frame->url();
                QUrl relativeUrl = QUrl(res.toString());

                QString source = baseUrl.resolved(relativeUrl).toString();

                if (source.trimmed().length() == 0)
                    continue;

                htmlObjects << UBWebKitUtils::HtmlObject(source, width, heigth);
            }
        }
    }

    return htmlObjects;
}


