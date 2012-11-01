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


#ifndef UBTRAPFLASHCONTROLLER_H_
#define UBTRAPFLASHCONTROLLER_H_

#include <QtGui>
#include "UBWebKitUtils.h"

namespace Ui
{
    class trapFlashDialog;
}


class UBTrapFlashController : public QObject
{
    Q_OBJECT;
    public:
        UBTrapFlashController(QWidget* parent = 0);
        virtual ~UBTrapFlashController();

        void showTrapFlash();
        void hideTrapFlash();

    public slots:
        void updateTrapFlashFromPage(QWebFrame* pCurrentWebFrame);
        void text_Changed(const QString &);
        void text_Edited(const QString &);


    private slots:
        void selectFlash(int pFlashIndex);
        void createWidget();

    private:

        void updateListOfFlashes(const QList<UBWebKitUtils::HtmlObject>& pAllFlashes);

        QString widgetNameForObject(UBWebKitUtils::HtmlObject pObject);

        QString generateFullPageHtml(const QString& pDirPath, bool pGenerateFile);
        QString generateHtml(const UBWebKitUtils::HtmlObject& pObject, const QString& pDirPath, bool pGenerateFile);

        QString generateIcon(const QString& pDirPath);

        void generateConfig(int pWidth, int pHeight, const QString& pDestinationPath);

        void importWidgetInLibrary(QDir pSourceDir);

        Ui::trapFlashDialog* mTrapFlashUi;
        QDialog* mTrapFlashDialog;
        QWidget* mParentWidget;
        QWebFrame* mCurrentWebFrame;
        QList<UBWebKitUtils::HtmlObject> mAvailableFlashes;
};


#endif /* UBTRAPFLASHCONTROLLER_H_ */
