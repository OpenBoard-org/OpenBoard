/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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



#include "UBApplication.h"

#include <QtGui>
#include <QtWebKit>
#include <QtXml>
#include <QFontDatabase>

#if defined(Q_WS_MACX)
#include <Carbon/Carbon.h>
#endif

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

#include "adaptors/publishing/UBSvgSubsetRasterizer.h"

#include "ui_mainWindow.h"

#include "frameworks/UBCryptoUtils.h"
#include "tools/UBToolsManager.h"

#include "UBDisplayManager.h"
#include "core/memcheck.h"

QPointer<QUndoStack> UBApplication::undoStack;

UBApplicationController* UBApplication::applicationController = 0;
UBBoardController* UBApplication::boardController = 0;
UBWebController* UBApplication::webController = 0;
UBDocumentController* UBApplication::documentController = 0;

UBMainWindow* UBApplication::mainWindow = 0;

const QString UBApplication::mimeTypeUniboardDocument = QString("application/vnd.mnemis-uniboard-document");
const QString UBApplication::mimeTypeUniboardPage = QString("application/vnd.mnemis-uniboard-page");
const QString UBApplication::mimeTypeUniboardPageItem =  QString("application/vnd.mnemis-uniboard-page-item");
const QString UBApplication::mimeTypeUniboardPageThumbnail = QString("application/vnd.mnemis-uniboard-thumbnail");

#ifdef Q_WS_MAC
bool bIsMinimized = false;
#endif

QObject* UBApplication::staticMemoryCleaner = 0;

#if defined(Q_WS_MAC)
static OSStatus ub_appleEventProcessor(const AppleEvent *ae, AppleEvent *event, long handlerRefCon)
{
    Q_UNUSED(event);
    OSType aeID = typeWildCard;
    OSType aeClass = typeWildCard;

    AEGetAttributePtr(ae, keyEventClassAttr, typeType, 0, &aeClass, sizeof(aeClass), 0);
    AEGetAttributePtr(ae, keyEventIDAttr, typeType, 0, &aeID, sizeof(aeID), 0);

    if (aeClass == kCoreEventClass && aeID == kAEReopenApplication)
    {
        // User clicked on Uniboard in the Dock
        ((UBApplicationController*)handlerRefCon)->hideDesktop();
        return noErr;
    }

    return eventNotHandledErr;
}
#endif


UBApplication::UBApplication(const QString &id, int &argc, char **argv) : QtSingleApplication(id, argc, argv)
  , mPreferencesController(NULL)
  , mApplicationTranslator(NULL)
  , mQtGuiTranslator(NULL)
{

    staticMemoryCleaner = new QObject(0); // deleted in UBApplication destructor

    setOrganizationName("Open Education Foundation");
    setOrganizationDomain("oe-f.org");
    setApplicationName("OpenBoard");

    setApplicationVersion(UBVERSION);

#if defined(Q_WS_MAC) && !defined(QT_NO_DEBUG)
    CFStringRef shortVersion = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(), CFSTR("CFBundleShortVersionString"));
    const char *version = CFStringGetCStringPtr(shortVersion, kCFStringEncodingMacRoman);
    Q_ASSERT(version);
    setApplicationVersion(version);
#endif

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

#ifndef Q_WS_MAC
    setWindowIcon(QIcon(":/images/OpenBoard.png"));
#endif

    setStyle(new UBStyle()); // Style is owned and deleted by the application

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
            if(availableTranslations.contains(shortLanguageCode,Qt::CaseInsensitive))
                return shortLanguageCode;
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

        if(!qtGuiTranslationPath.isEmpty()){
            mQtGuiTranslator->load(qtGuiTranslationPath);
            installTranslator(mQtGuiTranslator);
        }
        else
            qDebug() << "Qt gui translation in " << language << " is not available";
    }

    QLocale::setDefault(QLocale(language));
    qDebug() << "Running application in:" << language;
}

int UBApplication::exec(const QString& pFileToImport)
{
    QPixmapCache::setCacheLimit(1024 * 100);

    QString webDbPath = UBSettings::userDataDirectory() + "/web-databases";
    QDir webDbDir(webDbPath);
    if (!webDbDir.exists(webDbPath))
        webDbDir.mkpath(webDbPath);

    QWebSettings::setIconDatabasePath(webDbPath);
    QWebSettings::setOfflineStoragePath (webDbPath);

    QWebSettings *gs = QWebSettings::globalSettings();
    gs->setAttribute(QWebSettings::JavaEnabled, true);
    gs->setAttribute(QWebSettings::PluginsEnabled, true);
    gs->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    gs->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    gs->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    gs->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    gs->setAttribute(QWebSettings::DnsPrefetchEnabled, true);


    mainWindow = new UBMainWindow(0, Qt::FramelessWindowHint); // deleted by application destructor
    mainWindow->setAttribute(Qt::WA_NativeWindow, true);

    mainWindow->actionCopy->setShortcuts(QKeySequence::Copy);
    mainWindow->actionPaste->setShortcuts(QKeySequence::Paste);
    mainWindow->actionCut->setShortcuts(QKeySequence::Cut);

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


    connect(applicationController, SIGNAL(mainModeChanged(UBApplicationController::MainMode)),
            boardController->paletteManager(), SLOT(slot_changeMainMode(UBApplicationController::MainMode)));

    connect(applicationController, SIGNAL(desktopMode(bool)),
            boardController->paletteManager(), SLOT(slot_changeDesktopMode(bool)));



    connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), applicationController, SLOT(showDesktop(bool)));
    connect(mainWindow->actionDesktop, SIGNAL(triggered(bool)), this, SLOT(stopScript()));
#ifndef Q_WS_MAC
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), mainWindow, SLOT(showMinimized()));
#else
    connect(mainWindow->actionHideApplication, SIGNAL(triggered()), this, SLOT(showMinimized()));
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
        UBApplication::applicationController->importFile(pFileToImport);

#if defined(Q_WS_MAC)
    static AEEventHandlerUPP ub_proc_ae_handlerUPP = AEEventHandlerUPP(ub_appleEventProcessor);
    AEInstallEventHandler(kCoreEventClass, kAEReopenApplication, ub_proc_ae_handlerUPP, SRefCon(UBApplication::applicationController), true);
#endif

    if (UBSettings::settings()->appStartMode->get().toInt())
        applicationController->showDesktop();
    else
        applicationController->showBoard();

    onScreenCountChanged(1);
    connect(desktop(), SIGNAL(screenCountChanged(int)), this, SLOT(onScreenCountChanged(int)));
    return QApplication::exec();
}

void UBApplication::onScreenCountChanged(int newCount)
{
    Q_UNUSED(newCount);
    UBDisplayManager displayManager;
    mainWindow->actionMultiScreen->setEnabled(displayManager.numScreens() > 1);
}

#ifdef Q_WS_MAC
void UBApplication::showMinimized()
{
    mainWindow->hide();
    bIsMinimized = true;
}

#endif

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
            menu->addAction(mainWindow->actionPreferences);
            menu->addAction(mainWindow->actionMultiScreen);
            // SANKORE-48: Hide the check update action if the setting
            // EnableAutomaticSoftwareUpdates is false in Uniboard.config
            if(UBSettings::settings()->appEnableAutomaticSoftwareUpdates->get().toBool())
                menu->addAction(mainWindow->actionCheckUpdate);
            else
                mainWindow->actionCheckUpdate->setEnabled(false);

            menu->addSeparator();

#ifndef Q_WS_X11 // No Podcast on Linux yet
            menu->addAction(mainWindow->actionPodcast);
            mainWindow->actionPodcast->setText(tr("Podcast"));
#endif

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

#if defined(Q_WS_MACX)
        ProcessSerialNumber psn;
        GetCurrentProcess(&psn);
        SetFrontProcess(&psn);
#endif

        applicationController->importFile(fileToOpenEvent->file());
    }

    if (event->type() == QEvent::TabletLeaveProximity)
    {
        if (boardController && boardController->controlView())
            boardController->controlView()->forcedTabletRelease();
    }


    if (event->type() == QEvent::ApplicationActivate)
    {
        boardController->controlView()->setMultiselection(false);
    }

#ifdef Q_WS_MAC
    if (bIsMinimized && event->type() == QEvent::ApplicationActivate){
        if (mainWindow->isHidden()) mainWindow->show();
        bIsMinimized = false;
    }
#endif
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

void UBStyle::drawItemText(QPainter *painter, const QRect &rect, int alignment, const QPalette &pal,
                          bool enabled, const QString& text, QPalette::ColorRole textRole) const
{
    if (text.isEmpty())
        return;

    QPen savedPen;
    if (textRole != QPalette::NoRole)
    {
        savedPen = painter->pen();
        painter->setPen(QPen(pal.brush(textRole), savedPen.widthF()));
    }

    if (!enabled)
    {
        QPen pen = painter->pen();
        QColor half = pen.color();

        half.setRed(half.red() / 2);
        half.setGreen(half.green() / 2);
        half.setBlue(half.blue() / 2);

        painter->setPen(half);
        painter->drawText(rect, alignment, text);
        painter->setPen(pen);
    }

    painter->drawText(rect, alignment, text);

    if (textRole != QPalette::NoRole)
        painter->setPen(savedPen);
}

QString UBApplication::urlFromHtml(QString html)
{
    QString _html;
    QRegExp comments("\\<![ \r\n\t]*(--([^\\-]|[\r\n]|-[^\\-])*--[ \r\n\t]*)\\>");
    QString url;
    QDomDocument domDoc;

    //    We remove all the comments & CRLF of this html
    _html = html.remove(comments);
    domDoc.setContent(_html.remove(QRegExp("[\\0]")));
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

    if( url.startsWith("uniboardTool://") ||
        url.startsWith("file://") ||
        url.startsWith("/")){
        res = false;
    }

    return res;
}
