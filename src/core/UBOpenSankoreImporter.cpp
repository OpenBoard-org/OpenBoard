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


#include <QProcess>

#include "UBOpenSankoreImporter.h"
#include "UBSettings.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"


UBOpenSankoreImporter::UBOpenSankoreImporter(QObject *parent) :
    QObject(parent)
{
    if(UBSettings::settings()->appLookForOpenSankoreInstall->get().toBool() &&
            QDir(UBSettings::userDataDirectory().replace(qApp->applicationName(),"Sankore")).exists()){
        if(UBApplication::mainWindow->yesNoQuestion(tr("Open-Sankoré data detected"),tr("Open-Sankoré directory is present on the disk. It's possible to import the Open-Sankoré documents into OpenBoard as the preferences. Pushing Ok will close OpenBoard and run the importer application."))){
            QProcess newProcess;
#ifdef Q_WS_X11
            newProcess.startDetached(qApp->applicationDirPath()+"/Importer/OpenBoardImporter");
#elif defined Q_WS_MACX
            newProcess.startDetached(qApp->applicationDirPath()+"/../Resources/OpenBoardImporter.app/Contents/MacOS/OpenBoardImporter");
#else
            newProcess.startDetached(qApp->applicationDirPath()+"/Importer/OpenBoardImporter.exe");
#endif
            qApp->exit(0);
        }
    }
}


