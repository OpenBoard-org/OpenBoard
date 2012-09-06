/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QColor>
#include <QLabel>
#include <QDebug>
#include <QUrl>
#include <QWebSettings>
#include <QApplication>
#include <QDomElement>
#include <QWebFrame>

#include "UBTeacherGuideWidgetsTools.h"

#include "core/UBPersistenceManager.h"
#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "domain/UBGraphicsWidgetItem.h"

#include "globals/UBGlobals.h"

#include "frameworks/UBFileSystemUtils.h"

#include "customWidgets/UBMediaWidget.h"

#include "core/memcheck.h"


/***************************************************************************
 *                             class    UBAddItem                          *
 ***************************************************************************/
UBAddItem::UBAddItem(const QString &string, int addSubItemWidgetType, QTreeWidget* parent): QTreeWidgetItem(parent)
{
    setIcon(0,QIcon(":images/increase.svg"));
    setText(0,string);
    setData(0,Qt::UserRole,QVariant(addSubItemWidgetType));
    setData(1,Qt::UserRole,QVariant(addSubItemWidgetType));
    setData(0,Qt::BackgroundRole,QVariant(QColor(200,200,200)));
    setData(1,Qt::BackgroundRole,QVariant(QColor(200,200,200)));
    setData(0,Qt::FontRole,QVariant(QFont(QApplication::font().family(),12)));
}

UBAddItem::~UBAddItem()
{
    //NOOP
}

/***************************************************************************
 *                      class    UBTGActionWidget                          *
 ***************************************************************************/
UBTGActionWidget::UBTGActionWidget(QTreeWidgetItem* widget, QWidget* parent, const char* name) : QWidget(parent)
  , mpLayout(NULL)
  , mpOwner(NULL)
  , mpTask(NULL)
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout(this);
    mpOwner = new QComboBox(this);
    mpOwner->setObjectName("DockPaletteWidgetComboBox");
    mpOwner->setMinimumHeight(22);
    QStringList qslOwner;
    qslOwner << tr("Teacher") << tr("Student");
    mpOwner->insertItems(0,qslOwner);
    mpOwner->setCurrentIndex(0);
    mpTask = new UBTGAdaptableText(widget,this);
    mpTask->setPlaceHolderText(tr("Type task here ..."));
    mpTask->setAcceptRichText(true);
    mpTask->setTextColor(QColor().green());
    mpTask->setObjectName("ActionWidgetTaskTextEdit");
    mpLayout->addWidget(mpOwner);
    mpLayout->addWidget(mpTask);
}

UBTGActionWidget::~UBTGActionWidget()
{
    DELETEPTR(mpOwner);
    DELETEPTR(mpTask);
    DELETEPTR(mpLayout);
}

void UBTGActionWidget::initializeWithDom(QDomElement element)
{
    mpOwner->setCurrentIndex(element.attribute("owner").toInt());
    mpTask->setInitialText(element.attribute("task"));
}

tUBGEElementNode* UBTGActionWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->name = "action";
    result->attributes.insert("owner",QString("%0").arg(mpOwner->currentIndex()));
    result->attributes.insert("task",mpTask->text());
    return result;
}

/***************************************************************************
 *                      class    UBTGAdaptableText                         *
 ***************************************************************************/
UBTGAdaptableText::UBTGAdaptableText(QTreeWidgetItem* widget, QWidget* parent, const char* name):QTextEdit(parent)
  , mBottomMargin(5)
  , mpTreeWidgetItem(widget)
  , mMinimumHeight(0)
  , mHasPlaceHolder(false)
  , mIsUpdatingSize(false)
  , mMaximumLength(0)
{
    setObjectName(name);
    connect(this,SIGNAL(textChanged()),this,SLOT(onTextChanged()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mMinimumHeight = document()->size().height() + mBottomMargin;
    setMinimumHeight(mMinimumHeight);

}

void UBTGAdaptableText::setMaximumLength(int length)
{
    mMaximumLength = length;
}

void UBTGAdaptableText::setPlaceHolderText(QString text)
{
    mHasPlaceHolder = true;

    // the space addition is to make this string unique and check against it to know
    // if we are talking about a typed string or the placeholder string
    mPlaceHolderText = text + "     ";
    setTextColor(QColor(Qt::lightGray));
    setPlainText(mPlaceHolderText);
}

void UBTGAdaptableText::keyPressEvent(QKeyEvent* e)
{
    if(isReadOnly()){
        // this is important if you set a placeholder. In this case even if the text field is readonly the
        // keypressed event came here. So if you don't ignore it you'll have a flick on the text zone
        e->ignore();
        return;
    }

    if(toPlainText() == mPlaceHolderText){
        setPlainText("");
    }
    setTextColor(QColor(Qt::black));
    QTextEdit::keyPressEvent(e);
}

void UBTGAdaptableText::keyReleaseEvent(QKeyEvent* e)
{
    QTextEdit::keyReleaseEvent(e);

    if(toPlainText().isEmpty()){
        setTextColor(QColor(Qt::lightGray));
        setPlainText(mPlaceHolderText);
    }
    if(mMaximumLength && toPlainText().length()>mMaximumLength){
        setPlainText(toPlainText().left(mMaximumLength));
        QTextCursor tc(document());
        tc.setPosition(mMaximumLength);
        setTextCursor(tc);
    }
}

void UBTGAdaptableText::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);
    if(!mIsUpdatingSize && mHasPlaceHolder && toPlainText().isEmpty())
        setPlainText(mPlaceHolderText);
    else
    	// If the teacherguide is collapsed, don't updated the size. Or set the size as the expanded size
        onTextChanged();
}

QString UBTGAdaptableText::text()
{
    QString result = toPlainText();
    if(mHasPlaceHolder && result == mPlaceHolderText)
        return "";

    return result;
}

void UBTGAdaptableText::onTextChanged()
{
	//qDebug() << ">> onTextChanged CALLED!";
    qreal documentSize = document()->size().height();
    //qDebug() << ">>  documentSize: " << documentSize << ", height: " << height();
    if(height() == documentSize + mBottomMargin){
    	return;
    }
    mIsUpdatingSize = true;


    if(documentSize < mMinimumHeight)
        setFixedHeight(mMinimumHeight);
    else
        setFixedHeight(documentSize+mBottomMargin);

    updateGeometry();
    //to trig a resize on the tree widget item
    if(mpTreeWidgetItem){
        mpTreeWidgetItem->setDisabled(true);
        mpTreeWidgetItem->setExpanded(true);
        mpTreeWidgetItem->setDisabled(false);
        setFocus();
    }
    mIsUpdatingSize = false;
    

}

void UBTGAdaptableText::setInitialText(const QString& text)
{
    setText(text);
    setReadOnly(false);
    onTextChanged();
}

void UBTGAdaptableText::showText(const QString & text)
{
    setText(text);
    setReadOnly(true);
    onTextChanged();
}

void UBTGAdaptableText::bottomMargin(int newValue)
{
    mBottomMargin = newValue;
    onTextChanged();
}


/***************************************************************************
 *                      class   UBTGDraggableWeb                           *
 ***************************************************************************/
UBDraggableWeb::UBDraggableWeb(QString& relativePath, QWidget* parent): QWebView(parent)
  , mDragStartPosition(QPoint(-1,-1))
  , mDragStarted(false)

{
	if(!relativePath.startsWith("file://"))
		mRelativePath = QUrl::fromLocalFile(relativePath).toString();
	else
		mRelativePath = relativePath;
    //NOOP
}

void UBDraggableWeb::mousePressEvent(QMouseEvent* event)
{
    mDragStartPosition = event->pos();
    mDragStarted = true;
    QWebView::mousePressEvent(event);
}

void UBDraggableWeb::mouseReleaseEvent(QMouseEvent* event)
{
    mDragStarted = false;
    QWebView::mouseReleaseEvent(event);
}

void UBDraggableWeb::mouseMoveEvent(QMouseEvent* event)
{
    if(mDragStarted && (event->pos() - mDragStartPosition).manhattanLength() > QApplication::startDragDistance()){
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urlList;
        urlList << QUrl(mRelativePath);
        mimeData->setUrls(urlList);
        drag->setMimeData(mimeData);

        drag->exec();
        event->accept();
        mDragStarted = false;
    }
    else
        QWebView::mouseMoveEvent(event);

}

/***************************************************************************
 *                      class    UBTGMediaWidget                           *
 ***************************************************************************/
UBTGMediaWidget::UBTGMediaWidget(QTreeWidgetItem* widget, QWidget* parent,const char* name): QStackedWidget(parent)
  , mpTreeWidgetItem(widget)
  , mpDropMeWidget(NULL)
  , mpWorkWidget(NULL)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpMediaLabelWidget(NULL)
  , mpMediaWidget(NULL)
  , mpWebView(NULL)
  , mMediaPath(QString(""))
  , mIsPresentationMode(false)
  , mIsInitializationMode(false)
{
    setObjectName(name);
    mpDropMeWidget = new QLabel();
    mpDropMeWidget->setObjectName("UBTGMediaDropMeLabel");
    mpDropMeWidget->setText(tr("drop media here ..."));
    mpDropMeWidget->setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    addWidget(mpDropMeWidget);

    setMinimumHeight(250);
}

UBTGMediaWidget::UBTGMediaWidget(QString mediaPath, QTreeWidgetItem* widget, QWidget* parent,bool forceFlashMediaType,const char* name): QStackedWidget(parent)
  , mpTreeWidgetItem(widget)
  , mpDropMeWidget(NULL)
  , mpWorkWidget(NULL)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpMediaLabelWidget(NULL)
  , mpMediaWidget(NULL)
  , mpWebView(NULL)
  , mIsPresentationMode(true)
  , mMediaType("")
  , mIsInitializationMode(false)
{
    setObjectName(name);
    mMediaPath = UBApplication::boardController->selectedDocument()->persistencePath()+ "/" + mediaPath;
    setAcceptDrops(false);
    createWorkWidget(forceFlashMediaType);
    setFixedHeight(200);
}

UBTGMediaWidget::~UBTGMediaWidget()
{
    DELETEPTR(mpTitle);
    DELETEPTR(mpMediaLabelWidget);
    DELETEPTR(mpMediaWidget);
    DELETEPTR(mpWebView);
    DELETEPTR(mpLayout);

    removeWidget(mpDropMeWidget);
    DELETEPTR(mpDropMeWidget);
    removeWidget(mpWorkWidget);
    DELETEPTR(mpWorkWidget);
}

void UBTGMediaWidget::initializeWithDom(QDomElement element)
{
    mIsInitializationMode = true;
    setAcceptDrops(false);
    mMediaPath = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + element.attribute("relativePath");
    createWorkWidget(element.attribute("mediaType").contains("flash"));
    setFixedHeight(200);
    mpTitle->setInitialText(element.attribute("title"));
    mIsInitializationMode = false;
}

void UBTGMediaWidget::removeSource()
{
    QFileInfo fileInfo(mMediaPath);
    if(fileInfo.isFile())
        QFile(mMediaPath).remove();
    else
        UBFileSystemUtils::deleteDir(mMediaPath);
}

void UBTGMediaWidget::hideEvent(QHideEvent* event)
{
    if(mpWebView)
        mpWebView->page()->mainFrame()->setContent(UBGraphicsW3CWidgetItem::freezedWidgetPage().toAscii());
    QWidget::hideEvent(event);
}

void UBTGMediaWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if(mpWebView){
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
}

tUBGEElementNode* UBTGMediaWidget::saveData()
{
    if(!mpTitle)
        return 0;
    tUBGEElementNode* result = new tUBGEElementNode();
    QString relativePath = mMediaPath;
    relativePath = relativePath.replace(UBApplication::boardController->selectedDocument()->persistencePath()+"/","");
	result->name = "media";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("relativePath",relativePath);
    result->attributes.insert("mediaType",mMediaType);
    return result;
}

void UBTGMediaWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void UBTGMediaWidget::createWorkWidget(bool forceFlashMediaType)
{
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mMediaPath);
    bool setMedia = true;
    UBDocumentProxy* proxyDocument = UBApplication::boardController->selectedDocument();
    if(mimeType.contains("audio") || mimeType.contains("video")){
        mMediaType = mimeType.contains("audio")? "audio":"movie";
        mpMediaWidget = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
        if(mIsPresentationMode || mIsInitializationMode){
            mpMediaWidget->setFile(mMediaPath);
        }
        else{
            mMediaPath = UBPersistenceManager::persistenceManager()->addObjectToTeacherGuideDirectory(proxyDocument, mMediaPath);
            mpMediaWidget->setFile(mMediaPath);
        }
    }
    else if(mimeType.contains("image")){
        mMediaType = "image";
        if(!(mIsPresentationMode || mIsInitializationMode))
            mMediaPath = UBPersistenceManager::persistenceManager()->addObjectToTeacherGuideDirectory(proxyDocument, mMediaPath);

        mpMediaLabelWidget = new QLabel();
        QPixmap pixmap = QPixmap(mMediaPath);
        pixmap = pixmap.scaledToWidth(mpTreeWidgetItem->treeWidget()->size().width());
        mpMediaLabelWidget->setPixmap(pixmap);
        mpMediaLabelWidget->setScaledContents(true);
    }
    else if(mimeType.contains("widget") && !forceFlashMediaType){
        mMediaType = "w3c";
        if(!(mIsPresentationMode || mIsInitializationMode)){
            mMediaPath = UBPersistenceManager::persistenceManager()->addWidgetToTeacherGuideDirectory(proxyDocument, mMediaPath);
        }
        mpWebView = new UBDraggableWeb(mMediaPath);
        mpWebView->setAcceptDrops(false);
        mpWebView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mpWebView->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
    else if(mimeType.contains("x-shockwave-flash") || forceFlashMediaType){
        mMediaType = "flash";
        if(!(mIsPresentationMode || mIsInitializationMode)){
            QDir baseW3CDirectory(UBPersistenceManager::persistenceManager()->teacherGuideAbsoluteObjectPath(proxyDocument));
            mMediaPath = UBGraphicsW3CWidgetItem::createNPAPIWrapperInDir(mMediaPath,baseW3CDirectory,mimeType,QSize(100,100),QUuid::createUuid());
        }
        qDebug() << mMediaPath;
        mpWebView = new UBDraggableWeb(mMediaPath);
        mpWebView->setAcceptDrops(false);
        mpWebView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mpWebView->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
        QString indexPath = mMediaPath+"/index.htm";
        if(!QFile::exists(indexPath))
            indexPath += "l";
        mpWebView->load(QUrl::fromLocalFile(indexPath));
    }
    else{
        qDebug() << "createWorkWidget mime type not handled" << mimeType;
        setMedia=false;
    }

    if(setMedia){
        setAcceptDrops(false);
        mpWorkWidget = new QWidget(this);
        mpLayout = new QVBoxLayout(mpWorkWidget);
        if(!mIsPresentationMode){
            mpTitle = new UBTGAdaptableText(mpTreeWidgetItem,mpWorkWidget);
            mpTitle->setPlaceHolderText(tr("Type title here..."));
            mpLayout->addWidget(mpTitle);
        }
        if(mpMediaLabelWidget){
            mpMediaLabelWidget->setMaximumHeight(width());
            mpMediaLabelWidget->setParent(mpWorkWidget);
            mpLayout->addWidget(mpMediaLabelWidget);
        }
        else if (mpMediaWidget){
            mpMediaWidget->setMaximumHeight(width());
            mpMediaWidget->setParent(mpWorkWidget);
            mpLayout->addWidget(mpMediaWidget);
        }
        else if (mpWebView){
            mpWebView->setMaximumHeight(width());
            mpWebView->setParent(mpWorkWidget);
            mpLayout->addWidget(mpWebView);
            mpWebView->show();
        }
        mpWorkWidget->setLayout(mpLayout);
        addWidget(mpWorkWidget);
        setCurrentWidget(mpWorkWidget);
    }
}

void UBTGMediaWidget::parseMimeData(const QMimeData* pMimeData)
{
    if(pMimeData){
        if(pMimeData->hasText()){
            mMediaPath = QUrl::fromLocalFile(pMimeData->text()).toString();
        }
        else if(pMimeData->hasUrls()){
            mMediaPath = pMimeData->urls().at(0).toString();
        }
        else if(pMimeData->hasImage()){
            qDebug() << "Not yet implemented";
        }
    }
    else
        qDebug() << "No mime data present";

    createWorkWidget();
}

void UBTGMediaWidget::dropEvent(QDropEvent* event)
{
    parseMimeData(event->mimeData());
    event->accept();
}

void UBTGMediaWidget::mousePressEvent(QMouseEvent *event)
{
    if (!mIsPresentationMode)
        event->ignore();
     else{

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData();
        QList<QUrl> urlList;
#ifdef Q_WS_WIN
		urlList << QUrl::fromLocalFile(mMediaPath);
#else
        urlList << QUrl(mMediaPath);
#endif
        mimeData->setUrls(urlList);
        drag->setMimeData(mimeData);

        drag->exec();
        event->accept();
    }
}



/***************************************************************************
 *                      class    UBTGUrlWidget                             *
 ***************************************************************************/
UBTGUrlWidget::UBTGUrlWidget(QWidget* parent, const char* name ):QWidget(parent)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpUrl(NULL)
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("UBTGLineEdit");
    mpTitle->setPlaceholderText(tr("Insert link title here..."));
    mpUrl = new QLineEdit(this);
    connect(mpUrl,SIGNAL(editingFinished()),this,SLOT(onUrlEditionFinished()));
    mpUrl->setObjectName("UBTGLineEdit");
    mpUrl->setPlaceholderText("http://");
    mpLayout->addWidget(mpTitle);
    mpLayout->addWidget(mpUrl);
}

UBTGUrlWidget::~UBTGUrlWidget()
{
    DELETEPTR(mpTitle);
    DELETEPTR(mpUrl);
    DELETEPTR(mpLayout);
}

void UBTGUrlWidget::onUrlEditionFinished()
{
	QString url = mpUrl->text();
	if(url.length() && !url.startsWith("http://") && !url.startsWith("https://") && !url.startsWith("ftp://") && !url.startsWith("sftp://") && !url.startsWith("http://")){
		mpUrl->setText("http://" + mpUrl->text());
		setFocus();
	}
}

void UBTGUrlWidget::initializeWithDom(QDomElement element)
{
    mpTitle->setText(element.attribute("title"));
    mpUrl->setText(element.attribute("url"));
}

tUBGEElementNode* UBTGUrlWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->name = "link";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("url",mpUrl->text());
    return result;
}


/***************************************************************************
 *              class    UBTGDraggableTreeItem                             *
 ***************************************************************************/
UBTGDraggableTreeItem::UBTGDraggableTreeItem(QWidget* parent, const char* name) : QTreeWidget(parent)
{
    setObjectName(name);
}

QMimeData* UBTGDraggableTreeItem::mimeData(const QList<QTreeWidgetItem *> items) const
{
    QMimeData* result = new QMimeData();
    QList<QUrl> urls;
    urls << QUrl(items.at(0)->data(0,TG_USER_ROLE_MIME_TYPE).toString());
    result->setUrls(urls);
    return result;
}
