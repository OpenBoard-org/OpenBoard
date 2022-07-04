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



#include <QProcess>

#include "UBOpenSankoreImporter.h"
#include "UBSettings.h"

#include "core/UBApplication.h"
#include "frameworks/UBPlatformUtils.h"
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
    newProcess.startDetached(UBPlatformUtils::applicationResourcesDirectory()+"/importer/OpenBoardImporter");
#elif defined Q_OS_OSX
    newProcess.startDetached(qApp->applicationDirPath()+"/../Resources/OpenBoardImporter.app/Contents/MacOS/OpenBoardImporter");
#elif defined Q_OS_WIN
    QString importerPath = QDir::toNativeSeparators(qApp->applicationDirPath())+"\\OpenBoardImporter.exe";
    newProcess.startDetached("explorer.exe", QStringList() << importerPath);
#endif
    qApp->exit(0);

}


