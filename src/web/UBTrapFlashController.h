/*
 * UBTrapFlashController.h
 *
 *  Created on: Feb 24, 2009
 *      Author: julienbachmann
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
