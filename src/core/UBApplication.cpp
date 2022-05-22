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




#include "UBApplication.h"

#include <QtGui>
#include <QtXml>
#include <QFontDatabase>
#include <QStyleFactory>

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBStringUtils.h"

#include "UBSettings.h"
#include "UBSetting.h"
#include "UBPersistenceManager.h"
#include "UBDocumentManager.h"
#include "UBPreferencesController.h"
#include "UBIdleTimer.h"
#include "UBApplicationController.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"
#include "web/UBWebController.h"

#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"

#include "gui/UBMainWindow.h"
#include "gui/UBResources.h"
#include "gui/UBThumbnailWidget.h"

#include "ui_mainWindow.h"

#include "frameworks/UBCryptoUtils.h"
#include "tools/UBToolsManager.h"

#include "UBDisplayManager.h"
#include "core/memcheck.h"

QPointer<QUndoStack> UBApplication::undoStack;

UBDisplayManager* UBApplication::displayManager = nullptr;
UBApplicationController* UBApplication::applicationController = 0;
UBBoardController* UBApplication::boardController = 0;
UBWebController* UBApplication::webController = 0;
UBDocumentController* UBApplication::documentController = 0;

UBMainWindow* UBApplication::mainWindow = 0;

const QString UBApplication::mimeTypeUniboardDocument = QString("application/vnd.mnemis-uniboard-document");
const QString UBApplication::mimeTypeUniboardPage = QString("application/vnd.mnemis-uniboard-page");
const QString UBApplication::mimeTypeUniboardPageItem =  QString("application/vnd.mnemis-uniboard-page-item");
const QString UBApplication::mimeTypeUniboardPageThumbnail = QString("application/vnd.mnemis-uniboard-thumbnail");

QString UBApplication::fileToOpen = "";

#if defined(Q_OS_OSX) || defined(Q_OS_LINUX)
bool bIsMinimized = false;
#endif

QObject* UBApplication::staticMemoryCleaner = 0;


UBApplication::UBApplication(const QString &id, int &argc, char **argv) : SingleApplication(argc, argv)
  , mPreferencesController(NULL)
  , mApplicationTranslator(NULL)
  , mQtGuiTranslator(NULL)
{
    Q_UNUSED(id)
    staticMemoryCleaner = new QObject(0); // deleted in UBApplication destructor

    setOrganizationName("Open Education Foundation");
    setOrganizationDomain("oe-f.org");
    setApplicationName("OpenBoard");

    QString version = UBVERSION;
    if(version.endsWith("."))
        version = version.left(version.length()-1);
    setApplicationVersion(version);

    QStringList args = arguments();

    mIsVerbose = args.contains("-v")
        || args.contains("-verbose")
        || args.contains("verbose")
        || args.contains("-log")
        || args.contains("log");


    setupTranslators(args);

    UBResources::resources();

    if (!undoStack)
        undoStack = new QUndoStack(staticMemoryCleaner);

    UBPlatformUtils::init();

    UBSettings *settings = UBSettings::settings();

    connect(settings->appToolBarPositionedAtTop, SIGNAL(changed(QVariant)), this, SLOT(toolBarPositionChanged(QVariant)));
    connect(settings->appToolBarDisplayText, SIGNAL(changed(QVariant)), this, SLOT(toolBarDisplayTextChanged(QVariant)));
    updateProtoActionsState();

#ifndef Q_OS_OSX
    setWindowIcon(QIcon(":/images/OpenBoard.png"));
#endif

    setStyle("fusion");

    QString css = UBFileSystemUtils::readTextFile(UBPlatformUtils::applicationResourcesDirectory() + "/etc/"+ qApp->applicationName()+".css");
    if (css.length() > 0)
        setStyleSheet(css);

    QApplication::setStartDragDistance(8); // default is 4, and is a bit small for tablets

    installEventFilter(this);

}


UBApplication::~UBApplication()
{
    UBPlatformUtils::destroy();

    UBFileSystemUtils::deleteAllTempDirCreatedDuringSession();

    delete mainWindow;
    mainWindow = 0;

    UBPersistenceManager::destroy();

    UBDownloadManager::destroy();

    UBDrawingController::destroy();

    UBSettings::destroy();

    UBCryptoUtils::destroy();

    UBToolsManager::destroy();

    if(mApplicationTranslator != NULL){
        delete mApplicationTranslator;
        mApplicationTranslator = NULL;
    }
    if(mQtGuiTranslator!=NULL){
        delete mQtGuiTranslator;
        mQtGuiTranslator = NULL;
    }

    delete staticMemoryCleaner;
    staticMemoryCleaner = 0;
}

QString UBApplication::checkLanguageAvailabilityForSankore(QString &language)
{
    QStringList availableTranslations = UBPlatformUtils::availableTranslations();

    if(availableTranslations.contains(language,Qt::CaseInsensitive))
        return language;
    else{
        if(language.length() > 2){
            QString shortLanguageCode = language.left(2);

            foreach (const QString &str, availableTranslations) {
                       if (str.contains(shortLanguageCode))
                           return shortLanguageCode;
                   }

        }
    }
    return QString("");
}

void UBApplication::setupTranslators(QStringList args)
{
    QString forcedLanguage("");
    if(args.contains("-lang"))
        forcedLanguage=args.at(args.indexOf("-lang") + 1);
// TODO claudio: this has been commented because some of the translation seem to be loaded at this time
//               especially tools name. This is a workaround and we have to be able to load settings without
//               impacting the translations
//    else{
//        QString setLanguage = UBSettings::settings()->appPreferredLanguage->get().toString();
//        if(!setLanguage.isEmpty())
//            forcedLanguage = setLanguage;
//    }

    QString language("");

    if(!forcedLanguage.isEmpty())
        language = checkLanguageAvailabilityForSankore(forcedLanguage);

    if(language.isEmpty()){
        QString systemLanguage = UBPlatformUtils::systemLanguage();
        language = checkLanguageAvailabilityForSankore(systemLanguage);
    }

    if(language.isEmpty()){
        language = "en_US";
        //fallback if no translation are available
    }
    else{
        mApplicationTranslator = new QTranslator(this);
        mQtGuiTranslator = new QTranslator(this);
        mApplicationTranslator->load(UBPlatformUtils::translationPath(QString("OpenBoard_"),language));
        installTranslator(mApplicationTranslator);

        QString qtGuiTranslationPath = UBPlatformUtils::translationPath("qt_", language);


        if(!QFile(qtGuiTranslationPath).exists()){
            qtGuiTranslationPath = UBPlatformUtils::translationPath("qt_", language.left(2));
            if(!QFile(qtGuiTranslationPath).exists())
                qtGuiTranslationPath = "";
        }

        QLocale locale(language);
        QString qtTranslationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        bool loaded = false;

        if (qtGuiTranslationPath.isEmpty())
        {
            loaded = mQtGuiTranslator->load(locale, "qt", "_", qtTranslationPath, ".qm");
        }
        else
        {
            loaded = mQtGuiTranslator->load(qtGuiTranslationPath);
        }

        if (loaded)
        {
            qDebug() << "Loaded Qt translations";
            installTranslator(mQtGuiTranslator);
        }
        else
        {
            qDebug() << "Qt gui translation in " << language << " is not available";
        }

        // QtWebEngine translations
        QTranslator* qtWebEngineTranslator = new QTranslator(this);
        loaded = qtWebEngineTranslator->load(locale, "qtwebengine", "_", qtTranslationPath, ".qm");

        if (loaded)
        {
            qDebug() << "Loaded QWebengine translations";
            installTranslator(qtWebEngineTranslator);
        }
        else
        {
            qDebug() << "Qt WebEngine translation in " << language << " is not available";
        }
    }

    QLocale::setDefault(QLocale(language));
    qDebug() << "Running application in:" << language;
}

int UBApplication::exec(const QString& pFileToImport)
{
    QPixmapCache::setCacheLimit(1024 * 100);

    /* TODO remove, will be in the default directories
        QString webDbPath = UBSettings::userDataDirectory() + "/web-databases";
        QDir webDbDir(webDbPath);
        if (!webDbDir.exists(webDbPath))
            webDbDir.mkpath(webDbPath);
     */

    displayManager = new UBDisplayManager(staticMemoryCleaner);

    if (UBSettings::settings()->appRunInWindow->get().toBool()) {
        mainWindow = new UBMainWindow(0,
                Qt::Window |
                Qt::WindowCloseButtonHint |
                Qt::WindowMinimizeButtonHint |
                Qt::WindowMaximizeButtonHint |
                Qt::WindowShadeButtonHint
        ); // deleted by application destructor
    } else {
        mainWindow = new UBMainWindow(0, Qt::FramelessWindowHint); // deleted by application destructor
    }

    mainWindow->setAttribute(Qt::WA_NativeWindow, true);

    mainWindow->actionCopy->setShortcuts(QKeySequence::Copy);
    mainWindow->actionPaste->setShortcuts(QKeySequence::Paste);
    mainWindow->actionCut->setShortcuts(QKeySequence::Cut);

    UBThumbnailUI::_private::initCatalog();

    connect(mainWindow->actionBoard, SIGNAL(triggered()), this, SLOT(showBoard()));
    connect(mainWindow->actionWeb, SIGNAL(triggered()), this, SLOT(showInternet()));
    connect(mainWindow->actionWeb, SIGNAL(triggered()), this, SLOT(stopScript()));
    connect(mainWindow->actionDocument, SIGNAL(triggered()), this, SLOT(showDocument()));
    connect(mainWindow->actionDocument, SIGNAL(triggered()), this, SLOT(stopScript()));
    connect(mainWindow->actionQuit, SIGNAL(triggered()), this, SLOT(closing()));
    connect(mainWindow, SIGNAL(closeEvent_Signal(QCloseEvent*)), this, SLOT(closeEvent(QCloseEvent*)));

    boardController = new UBBoardController(mainWindow);
    boardController->init();

    webController = new UBWebController(mainWindow);
    documentController = new UBDocumentController(mainWindow);

    UBDrawingController::drawingController()->setStylusTool((int)UBStylusTool::Pen);

    applicationController = new UBApplicationController(boardController->controlView(),
                                                        boardController->displayView(),
                                                        mainWindow,
                                                        staticMemoryCleaner,
                                                        boardController->paletteManager()->rightPalette());


    if (!UBApplication::fileToOpen.isEmpty())
    {
        if (!UBApplication::fileToOpen.endsWith("ubx"))
           applicationController->importFile(UBApplication::fileToOpen);
        else
            applicationController->showMessage(tr("Cannot open your UBX file directly. Please import it in Documents mode instead"), false);
    }

    connect(applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode)),
            boardController->paletteManager(), SLOT(slot_changeMainMode(UBApplicationController::MainMode)));

    connect(applicationController, SIGNAL(desktopMode(bool)),
            boardController->paletteManager(), SLOT(slot_changeDesktopMode(bool)));

    connect(applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode))
          , boardController,       SLOT(appMainModeChanged(UBApplicationController::MainMode)));

    connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), applicationController, SLOT(showDesktop(bool)));
    connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), this, SLOT(stopScript()));
#if defined(Q_OS_OSX) || defined(Q_OS_LINUX)
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), this, SLOT(showMinimized()));
#else
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), mainWindow, SLOT(showMinimized()));
#endif

    mPreferencesController = new UBPreferencesController(mainWindow);

    connect(mainWindow->actionPreferences, SIGNAL(triggered()), mPreferencesController, SLOT(show()));
    connect(mainWindow->actionCheckUpdate, SIGNAL(triggered()), applicationController, SLOT(checkUpdateRequest()));


    toolBarPositionChanged(UBSettings::settings()->appToolBarPositionedAtTop->get());

    bool bUseMultiScreen = UBSettings::settings()->appUseMultiscreen->get().toBool();
    mainWindow->actionMultiScreen->setChecked(bUseMultiScreen);
    connect(mainWindow->actionMultiScreen, SIGNAL(triggered(bool)), applicationController, SLOT(useMultiScreen(bool)));
    connect(mainWindow->actionWidePageSize, SIGNAL(triggered(bool)), boardController, SLOT(setWidePageSize(bool)));
    connect(mainWindow->actionRegularPageSize, SIGNAL(triggered(bool)), boardController, SLOT(setRegularPageSize(bool)));

    connect(mainWindow->actionCut, SIGNAL(triggered()), applicationController, SLOT(actionCut()));
    connect(mainWindow->actionCopy, SIGNAL(triggered()), applicationController, SLOT(actionCopy()));
    connect(mainWindow->actionPaste, SIGNAL(triggered()), applicationController, SLOT(actionPaste()));

    applicationController->initScreenLayout(bUseMultiScreen);
    boardController->setupLayout();

    if (pFileToImport.length() > 0)
    {
        if (!pFileToImport.endsWith("ubx"))
            applicationController->importFile(pFileToImport);
        else
            applicationController->showMessage(tr("Cannot open your UBX file directly. Please import it in Documents mode instead"), false);
    }

    if (UBSettings::settings()->appStartMode->get().toInt())
        applicationController->showDesktop();
    else
        applicationController->showBoard();

    emit UBDrawingController::drawingController()->colorPaletteChanged();

    onScreenCountChanged(displayManager->numScreens());
    connect(displayManager, SIGNAL(availableScreenCountChanged(int)), this, SLOT(onScreenCountChanged(int)));
    return QApplication::exec();
}

void UBApplication::onScreenCountChanged(int newCount)
{
    mainWindow->actionMultiScreen->setEnabled(newCount > 1);
}

void UBApplication::showMinimized()
{
#ifdef Q_OS_OSX
    mainWindow->hide();
    bIsMinimized = true;
#elif defined(Q_OS_LINUX)
    mainWindow->showMinimized();
    bIsMinimized = true;
#endif

}


void UBApplication::startScript()
{
    this->boardController->freezeW3CWidgets(false);
}

void UBApplication::stopScript()
{
    this->boardController->freezeW3CWidgets(true);
}

void UBApplication::showBoard()
{
    applicationController->showBoard();
}

void UBApplication::showInternet()
{
    applicationController->showInternet();
    webController->showTabAtTop(true);
}

void UBApplication::showDocument()
{
    applicationController->showDocument();
}

int UBApplication::toolBarHeight()
{
    return mainWindow->boardToolBar->rect().height();
}


void UBApplication::toolBarPositionChanged(QVariant topOrBottom)
{
    Qt::ToolBarArea area;

    if (topOrBottom.toBool())
        area = Qt::TopToolBarArea;
    else
        area = Qt::BottomToolBarArea;

    mainWindow->addToolBar(area, mainWindow->boardToolBar);
    mainWindow->addToolBar(area, mainWindow->webToolBar);
    mainWindow->addToolBar(area, mainWindow->documentToolBar);

    webController->showTabAtTop(topOrBottom.toBool());

}


void UBApplication::toolBarDisplayTextChanged(QVariant display)
{
    Qt::ToolButtonStyle toolButtonStyle = display.toBool() ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly;
    mainWindow->boardToolBar->setToolButtonStyle(toolButtonStyle);
    mainWindow->webToolBar->setToolButtonStyle(toolButtonStyle);
    mainWindow->documentToolBar->setToolButtonStyle(toolButtonStyle);
}


void UBApplication::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    closing();
}

void UBApplication::closing()
{
    if (UBSettings::settings()->emptyTrashForOlderDocuments->get().toBool())
    {
        UBDocumentTreeModel *docModel = UBPersistenceManager::persistenceManager()->mDocumentTreeStructureModel;
        documentController->deleteDocumentsInFolderOlderThan(docModel->trashIndex(), UBSettings::settings()->emptyTrashDaysValue->get().toInt());
        if (docModel->hasChildren(docModel->trashIndex()))
            documentController->deleteEmptyFolders(docModel->trashIndex());
    }

    if (boardController)
        boardController->closing();

    if (applicationController)
        applicationController->closing();

    if (webController)
        webController->closing();

    UBSettings::settings()->closing();

    UBSettings::settings()->appToolBarPositionedAtTop->set(mainWindow->toolBarArea(mainWindow->boardToolBar) == Qt::TopToolBarArea);

    quit();
}


void UBApplication::showMessage(const QString& message, bool showSpinningWheel)
{
    if (applicationController)
        applicationController->showMessage(message, showSpinningWheel);
}


void UBApplication::setDisabled(bool disable)
{
    boardController->setDisabled(disable);
}


void UBApplication::decorateActionMenu(QAction* action)
{
    foreach(QWidget* menuWidget,  action->associatedWidgets())
    {
        QToolButton *tb = qobject_cast<QToolButton*>(menuWidget);

        if (tb && !tb->menu())
        {
            tb->setObjectName("ubButtonMenu");
            tb->setPopupMode(QToolButton::InstantPopup);
            QMenu* menu = new QMenu(mainWindow);

            QActionGroup* pageSizeGroup = new QActionGroup(mainWindow);
            pageSizeGroup->addAction(mainWindow->actionWidePageSize);
            pageSizeGroup->addAction(mainWindow->actionRegularPageSize);
            pageSizeGroup->addAction(mainWindow->actionCustomPageSize);

            QMenu* documentSizeMenu = menu->addMenu(QIcon(":/images/toolbar/pageSize.png"),tr("Page Size"));
            documentSizeMenu->addAction(mainWindow->actionWidePageSize);
            documentSizeMenu->addAction(mainWindow->actionRegularPageSize);
            documentSizeMenu->addAction(mainWindow->actionCustomPageSize);
            menu->addAction(mainWindow->actionCut);
            menu->addAction(mainWindow->actionCopy);
            menu->addAction(mainWindow->actionPaste);
            menu->addAction(mainWindow->actionHideApplication);
            menu->addAction(mainWindow->actionSleep);

            menu->addSeparator();
            menu->addAction(mainWindow->actionOpenTutorial);
            menu->addSeparator();
            menu->addAction(mainWindow->actionPreferences);
            menu->addAction(mainWindow->actionMultiScreen);
            if (!UBSettings::settings()->appHideCheckForSoftwareUpdate->get().toBool())
                menu->addAction(mainWindow->actionCheckUpdate);
            menu->addSeparator();

            menu->addAction(mainWindow->actionPodcast);
            mainWindow->actionPodcast->setText(tr("Podcast"));

            menu->addSeparator();
            menu->addAction(mainWindow->actionQuit);

            tb->setMenu(menu);
        }
    }
}


void UBApplication::updateProtoActionsState()
{
    if (mainWindow)
    {
        mainWindow->actionMultiScreen->setVisible(true);
    }

    foreach(QMenu* protoMenu, mProtoMenus)
        protoMenu->setVisible(true);

}


void UBApplication::insertSpaceToToolbarBeforeAction(QToolBar* toolbar, QAction* action, int width)
{
    QWidget* spacer = new QWidget();

    if (width >= 0){
        QHBoxLayout *layout = new QHBoxLayout();
        layout->addSpacing(width);
        spacer->setLayout(layout);
    }
    else
        spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    toolbar->insertWidget(action, spacer);
}


bool UBApplication::eventFilter(QObject *obj, QEvent *event)
{
    bool result = QObject::eventFilter(obj, event);

    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent *fileToOpenEvent = static_cast<QFileOpenEvent *>(event);

        UBPlatformUtils::setFrontProcess();

        if (applicationController)
        {
            if (!fileToOpenEvent->file().endsWith("ubx"))
                applicationController->importFile(fileToOpenEvent->file());
            else
                applicationController->showMessage(tr("Cannot open your UBX file directly. Please import it in Documents mode instead"), false);
        }
        else
        {
            //startup : progressdialog.exec() is called and fileOpenEvent is consumed too early
            // we store the file and will import it when the documents tree is ready

            UBApplication::fileToOpen = fileToOpenEvent->file();
            return true;
        }
    }

    if (event->type() == QEvent::TabletLeaveProximity)
    {
        if (boardController && boardController->controlView())
            boardController->controlView()->forcedTabletRelease();
    }


    if (event->type() == QEvent::ApplicationActivate)
    {
        boardController->controlView()->setMultiselection(false);

#if defined(Q_OS_OSX)
        if (bIsMinimized) {
            if (mainWindow->isHidden())
                mainWindow->show();
            bIsMinimized = false;
        }
#elif defined(Q_OS_LINUX)
        if (bIsMinimized) {
            bIsMinimized = false;
            UBPlatformUtils::showFullScreen(mainWindow);
        }
#endif
    }

    return result;
}


bool UBApplication::handleOpenMessage(const QString& pMessage)
{
    qDebug() << "received message" << pMessage;

    if (pMessage == UBSettings::appPingMessage)
    {
        qDebug() << "received ping";
        return true;
    }

    qDebug() << "importing file" << pMessage;

    UBApplication::applicationController->importFile(pMessage);

    return true;
}

void UBApplication::cleanup()
{
    if (applicationController) delete applicationController;
    if (boardController) delete boardController;
    if (webController) delete webController;
    if (documentController) delete documentController;

    applicationController = NULL;
    boardController = NULL;
    webController = NULL;
    documentController = NULL;
}

QString UBApplication::urlFromHtml(QString html)
{
    QString _html;
    QRegularExpression comments("\\<![ \r\n\t]*(--([^\\-]|[\r\n]|-[^\\-])*--[ \r\n\t]*)\\>");
    QString url;
    QDomDocument domDoc;

    //    We remove all the comments & CRLF of this html
    _html = html.remove(comments);
    domDoc.setContent(_html.remove(QRegularExpression("[\\0]")));
    QDomElement rootElem = domDoc.documentElement();

    //  QUICKFIX: Here we have to check rootElem. Sometimes it can be a <meta> tag
    //  In such a case we will not be able to retrieve the src value
    if(rootElem.tagName().toLower().contains("meta")){
        qDebug() << rootElem.firstChildElement().tagName();
        //  In that case we get the next element
        url = rootElem.firstChildElement().attribute("src");
    }else{
        url = rootElem.attribute("src");
    }

    return url;
}

bool UBApplication::isFromWeb(QString url)
{
    bool res = true;

    if( url.startsWith("openboardtool://") ||
        url.startsWith("file://") ||
        url.startsWith("/")){
        res = false;
    }

    return res;
}
