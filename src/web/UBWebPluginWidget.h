/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBWEBPLUGINWIDGET_H
#define UBWEBPLUGINWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QToolButton>

#include "network/UBHttpGet.h"

class UBWebPluginWidget : public QWidget
{
    Q_OBJECT

    public:
        UBWebPluginWidget(const QUrl &url, QWidget *parent = 0);
        virtual ~UBWebPluginWidget();

        virtual QString title() const;

    protected:
        virtual void handleFile(const QString &filePath) = 0;

        virtual void paintEvent(QPaintEvent *paintEvent) = 0;
        virtual void resizeEvent(QResizeEvent *event);

    private slots:
        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData);

    private:
        QProgressBar mLoadingProgressBar;
};

#endif // UBWEBPLUGINWIDGET_H
