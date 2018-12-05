/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBWEBPLUGINPDFWIDGET_H
#define UBWEBPLUGINPDFWIDGET_H

#include "web/UBWebPluginWidget.h"

class PDFRenderer;
class QProgressBar;

class UBWebPluginPDFWidget : public UBWebPluginWidget
{
    Q_OBJECT

    public:
        UBWebPluginPDFWidget(const QUrl &url, QWidget *parent = 0);
        virtual ~UBWebPluginPDFWidget();

        virtual QString title() const;

    public slots:
        virtual void zoomIn();
        virtual void zoomOut();

    protected:
        virtual void handleFile(const QString &filePath);

        virtual void paintEvent(QPaintEvent *paintEvent);

        virtual void keyReleaseEvent(QKeyEvent *keyEvent);

    protected slots:
        virtual bool previousPage();
        virtual bool nextPage();

    private:
        QToolButton mPreviousPageButton;
        QToolButton mNextPageButton;
        QAction *mPreviousPageAction;
        QAction *mNextPageAction;

        PDFRenderer *mRenderer;
        qreal mScale;
        int mPageNumber;
};

#endif // UBWEBPLUGINPDFWIDGET_H
