/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UBFavoriteToolPalette.h"

#include "core/UBSettings.h"

#include "board/UBBoardController.h"

#include "tools/UBToolsManager.h"

#include "domain/UBAbstractWidget.h"

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"

UBFavoriteToolPalette::UBFavoriteToolPalette(QWidget* parent)
    : UBActionPalette(Qt::Horizontal, parent)
{
    QWidget *container = new QWidget(this);
    container->setStyleSheet("QWidget {background-color: transparent}");

    QGridLayout *gridLayout = new QGridLayout();
    container->setLayout(gridLayout);
    layout()->addWidget(container);

    QList<QAction*> toolsActions;

    QStringList favoritesToolUris = UBSettings::settings()->favoritesNativeToolUris->get().toStringList();

    foreach(QString uri, favoritesToolUris)
    {
        UBToolsManager::UBToolDescriptor desc = UBToolsManager::manager()->toolByID(uri);

        if (desc.label.length() > 0 && !desc.icon.isNull())
        {
            QAction *action = new QAction(desc.label + " " + desc.version, this);
            action->setData(QUrl(desc.id));
            action->setIcon(desc.icon);
            connect(action, SIGNAL(triggered()), this, SLOT(addFavorite()));

            toolsActions << action;
        }
    }

    QDir favoritesDir(UBSettings::settings()->userInteractiveFavoritesDirectory());
    QStringList favoritesSubDirs =  favoritesDir.entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDir::Name);
    QStringList appPathes;

    foreach(QString subDirName, favoritesSubDirs)
    {
        appPathes << favoritesDir.path() + "/" + subDirName;
    }

    foreach(QString widgetPath, appPathes)
    {
        QAction *action = new QAction(UBAbstractWidget::widgetName(QUrl::fromLocalFile(widgetPath)), this);
        action->setData(QUrl::fromLocalFile(widgetPath));
        action->setIcon(QIcon(UBAbstractWidget::iconFilePath(QUrl::fromLocalFile(widgetPath))));
        connect(action, SIGNAL(triggered()), this, SLOT(addFavorite()));

        toolsActions << action;
    }


    if (toolsActions.size() < 4)
    {
        QStringList toolsIDs = UBToolsManager::manager()->allToolIDs();

        foreach(QString id, favoritesToolUris)
            toolsIDs.removeAll(id);

        while(toolsIDs.size() > 0 && toolsActions.size() < 4)
        {
            UBToolsManager::UBToolDescriptor desc = UBToolsManager::manager()->toolByID(toolsIDs.takeFirst());

            if (desc.label.length() > 0)
            {
                QAction *action = new QAction(desc.label + " " + desc.version, this);
                action->setData(QUrl(desc.id));
                action->setIcon(desc.icon);
                connect(action, SIGNAL(triggered()), this, SLOT(addFavorite()));

                toolsActions << action;
            }
        }
    }

    int i = 0;

    foreach(QAction* action, toolsActions)
    {
        UBActionPaletteButton* button = createPaletteButton(action, container);
        gridLayout->addWidget(button, i / 4, i % 4);
        mActions << action;
        i++;
    }

    setClosable(true);
    setButtonIconSize(QSize(128, 128));
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    groupActions();
}


void UBFavoriteToolPalette::addFavorite()
{
    // we need the sender :-( hugly ...
    QAction* action = qobject_cast<QAction*>(sender());

    if(action)
    {
        QVariant widgetPathVar = action->data();
        if (!widgetPathVar.isNull())
        {
            UBApplication::boardController->downloadURL(widgetPathVar.toUrl());
        }
    }
}

UBFavoriteToolPalette::~UBFavoriteToolPalette()
{
    // NOOP
}
