/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UBGLOBALS_H
#define UBGLOBALS_H

#define DELETEPTR(ptr) if(NULL != ptr){ \
    delete ptr; \
    ptr = NULL; \
    }

#ifdef Q_WS_WIN

#define WARNINGS_DISABLE __pragma(warning(push, 0));
#define WARNINGS_ENABLE __pragma(warning(pop));

#ifdef NO_THIRD_PARTY_WARNINGS
// disabling warning level to 0 and save old state
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE
#else
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_DISABLE __pragma(warning(push));
#endif //#ifdef NO_THIRD_PARTY_WARNINGS
// anyway on WIN
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE

#else //#ifdef Q_WS_WIN

#define WARNINGS_DISABLE _Pragma("GCC diagnostic push"); \
_Pragma("GCC diagnostic ignored \"-Wunused-parameter\""); \
_Pragma("GCC diagnostic ignored \"-Wunused-variable\""); \
_Pragma("GCC diagnostic ignored \"-Wsign-compare\"");

#define WARNINGS_ENABLE _Pragma("GCC diagnostic pop"); 

#ifdef NO_THIRD_PARTY_WARNINGS
//disabling some warnings
#define THIRD_PARTY_WARNINGS_DISABLE WARNINGS_DISABLE

#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE
#else
// just save old state (needs for not empty define)
#define THIRD_PARTY_WARNINGS_ENABLE WARNINGS_ENABLE

#endif //#ifdef NO_THIRD_PARTY_WARNINGS

#endif //#ifdef Q_WS_WIN

#endif // UBGLOBALS_H
        
