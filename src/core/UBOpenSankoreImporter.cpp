/*
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



#include <QProcess>

#include "UBOpenSankoreImporter.h"
#include "UBSettings.h"

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "gui/UBOpenSankoreImporterWidget.h"

UBOpenSankoreImporter::UBOpenSankoreImporter(QWidget* mainWidget, QObject *parent) :
    QObject(parent)
  , mImporterWidget(NULL)
{
    if(UBSettings::settings()->appLookForOpenSankoreInstall->get().toBool() &&
            QDir(UBSettings::userDataDirectory().replace(qApp->applicationName(),"Sankore")).exists()){

        mImporterWidget = new UBOpenSankoreImporterWidget(mainWidget);

        connect(mImporterWidget->proceedButton(),SIGNAL(clicked()),mImporterWidget,SLOT(close()));
        connect(mImporterWidget->proceedButton(),SIGNAL(clicked()),this,SLOT(onProceedClicked()));
    }
}

void UBOpenSankoreImporter::onProceedClicked()
{
    QProcess newProcess;
#ifdef Q_OS_LINUX
    newProcess.startDetached(qApp->applicationDirPath()+"/importer/OpenBoardImporter");
#elif defined Q_OS_OSX
    newProcess.startDetached(qApp->applicationDirPath()+"/../Resources/OpenBoardImporter.app/Contents/MacOS/OpenBoardImporter");
#else
    // Windows does not allows to run easily an exe located in a subdirectory when the main
    // directory is placed into programs files.
    //newProcess.startDetached(qApp->applicationDirPath()+"\\Importer\\OpenBoardImporter.exe");
    newProcess.startDetached("C:/OpenBoard/Importer/OpenBoardImporter.exe");
#endif
    qApp->exit(0);

}


