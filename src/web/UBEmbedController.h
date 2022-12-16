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




#pragma once

#include <QDialog>
#include <QDir>
#include <QList>
#include <QString>
#include <QUrl>
#include <QWidget>

#include <web/UBEmbedContent.h>


namespace Ui
{
    class trapFlashDialog;
}


class UBEmbedController : public QObject
{
    Q_OBJECT

public:
    UBEmbedController(QWidget* parent = nullptr);
    virtual ~UBEmbedController();

    void updateListOfEmbeddableContent(const QList<UBEmbedContent>& pAllContent);

public slots:
    void showEmbedDialog();
    void hideEmbedDialog() const;

    void pageUrlChanged(const QUrl& url);
    void pageTitleChanged(const QString& title);

private slots:
    void textChanged(const QString& text);
    void selectFlash(int pFlashIndex);
    void createWidget();

private:
    void importWidgetInLibrary(const QDir& pSourceDir) const;

    QString generateFullPageHtml(const QUrl& url, const QString& pDirPath = QString(), bool pGenerateFile = false) const;
    QString generateHtml(const UBEmbedContent& pObject, const QString& pDirPath = QString(), bool pGenerateFile = false) const;
    QString generateIcon(const QString& pDirPath) const;
    void generateConfig(int pWidth, int pHeight, const QString& pDestinationPath) const;

    QString widgetNameForObject(const UBEmbedContent& pObject) const;

private:
    Ui::trapFlashDialog* mTrapFlashUi;
    QDialog* mTrapDialog;
    QWidget* mParentWidget;
    QList<UBEmbedContent> mAvailableContent;
    QUrl mUrl;
    QString mPageTitle;
};
