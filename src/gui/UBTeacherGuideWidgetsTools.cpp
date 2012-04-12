/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#include <QDomElement>
#include <QDomDocument>
#include <QApplication>

#include "UBTeacherGuideWidgetsTools.h"
#include "UBTGWidgetTreeDelegate.h"

#include "globals/UBGlobals.h"

#include "frameworks/UBFileSystemUtils.h"


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
    mpLayout->addWidget(mpOwner,0);
    mpLayout->addWidget(mpTask,1);
}

UBTGActionWidget::~UBTGActionWidget()
{
    DELETEPTR(mpOwner);
    DELETEPTR(mpTask);
    DELETEPTR(mpLayout);
}

tUBGEElementNode* UBTGActionWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->type = "action";
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
  , mMinimumHeight(20)
  , mHasPlaceHolder(false)
  , mIsUpdatingSize(false)
{
    setObjectName(name);
    setStyleSheet( "QWidget {background: white; border:1 solid #999999; border-radius : 10px; padding: 2px;}");
    connect(this,SIGNAL(textChanged()),this,SLOT(onTextChanged()));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setMinimumHeight(mMinimumHeight);
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
    if(toPlainText() == mPlaceHolderText){
        setTextColor(QColor(Qt::black));
        setPlainText("");
    }
    QTextEdit::keyPressEvent(e);
}

void UBTGAdaptableText::keyReleaseEvent(QKeyEvent* e)
{
    QTextEdit::keyReleaseEvent(e);

    if(toPlainText().isEmpty()){
        setTextColor(QColor(Qt::lightGray));
        setPlainText(mPlaceHolderText);
    }
}

void UBTGAdaptableText::showEvent(QShowEvent* e)
{
    Q_UNUSED(e);
    if(!mIsUpdatingSize && mHasPlaceHolder && toPlainText().isEmpty())
        setPlainText(mPlaceHolderText);
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
    mIsUpdatingSize = true;
    if(document()->size().height() < mMinimumHeight)
        setFixedHeight(mMinimumHeight);
    else
        setFixedHeight(document()->size().height()+mBottomMargin);
    updateGeometry();
    //to trig the widget item to resize it
    if(mpTreeWidgetItem){
        mpTreeWidgetItem->setExpanded(false);
        mpTreeWidgetItem->setExpanded(true);
        setFocus();
    }
    mIsUpdatingSize = false;
}

void UBTGAdaptableText::showText(const QString & text)
{
    setText(text);
    //A first rendering has to be done to calculate the text's size.
    show();
    hide();
    setReadOnly(true);
    onTextChanged();
    if(isHidden())
        show();
}

void UBTGAdaptableText::bottomMargin(int newValue)
{
    mBottomMargin = newValue;
    onTextChanged();
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
  , mRelativePath(QString(""))
  , mIsPresentationMode(false)
{
    setObjectName(name);
    mpDropMeWidget = new QLabel();
    mpDropMeWidget->setObjectName("UBTGMediaDropMeLabel");
    mpDropMeWidget->setText(tr("drop media here ..."));
    mpDropMeWidget->setAlignment(Qt::AlignCenter);
    setAcceptDrops(true);
    addWidget(mpDropMeWidget);

    setMinimumHeight(200);
}

UBTGMediaWidget::UBTGMediaWidget(QString relativePath, QTreeWidgetItem* widget, QWidget* parent,const char* name): QStackedWidget(parent)
  , mpTreeWidgetItem(widget)
  , mpDropMeWidget(NULL)
  , mpWorkWidget(NULL)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpMediaLabelWidget(NULL)
  , mpMediaWidget(NULL)
  , mpWebView(NULL)
  , mRelativePath(relativePath)
  , mIsPresentationMode(true)
  , mMediaType("")
{
    setObjectName(name);
    setAcceptDrops(false);
    createWorkWidget(mRelativePath);
    setMinimumHeight(200);
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

tUBGEElementNode* UBTGMediaWidget::saveData()
{
    if(!mpTitle)
        return 0;
    tUBGEElementNode* result = new tUBGEElementNode();
    result->type = "media";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("relativePath",mRelativePath);
    result->attributes.insert("mediaType",mMediaType);
    return result;
}

void UBTGMediaWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->accept();
}

void UBTGMediaWidget::createWorkWidget(QString& path)
{
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(path);
    qDebug() << mimeType;
    bool setMedia = true;
    if(mimeType.contains("audio") || mimeType.contains("video")){
        mMediaType = mimeType.contains("audio")? "audio":"movie";
        mpMediaWidget = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
        mpMediaWidget->setFile(path);
    }
    else if(mimeType.contains("image")){
        mMediaType = "image";
        mpMediaLabelWidget = new QLabel();
        QPixmap pixmap = QPixmap(QUrl(path).toLocalFile());
        pixmap = pixmap.scaledToWidth(mpTreeWidgetItem->treeWidget()->size().width());
        mpMediaLabelWidget->setPixmap(pixmap);
        mpMediaLabelWidget->setScaledContents(true);
    }
    else if(mimeType.contains("application")){
        mMediaType = "w3c";
        mpWebView = new QWebView(0);
        mpWebView->setAcceptDrops(false);
        mpWebView->settings()->setAttribute(QWebSettings::JavaEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
        mpWebView->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
        mpWebView->settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);
        mpWebView->load(QUrl(path));
        mpWebView->show();
    }
    else{
        qDebug() << "createWorkWidget mime type not handled" << mimeType;
        setMedia=false;
    }

    if(setMedia){
        mRelativePath = path;
        setAcceptDrops(false);
        mpWorkWidget = new QWidget(this);
        mpLayout = new QVBoxLayout(mpWorkWidget);
        if(!mIsPresentationMode){
            mpTitle = new UBTGAdaptableText(mpTreeWidgetItem,mpWorkWidget);
            mpTitle->setPlaceHolderText(tr("Type title here..."));
            mpLayout->addWidget(mpTitle,1);
        }
        if(mpMediaLabelWidget){
            mpMediaLabelWidget->setParent(mpWorkWidget);
            mpLayout->addWidget(mpMediaLabelWidget);
        }
        else if (mpMediaWidget){
            mpMediaWidget->setMaximumHeight(mpTreeWidgetItem->treeWidget()->size().width());
            mpMediaWidget->setParent(mpWorkWidget);
            mpLayout->addWidget(mpMediaWidget);
        }
        else if (mpWebView){
            mpWebView->setMaximumHeight(mpTreeWidgetItem->treeWidget()->size().width());
            mpWebView->setParent(mpWorkWidget);
            mpLayout->addWidget(mpWebView);
        }
        mpWorkWidget->setLayout(mpLayout);
        addWidget(mpWorkWidget);
        setCurrentWidget(mpWorkWidget);
        updateSize();
    }
}

void UBTGMediaWidget::parseMimeData(const QMimeData* pMimeData)
{
    QString path;
    if(pMimeData){
        if(pMimeData->hasText()){
            path = QUrl::fromLocalFile(pMimeData->text()).toString();
        }
        else if(pMimeData->hasUrls()){
            path = pMimeData->urls().at(0).toString();
        }
        else if(pMimeData->hasImage()){
            qDebug() << "Not yet implemented";
        }
    }
    else
        qDebug() << "No mime data present";

    createWorkWidget(path);
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
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urlList;
        urlList << QUrl(mRelativePath);
        mimeData->setUrls(urlList);
        drag->setMimeData(mimeData);

        drag->exec();
        event->accept();
    }
}

void UBTGMediaWidget::updateSize()
{
    if(mpTreeWidgetItem){
        mpTreeWidgetItem->setExpanded(false);
        mpTreeWidgetItem->setExpanded(true);
        if(!mIsPresentationMode)
            mpTitle->setFocus();
    }
}

/***************************************************************************
 *                      class    UBTGUrlWdiget                             *
 ***************************************************************************/
UBTGUrlWidget::UBTGUrlWidget(QWidget* parent, const char* name ):QWidget(parent)
  , mpLayout(NULL)
  , mpTitle(NULL)
  , mpUrl(NULL)
{
    setObjectName(name);
    SET_STYLE_SHEET();
    mpLayout = new QVBoxLayout(this);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("UBTGLineEdit");
    mpTitle->setPlaceholderText(tr("Insert link title here..."));
    mpUrl = new QLineEdit(this);
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

tUBGEElementNode* UBTGUrlWidget::saveData()
{
    tUBGEElementNode* result = new tUBGEElementNode();
    result->type = "link";
    result->attributes.insert("title",mpTitle->text());
    result->attributes.insert("url",mpUrl->text());
    return result;
}
