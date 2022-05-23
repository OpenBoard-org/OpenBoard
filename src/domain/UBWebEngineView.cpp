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



#include "UBWebEngineView.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QMainWindow>
#include <QMenu>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBDisplayManager.h"

UBWebEngineView::UBWebEngineView(QWidget *parent) : QWebEngineView(parent)
  , mInspectorWindow(nullptr)
{

}

void UBWebEngineView::inspectPage()
{
    if (mInspectorWindow)
    {
        mInspectorWindow->activateWindow();
    }
    else
    {
        QRect controlGeometry = UBApplication::displayManager->screenGeometry(ScreenRole::Control);
        QRect inspectorGeometry(controlGeometry.left() + 50, controlGeometry.top() + 50, controlGeometry.width() / 2, controlGeometry.height() / 2);

        mInspectorWindow = new QMainWindow();
        mInspectorWindow->setAttribute(Qt::WA_DeleteOnClose, true);
        mInspectorWindow->setFocusPolicy(Qt::ClickFocus);

        QWebEngineView *inspector = new QWebEngineView();
        mInspectorWindow->setCentralWidget(inspector);
        mInspectorWindow->setGeometry(inspectorGeometry);
        mInspectorWindow->show();

        page()->setDevToolsPage(inspector->page());

        connect(mInspectorWindow, &QObject::destroyed, [this](){
            page()->setDevToolsPage(nullptr);
            mInspectorWindow = nullptr;
        });
    }
}

void UBWebEngineView::closeInspector()
{
    if (mInspectorWindow)
    {
        page()->setDevToolsPage(nullptr);
        mInspectorWindow->close();
        mInspectorWindow->deleteLater();
        mInspectorWindow = nullptr;
    }
}

void UBWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QMenu *menu = createStandardContextMenu();
#else
    QMenu *menu = page()->createStandardContextMenu();
#endif

    // suppress actions requiring a new window
    const QList<QWebEnginePage::WebAction> suppressed = {
        QWebEnginePage::OpenLinkInNewWindow,
        QWebEnginePage::OpenLinkInNewTab,
        QWebEnginePage::OpenLinkInNewBackgroundTab,
        QWebEnginePage::InspectElement,
        QWebEnginePage::ViewSource
    };

    for (QWebEnginePage::WebAction action : suppressed) {
        menu->removeAction(page()->action(action));
    }

    // add web inspector action
    QAction *action = new QAction(menu);
    action->setText(tr("Open Web Inspector"));
    connect(action, &QAction::triggered, [this]() { inspectPage(); });

    menu->addSeparator();
    menu->addAction(action);

    // set background style
    QPalette palette = menu->palette();
    palette.setBrush(QPalette::Window, QBrush(Qt::white));
    menu->setPalette(palette);
    menu->setBackgroundRole(QPalette::Window);
    menu->setAutoFillBackground(true);

    menu->popup(event->globalPos());
}
