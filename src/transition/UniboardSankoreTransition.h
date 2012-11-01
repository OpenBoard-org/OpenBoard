/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UNIBOARDSANKORETRANSITION_H
#define UNIBOARDSANKORETRANSITION_H

#include <QObject>
#include <QFileInfo>
#include <QThread>
#include "gui/UBUpdateDlg.h"
#include "document/UBDocumentProxy.h"

class UniboardSankoreThread : public QThread
{
    Q_OBJECT
public:
    UniboardSankoreThread(QObject* parent = 0);
    ~UniboardSankoreThread();

    void run();

};

class UniboardSankoreTransition : public QObject
{
    Q_OBJECT
public:
    explicit UniboardSankoreTransition(QObject *parent = 0);
    ~UniboardSankoreTransition();

    bool checkDocumentDirectory(QString& documentDirectoryPath);

    void documentTransition();
    bool checkPage(QString& sankorePagePath);
    bool updateSankoreHRef(QString &sankoreDocumentPath);
    bool checkWidget(QString& sankoreWidgetPath);
    bool updateIndexWidget(QString& sankoreWidgetPath);
    void executeTransition();


private:
    void rollbackDocumentsTransition(QFileInfoList& fileInfoList);
    UBUpdateDlg* mTransitionDlg;

protected:
    QString mUniboardSourceDirectory;
    QString mOldSankoreDirectory;
    UniboardSankoreThread* mThread;

signals:
    void transitionFinished(bool result);
    void docAdded(UBDocumentProxy* doc);
    void transitioningFile(QString documentName);

private slots:
    void startDocumentTransition();

};

#endif // UNIBOARDSANKORETRANSITION_H
