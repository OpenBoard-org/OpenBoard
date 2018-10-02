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




#ifndef UBGLOBALS_H
#define UBGLOBALS_H

#include <QString>
#include <QFile>

#define DELETEPTR(ptr) if(NULL != ptr){ \
    delete ptr; \
    ptr = NULL; \
    }

#define SET_STYLE_SHEET() {\
    setAttribute(Qt::WA_StyledBackground, true);\
    QString style;\
    QFile f(":style.qss");\
    if(f.exists() && f.open(QIODevice::ReadOnly))\
            style = QString(f.readAll());\
     setStyleSheet(style);\
}

#ifdef Q_OS_WIN

#define WARNINGS_DISABLE __pragma(warning(push, 0));
#define WARNINGS_ENABLE __pragma(warning(pop));

#ifdef NO_THIRD_PARTY_WARNINGS
// disabling warning level to 0 and save old state
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE
#else
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_DISABLE __pragma(warning(push));
#endif //#ifdef NO_THIRD_PARTY_WARNINGS

#else //#ifdef Q_OS_WIN

#define WARNINGS_DISABLE _Pragma("GCC diagnostic push"); \
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\""); \
_Pragma("GCC diagnostic ignored \"-Wunused-variable\""); \
_Pragma("GCC diagnostic ignored \"-Wsign-compare\"");

#define WARNINGS_ENABLE _Pragma("GCC diagnostic pop");

#ifdef NO_THIRD_PARTY_WARNINGS
//disabling some warnings
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE
#else
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_DISABLE _Pragma("GCC diagnostic push");

#endif //#ifdef NO_THIRD_PARTY_WARNINGS

#endif //#ifdef Q_OS_WIN

// anyway on any OS
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE

#endif // UBGLOBALS_H

