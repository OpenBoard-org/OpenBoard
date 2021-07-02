/*
 * Copyright (C) 2015-2021 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBEMBEDCONTROLLER_H_
#define UBEMBEDCONTROLLER_H_

#include <QtGui>

#include <web/UBEmbedContent.h>

// forward
class QWebEngineView;

namespace Ui
{
    class trapFlashDialog;
}


class UBEmbedController : public QObject
{
    Q_OBJECT;
    public:
        UBEmbedController(QWidget* parent = 0);
        virtual ~UBEmbedController();

        void showTrapDialog();
        void hideTrapFlash();

    public slots:
        void updateTrapFlashFromView(QWebEngineView* pCurrentWebFrame);
        void text_Changed(const QString &);
        void text_Edited(const QString &);


    private slots:
        void selectFlash(int pFlashIndex);
        void createWidget();

    private:

        void updateListOfFlashes(const QList<UBEmbedContent>& pAllContent);

        QString widgetNameForObject(const UBEmbedContent& pObject);

        QString generateFullPageHtml(const QUrl& url, const QString& pDirPath, bool pGenerateFile);
        QString generateHtml(const UBEmbedContent& pObject, const QString& pDirPath, bool pGenerateFile);

        QString generateIcon(const QString& pDirPath);

        void generateConfig(int pWidth, int pHeight, const QString& pDestinationPath);

        void importWidgetInLibrary(QDir pSourceDir);

        Ui::trapFlashDialog* mTrapFlashUi;
        QDialog* mTrapDialog;
        QWidget* mParentWidget;
        QWebEngineView* mCurrentWebFrame;
        QList<UBEmbedContent> mAvailableContent;
};


#endif /* UBTRAPFLASHCONTROLLER_H_ */
