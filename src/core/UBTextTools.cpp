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
#include "UBTextTools.h"

QString UBTextTools::cleanHtmlCData(const QString &_html){

    QString clean = "";


    for(int i = 0; i < _html.length(); i+=1){
        if(_html.at(i) != '\0')
            clean.append(_html.at(i));
    }
    return clean;
}

QString UBTextTools::cleanHtml(const QString& _html){
    const QString START_TAG = "<!doctype";
    const QString END_TAG = "</html";

    QString cleanSource = "";
    QString simplifiedHtml = _html;

    int start = simplifiedHtml.toLower().indexOf(START_TAG);
    int end = simplifiedHtml.toLower().indexOf(END_TAG) + END_TAG.size();

    cleanSource = simplifiedHtml.mid(start, end);

    return cleanSource;
}
