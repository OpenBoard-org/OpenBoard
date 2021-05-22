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




#include "UBTrapFlashController.h"

#include <QtXml>
#include <QWebEnginePage>


#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBPlatformUtils.h"

#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "network/UBNetworkAccessManager.h"

#include "domain/UBGraphicsScene.h"

#include "board/UBBoardController.h"

#include "web/UBWebController.h"

#include "ui_trapFlash.h"

#include "core/memcheck.h"

UBTrapFlashController::UBTrapFlashController(QWidget* parent)
    : QObject(parent)
    , mTrapFlashUi(0)
    , mTrapFlashDialog(0)
    , mParentWidget(parent)
    , mCurrentWebFrame(0)
{
    // NOOP
}


UBTrapFlashController::~UBTrapFlashController()
{
    // NOOP
}


void UBTrapFlashController::showTrapFlash()
{
    if (!mTrapFlashDialog)
    {
        Qt::WindowFlags flag = Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint ;
        flag &= ~ Qt::WindowContextHelpButtonHint;
        flag &= ~ Qt::WindowMinimizeButtonHint;

        mTrapFlashDialog = new QDialog(mParentWidget, flag);
        mTrapFlashUi = new Ui::trapFlashDialog();
        mTrapFlashUi->setupUi(mTrapFlashDialog);

//        mTrapFlashUi->webView->page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());
        int viewWidth = mParentWidget->width() / 2;
        int viewHeight = mParentWidget->height() * 2. / 3.;
        mTrapFlashDialog->setGeometry(
                (mParentWidget->width() - viewWidth) / 2
                , (mParentWidget->height() - viewHeight) / 2
                , viewWidth
                , viewHeight);

        QWebEngineProfile* profile = UBApplication::webController->widgetProfile();
        mTrapFlashUi->webView->setPage(new QWebEnginePage(profile, this));

        connect(mTrapFlashUi->flashCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectFlash(int)));
        connect(mTrapFlashUi->widgetNameLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(text_Changed(const QString &)));
        connect(mTrapFlashUi->widgetNameLineEdit, SIGNAL(textEdited(const QString &)), this, SLOT(text_Edited(const QString &)));
        connect(mTrapFlashUi->createWidgetButton, SIGNAL(clicked(bool)), this, SLOT(createWidget()));
        connect(mTrapFlashUi->webView, &QWebEngineView::loadFinished, [this](){
            mTrapFlashUi->webView->update();
        });

    }

    mTrapFlashDialog->show();
}

void UBTrapFlashController::text_Changed(const QString &newText)
{
    QString new_text = newText;

#ifdef Q_OS_WIN // Defined on Windows.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_OS_OSX // Defined on Mac OS X.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_OS_LINUX // Defined on X11.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    QRegExp regExp("[<>:\"/\\\\|?*]");
#endif

    if(new_text.indexOf(regExp) > -1)
    {
        new_text.remove(regExp);
        mTrapFlashUi->widgetNameLineEdit->setText(new_text);
        QToolTip::showText(mTrapFlashUi->widgetNameLineEdit->mapToGlobal(QPoint()), "Application name can`t contain any of the following characters:\r\n"+illegalCharList);
    }
}

void UBTrapFlashController::text_Edited(const QString &newText)
{
    Q_UNUSED(newText);
}

void UBTrapFlashController::hideTrapFlash()
{
    if (mTrapFlashDialog)
    {
        mTrapFlashDialog->hide();
    }
}


void UBTrapFlashController::updateListOfFlashes(const QList<UBEmbedContent>& pAllContent)
{
    if (mTrapFlashDialog)
    {
        mAvailableContent = pAllContent;
        disconnect(mTrapFlashUi->flashCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectFlash(int)));
        mTrapFlashUi->flashCombobox->clear();
        mTrapFlashUi->flashCombobox->addItem(tr("Whole page"));

        for (const UBEmbedContent& wrapper : pAllContent)
        {
            mTrapFlashUi->flashCombobox->addItem(widgetNameForObject(wrapper));
        }

        connect(mTrapFlashUi->flashCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(selectFlash(int)));
        selectFlash(mTrapFlashUi->flashCombobox->currentIndex());
    }
}


void UBTrapFlashController::selectFlash(int pFlashIndex)
{
    if (pFlashIndex == 0)
    {
        mTrapFlashUi->webView->setHtml(generateFullPageHtml(mCurrentWebFrame->url(), "", false));
        mCurrentWebFrame->page()->runJavaScript("window.document.title", [this](const QVariant& res){
            mTrapFlashUi->widgetNameLineEdit->setText(res.toString().trimmed());
        });
    }
    else if (pFlashIndex > 0 && pFlashIndex <= mAvailableContent.size())
    {
        UBEmbedContent currentObject = mAvailableContent.at(pFlashIndex - 1);
        mTrapFlashUi->webView->setHtml(generateHtml(currentObject, "", false));
        mTrapFlashUi->widgetNameLineEdit->setText(widgetNameForObject(currentObject));
    }
}


void UBTrapFlashController::createWidget()
{
    int selectedIndex = mTrapFlashUi->flashCombobox->currentIndex();

    // create widget
    QString tempDir = UBFileSystemUtils::createTempDir("TrapFlashRendering");
    QDir widgetDir(tempDir + "/" + mTrapFlashUi->widgetNameLineEdit->text() + ".wgt");

    if (widgetDir.exists() && !UBFileSystemUtils::deleteDir(widgetDir.path()))
    {
        qWarning() << "Cannot delete " << widgetDir.path();
    }

    widgetDir.mkpath(widgetDir.path());
    UBEmbedContent content;

    if (selectedIndex == 0)
    {
        generateFullPageHtml(mCurrentWebFrame->url(), widgetDir.path(), true);
    }
    else
    {
        content = mAvailableContent.at(selectedIndex - 1);
        generateHtml(content, widgetDir.path(), true);
    }

    generateIcon(widgetDir.path());

    int heigth = content.height();
    int width = content.width();

    if (heigth && width)
    {
        generateConfig(width+20, heigth+20, widgetDir.path());
    }
    else
    {
        generateConfig(800, 600, widgetDir.path());
    }

    //generateDefaultPng(width, height, widgetDir.path());

    importWidgetInLibrary(widgetDir);

    UBFileSystemUtils::deleteDir(tempDir);

    QString freezedWidgetPath = UBPlatformUtils::applicationResourcesDirectory() + "/etc/freezedWidgetWrapper.html";
    mTrapFlashUi->webView->load(QUrl::fromLocalFile(freezedWidgetPath));

    mTrapFlashDialog->hide();
}


void UBTrapFlashController::importWidgetInLibrary(QDir pSourceDir)
{
    const QString userWidgetPath = UBSettings::settings()->userInteractiveDirectory() + "/" + tr("Web");
    QDir userWidgetDir(userWidgetPath);

    if (!userWidgetDir.exists())
    {
        userWidgetDir.mkpath(userWidgetPath);
    }

    QString widgetLibraryPath = userWidgetPath + "/" + mTrapFlashUi->widgetNameLineEdit->text() + ".wgt";
    QDir widgetLibraryDir(widgetLibraryPath);

    if (widgetLibraryDir.exists())
    {
        if (!UBFileSystemUtils::deleteDir(widgetLibraryDir.path()))
        {
            qWarning() << "Cannot delete old widget " << widgetLibraryDir.path();
        }
    }

    qDebug() << "Widget imported in path " << widgetLibraryPath;
    UBFileSystemUtils::copyDir(pSourceDir.path(), widgetLibraryPath);

    // also add to current scene
    if (UBApplication::applicationController)
        UBApplication::applicationController->showBoard();

    if (UBApplication::boardController &&
        UBApplication::boardController->activeScene())
    {
        UBApplication::boardController->activeScene()->addWidget(QUrl::fromLocalFile(widgetLibraryPath));
    }
}


void UBTrapFlashController::updateTrapFlashFromView(QWebEngineView *pCurrentWebFrame)
{
    if (pCurrentWebFrame && mTrapFlashDialog && mTrapFlashDialog->isVisible())
    {
        QList<UBEmbedContent> list = UBApplication::webController->getEmbeddedContent(pCurrentWebFrame);
        mCurrentWebFrame = pCurrentWebFrame;
        updateListOfFlashes(list);
    }
}


QString UBTrapFlashController::generateIcon(const QString& pDirPath)
{
    QDesktopWidget* desktop = QApplication::desktop();
    QPoint webViewPosition = mTrapFlashUi->webView->mapToGlobal(mTrapFlashUi->webView->pos());
    QSize webViewSize = mTrapFlashUi->webView->size();
    QPixmap capture = QPixmap::grabWindow(desktop->winId(), webViewPosition.x(), webViewPosition.y()
            , webViewSize.width() - 10, webViewSize.height() -10);

    QPixmap widgetIcon(75,75);
    widgetIcon.fill(Qt::transparent);

    QPainter painter(&widgetIcon);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(180,180,180)));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(0, 0, 75, 75, 10, 10);
    QPixmap icon = capture.scaled(65, 65);
    painter.drawPixmap(5,5,icon);

    QString iconFile = pDirPath + "/icon.png";
    widgetIcon.save(iconFile, "PNG");

    return iconFile;
}


void UBTrapFlashController::generateConfig(int pWidth, int pHeight, const QString& pDestinationPath)
{
    QFile configFile(pDestinationPath + "/" + "config.xml");

    if (configFile.exists())
    {
        configFile.remove(configFile.fileName());
    }

    if (!configFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open file " << configFile.fileName();
        return;
    }

    QTextStream out(&configFile);
    out.setCodec("UTF-8");
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    out << "<widget xmlns=\"http://www.w3.org/ns/widgets\"" << endl;
    out << "        xmlns:ub=\"http://uniboard.mnemis.com/widgets\"" << endl;
    out << "        identifier=\"http://uniboard.mnemis.com/" << mTrapFlashUi->widgetNameLineEdit->text() << "\"" <<endl;

    out << "        version=\"1.0\"" << endl;
    out << "        width=\"" << pWidth << "\"" << endl;
    out << "        height=\"" << pHeight << "\"" << endl;
    out << "        ub:resizable=\"true\">" << endl;

    out << "    <name>" << mTrapFlashUi->widgetNameLineEdit->text() << "</name>" << endl;
    out << "    <content src=\"" << mTrapFlashUi->widgetNameLineEdit->text() << ".html\"/>" << endl;

    out << "</widget>" << endl;


    configFile.close();
}


QString UBTrapFlashController::generateFullPageHtml(const QUrl &url, const QString& pDirPath, bool pGenerateFile)
{
    if (mCurrentWebFrame->url().isEmpty())
    {
        return "";
    }

    QString htmlContentString;

//  htmlContentString += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\r\n";
    htmlContentString += "<!DOCTYPE html>\r\n";
    htmlContentString += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\r\n";
    htmlContentString += "  <head>\r\n";
    htmlContentString += "    <meta http-equiv=\"refresh\" content=\"0; " + url.toString() + "\">\r\n";
    htmlContentString += "  </head>\r\n";
    htmlContentString += "  <body>\r\n";
    htmlContentString += "    Redirect to target...\r\n";
    htmlContentString += "  </body>\r\n";
    htmlContentString += "</html>\r\n";

    if (!pGenerateFile)
    {
        return htmlContentString;
    }
    else
    {
        QString fileName = mTrapFlashUi->widgetNameLineEdit->text();
        const QString fullHtmlFileName = pDirPath + "/" + fileName + ".html";
        QDir dir(pDirPath);
        if (!dir.exists())
        {
            dir.mkpath(dir.path());
        }
        QFile widgetHtmlFile(fullHtmlFileName);
        if (widgetHtmlFile.exists())
        {
            widgetHtmlFile.remove(widgetHtmlFile.fileName());
        }
        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return "";
        }
        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
}


QString UBTrapFlashController::generateHtml(const UBEmbedContent& pObject,
        const QString& pDirPath, bool pGenerateFile)
{
    if (pObject.type() == UBEmbedType::IFRAME)
    {
        return generateFullPageHtml(pObject.url(), pDirPath, pGenerateFile);
    }

    QString htmlContentString;

    htmlContentString += "<!DOCTYPE html>\r\n";
//    htmlContentString += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n";
    htmlContentString += "<html>\n";
    htmlContentString += "<head>\n";
    htmlContentString += "    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n";
    htmlContentString += "</head>\n";

    if (!pGenerateFile)
    {
        htmlContentString += "<body bgcolor=\"rgb(180,180,180)\">\n";
    }
    else
    {
        htmlContentString += "<body>\n";
    }

    htmlContentString += "        <div align='center'>\n";
    htmlContentString += pObject.html();
    htmlContentString += "        </div>\n";
    htmlContentString += "</body>\n";
    htmlContentString += "</html>\n";

    if (!pGenerateFile)
    {
            return htmlContentString;
    }
    else
    {
        QString fileName = mTrapFlashUi->widgetNameLineEdit->text();
        const QString fullHtmlFileName = pDirPath + "/" + fileName + ".html";
        QDir dir(pDirPath);

        if (!dir.exists())
        {
            dir.mkpath(dir.path());
        }

        QFile widgetHtmlFile(fullHtmlFileName);

        if (widgetHtmlFile.exists())
        {
            widgetHtmlFile.remove(widgetHtmlFile.fileName());
        }

        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return "";
        }

        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
}

QString UBTrapFlashController::widgetNameForObject(const UBEmbedContent& pObject)
{
    QString result;

    if (!pObject.title().isEmpty())
    {
        result = pObject.title();
    }
    else if (pObject.url().isValid())
    {
        QUrl url = pObject.url();
        result = url.path();
        int lastSlashIndex = result.lastIndexOf("/");
        result = result.right(result.length() - lastSlashIndex);
    }

    result = UBFileSystemUtils::cleanName(result);

    return result;
}

