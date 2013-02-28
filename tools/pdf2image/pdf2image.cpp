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


#include <QtGui>
#include <XPDFRenderer.h>
#include "core/UBPlatformUtils.h"

void usage(QString progName)
{
    qDebug() << "usage:" << progName << "pdfFile pageNumber width height outputDir [imageFormat=png]";
    qDebug() << "pdfFile is the path to the pdf file";
    qDebug() << "imageFormat must be one of " << QImageWriter::supportedImageFormats();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QPixmapCache::setCacheLimit(0);

    QStringList args = app.arguments();

    qreal width, height;
    QString pdfFile;
    QString imageFormat = "png";
    QString outputDir = ".";
    int pageNumber = 1;
    qDebug() << UBPlatformUtils::applicationResourcesDirectory();
    if (args.count() >= 6 && args.count() <= 7) {
        pdfFile    = args.at(1);
        pageNumber = args.at(2).toInt();
        width      = args.at(3).toDouble();
        height     = args.at(4).toDouble();
        outputDir  = args.at(5);
        if (args.count() == 7) {
            imageFormat = args.at(6);
        }
    } else {
        usage(args.at(0));
        return 1;
    }

    QString fileName = QFileInfo(pdfFile).completeBaseName();

    if (!QImageWriter::supportedImageFormats().contains(imageFormat.toAscii())) {
        usage(args.at(0));
        return 1;
    }

    XPDFRenderer pdf(pdfFile);

    if (!pdf.isValid()) {
        qCritical() << fileName << "appears to be an invalid pdf file";
        return 1;
    }

    if (pageNumber < 1 || pageNumber > pdf.pageCount()) {
        qCritical() << fileName << "has" << pdf.pageCount() << "pages";
        return 1;
    }

    //qDebug() << "Converting" << pdfFile << "(" << pageNumber << ") into" << imageFormat;

    QImage image(width, height, QImage::Format_ARGB32);

    QPainter p(&image);

    p.setBackground(Qt::transparent);
    p.eraseRect(0, 0, width, height);

    qreal pdfWidth = pdf.pageSize(pageNumber).width();
    qreal pdfHeight = pdf.pageSize(pageNumber).height();
    qreal ratio = qMin(width / pdfWidth, height / pdfHeight);
    p.scale(ratio, ratio);
    if (width > pdfWidth) {
        p.translate((pdfWidth - (width / ratio)) / -2, 0);
    }
    if (height > pdfHeight) {
        p.translate(0, (pdfHeight - (height / ratio)) / -2);
    }
    pdf.render(&p, pageNumber);

    QString pageStr = QString("%1").arg(pageNumber, 5, 10, QChar('0'));
    QString outputPath = outputDir + "/" + fileName + pageStr + "." + imageFormat;
    bool ok = image.save(outputPath, imageFormat.toAscii().constData());

    return ok ? 0 : 1;
}
