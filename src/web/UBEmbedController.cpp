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




#include "UBEmbedController.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "core/UBApplication.h"

#include "domain/UBGraphicsScene.h"

#include "frameworks/UBFileSystemUtils.h"

#include "web/UBWebController.h"
#include "web/simplebrowser/webpage.h"

#include "ui_trapFlash.h"

#include "core/memcheck.h"

UBEmbedController::UBEmbedController(QWidget* parent)
    : QObject(parent)
    , mTrapFlashUi(nullptr)
    , mTrapDialog(nullptr)
    , mParentWidget(parent)
{
    // NOOP
}


UBEmbedController::~UBEmbedController()
{
    // NOOP
}


void UBEmbedController::updateListOfEmbeddableContent(const QList<UBEmbedContent>& pAllContent)
{
    if (mTrapDialog && mTrapDialog->isVisible())
    {
        mAvailableContent = pAllContent;
        disconnect(mTrapFlashUi->flashCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UBEmbedController::selectFlash);
        mTrapFlashUi->flashCombobox->clear();
        mTrapFlashUi->flashCombobox->addItem(tr("Whole page"));

        for (const UBEmbedContent& wrapper : pAllContent)
        {
            mTrapFlashUi->flashCombobox->addItem(widgetNameForObject(wrapper));
        }

        connect(mTrapFlashUi->flashCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UBEmbedController::selectFlash);
        selectFlash(mTrapFlashUi->flashCombobox->currentIndex());
    }
}


void UBEmbedController::showEmbedDialog()
{
    if (!mTrapDialog)
    {
        Qt::WindowFlags flag = Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint ;
        flag &= ~ Qt::WindowContextHelpButtonHint;
        flag &= ~ Qt::WindowMinimizeButtonHint;

        mTrapDialog = new QDialog(mParentWidget, flag);
        mTrapFlashUi = new Ui::trapFlashDialog();
        mTrapFlashUi->setupUi(mTrapDialog);

        int viewWidth = mParentWidget->width() / 2;
        int viewHeight = mParentWidget->height() * 2. / 3.;
        mTrapDialog->resize(viewWidth, viewHeight);

        QWebEngineProfile* profile = UBApplication::webController->webProfile();
        mTrapFlashUi->webView->setPage(new WebPage(profile, this));

        connect(mTrapFlashUi->flashCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &UBEmbedController::selectFlash);
        connect(mTrapFlashUi->widgetNameLineEdit, &QLineEdit::textChanged, this, &UBEmbedController::textChanged);
        connect(mTrapFlashUi->createWidgetButton, &QPushButton::clicked, this, &UBEmbedController::createWidget);
        connect(mTrapFlashUi->webView, &QWebEngineView::loadFinished, this, [this](){
            mTrapFlashUi->webView->update();
        });
    }

    mTrapDialog->show();
}


void UBEmbedController::hideEmbedDialog() const
{
    if (mTrapDialog)
    {
        mTrapDialog->hide();
    }
}


void UBEmbedController::pageUrlChanged(const QUrl& url)
{
    mUrl = url;
}


void UBEmbedController::pageTitleChanged(const QString& title)
{
    mPageTitle = title;
}


void UBEmbedController::textChanged(const QString& newText)
{
    QString new_text = newText;

#ifdef Q_OS_WIN // Defined on Windows.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    static const QRegularExpression regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_OS_OSX // Defined on Mac OS X.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    static const QRegularExpression regExp("[<>:\"/\\\\|?*]");
#endif

#ifdef Q_OS_LINUX // Defined on X11.
    QString illegalCharList("      < > : \" / \\ | ? * ");
    static const QRegularExpression regExp("[<>:\"/\\\\|?*]");
#endif

    if (new_text.indexOf(regExp) > -1)
    {
        new_text.remove(regExp);
        mTrapFlashUi->widgetNameLineEdit->setText(new_text);
        QToolTip::showText(mTrapFlashUi->widgetNameLineEdit->mapToGlobal(QPoint()), tr("Application name can`t contain any of the following characters:\r\n") + illegalCharList);
    }
}


void UBEmbedController::selectFlash(int pFlashIndex)
{
    if (pFlashIndex == 0)
    {
        mTrapFlashUi->webView->setHtml(generateFullPageHtml(mUrl));
        mTrapFlashUi->widgetNameLineEdit->setText(mPageTitle.trimmed());
    }
    else if (pFlashIndex > 0 && pFlashIndex <= mAvailableContent.size())
    {
        UBEmbedContent currentObject = mAvailableContent.at(pFlashIndex - 1);
        mTrapFlashUi->webView->setHtml(generateHtml(currentObject));
        mTrapFlashUi->widgetNameLineEdit->setText(widgetNameForObject(currentObject));
    }
}


void UBEmbedController::createWidget()
{
    int selectedIndex = mTrapFlashUi->flashCombobox->currentIndex();

    // create widget
    QString tempDir = UBFileSystemUtils::createTempDir("EmbedRendering");
    QDir widgetDir(tempDir + "/" + mTrapFlashUi->widgetNameLineEdit->text() + ".wgt");

    if (widgetDir.exists() && !UBFileSystemUtils::deleteDir(widgetDir.path()))
    {
        qWarning() << "Cannot delete " << widgetDir.path();
    }

    widgetDir.mkpath(widgetDir.path());
    UBEmbedContent content;

    if (selectedIndex == 0)
    {
        generateFullPageHtml(mUrl, widgetDir.path(), true);
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

    importWidgetInLibrary(widgetDir);

    UBFileSystemUtils::deleteDir(tempDir);

    mTrapDialog->hide();
}


void UBEmbedController::importWidgetInLibrary(const QDir& pSourceDir) const
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
    {
        UBApplication::applicationController->showBoard();
    }

    if (UBApplication::boardController &&
        UBApplication::boardController->activeScene())
    {
        UBApplication::boardController->activeScene()->addWidget(QUrl::fromLocalFile(widgetLibraryPath));
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
}


QString UBEmbedController::generateIcon(const QString& pDirPath) const
{
    // render web page to QPixmap
    QPixmap capture(mTrapFlashUi->webView->size());
    QPainter p(&capture);
    mTrapFlashUi->webView->render(&p);

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


void UBEmbedController::generateConfig(int pWidth, int pHeight, const QString& pDestinationPath) const
{
    QFile configFile(pDestinationPath + "/" + "config.xml");

    if (configFile.exists())
    {
        configFile.remove();
    }

    if (!configFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open file " << configFile.fileName();
        return;
    }

    QTextStream out(&configFile);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    out.setCodec("UTF-8");
#endif
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << '\n';
    out << "<widget xmlns=\"http://www.w3.org/ns/widgets\"" << '\n';
    out << "        xmlns:ub=\"http://uniboard.mnemis.com/widgets\"" << '\n';
    out << "        id=\"http://uniboard.mnemis.com/" << mTrapFlashUi->widgetNameLineEdit->text() << "\"" <<'\n';

    out << "        version=\"2.0\"" << '\n';
    out << "        width=\"" << pWidth << "\"" << '\n';
    out << "        height=\"" << pHeight << "\"" << '\n';
    out << "        ub:resizable=\"true\">" << '\n';

    out << "    <name>" << mTrapFlashUi->widgetNameLineEdit->text() << "</name>" << '\n';
    out << "    <content src=\"" << mTrapFlashUi->widgetNameLineEdit->text() << ".html\"/>" << '\n';

    out << "</widget>" << '\n';


    configFile.close();
}


QString UBEmbedController::generateFullPageHtml(const QUrl& url, const QString& pDirPath, bool pGenerateFile) const
{
    if (url.isEmpty())
    {
        return QString();
    }

    QString htmlContentString;

    htmlContentString += "<!DOCTYPE html>\n";
    htmlContentString += "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
    htmlContentString += "  <head>\n";
    htmlContentString += "    <meta http-equiv=\"refresh\" content=\"0; " + url.toString() + "\">\n";
    htmlContentString += "  </head>\n";
    htmlContentString += "  <body>\n";
    htmlContentString += "    Redirect to target...\n";
    htmlContentString += "  </body>\n";
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
            widgetHtmlFile.remove();
        }

        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return QString();
        }

        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
}


QString UBEmbedController::generateHtml(const UBEmbedContent& pObject,
        const QString& pDirPath, bool pGenerateFile) const
{
    if (pObject.type() == UBEmbedType::IFRAME)
    {
        return generateFullPageHtml(pObject.url(), pDirPath, pGenerateFile);
    }

    QString htmlContentString;

    htmlContentString += "<!DOCTYPE html>\n";
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
            widgetHtmlFile.remove();
        }

        if (!widgetHtmlFile.open(QIODevice::WriteOnly))
        {
            qWarning() << "cannot open file " << widgetHtmlFile.fileName();
            return QString();
        }

        QTextStream out(&widgetHtmlFile);
        out << htmlContentString;

        widgetHtmlFile.close();
        return widgetHtmlFile.fileName();
    }
}

QString UBEmbedController::widgetNameForObject(const UBEmbedContent& pObject) const
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

