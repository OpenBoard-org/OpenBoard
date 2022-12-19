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


/**
 * @brief The UBEmbedController class provides means to present embeddable content to the
 * user and to create web widgets from selected content.
 *
 * @sa UBEmbedParser, UBEmbedContent
 */
class UBEmbedController : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructs a UBEmbedController object which is a child of the given parent widget.
     * @param parent Parent widget. Used to position and size the embed dialog so that it
     * appears on top of that widget.
     */
    UBEmbedController(QWidget* parent = nullptr);

    /**
     * @brief Destroys the UBEmbedController object.
     */
    virtual ~UBEmbedController();

    /**
     * @brief Update the list of embeddable content shown in the embed dialog.
     *
     * In addition to the entries of the list provided by this call, the dialog always
     * contains a first entry offering to embed the complete web page.
     *
     * This function may be called multiple times to update the list.
     *
     * @param pAllContent List of embeddable content descriptions.
     */
    void updateListOfEmbeddableContent(const QList<UBEmbedContent>& pAllContent);

public slots:
    /**
     * @brief Show the dialog allowing to select embeddable content.
     *
     * Before calling this function, the URL and title of the page should be set by
     * pageUrlChanged and pageTitleChanged.
     */
    void showEmbedDialog();

    /**
     * @brief Hide the dialog allowing to select embeddable content.
     */
    void hideEmbedDialog() const;

    /**
     * @brief Update the URL of the web page containing the embeddable content.
     *
     * This slot must be called before the UBEmbedController can create a web widget
     * embedding the complete web page.
     *
     * @param url URL of web page.
     */
    void pageUrlChanged(const QUrl& url);

    /**
     * @brief Update the title of the web page containing the embeddable content.
     *
     * This slot should be called before the UBEmbedController can create a web widget
     * embedding the complete web page. This information is used to propose a name for
     * the web widget.
     *
     * @param title Title of web page.
     */
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
