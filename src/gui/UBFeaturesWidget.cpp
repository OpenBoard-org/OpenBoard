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




#include <QDomDocument>
#include <QWebChannel>
#include <QWidget>

#include "UBFeaturesWidget.h"
#include "gui/UBThumbnailWidget.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBDownloadManager.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"
#include "web/UBWebController.h"

const char *UBFeaturesWidget::objNamePathList = "PathList";
const char *UBFeaturesWidget::objNameFeatureList = "FeatureList";

const QMargins FeatureListMargins(0, 0, 0, 30);
const int FeatureListBorderOffset = 10;
const char featureTypeSplitter = ':';
static const QString mimeSankoreFeatureTypes = "Sankore/featureTypes";

UBFeaturesWidget::UBFeaturesWidget(QWidget *parent, const char *name)
    : UBDockPaletteWidget(parent)
    , imageGatherer(NULL)
{
    setObjectName(name);
    mName = "FeaturesWidget";
    mVisibleState = true;

    SET_STYLE_SHEET();

    mIconToLeft = QPixmap(":images/library_open.png");
    mIconToRight = QPixmap(":images/library_close.png");
    setAcceptDrops(true);

    //Main UBFeature functionality
    controller = new UBFeaturesController(this);

    //Main layout including all the widgets in palette
    layout = new QVBoxLayout(this);

    //Path icon view on the top of the palette
    pathListView = new UBFeaturesListView(this, objNamePathList);
    controller->assignPathListView(pathListView);

    centralWidget = new UBFeaturesCentralWidget(this);
    controller->assignFeaturesListView(centralWidget->listView());
    centralWidget->setSliderPosition(UBSettings::settings()->featureSliderPosition->get().toInt());

    //Bottom actionbar for DnD, quick search etc
    mActionBar = new UBFeaturesActionBar(controller, this);

    //Filling main layout
    layout->addWidget(pathListView);
    layout->addWidget(centralWidget);
    layout->addWidget(mActionBar);

    connect(centralWidget->listView(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(currentSelected(const QModelIndex &)));
    connect(this, SIGNAL(sendFileNameList(QStringList)), centralWidget, SIGNAL(sendFileNameList(QStringList)));
    connect(mActionBar, SIGNAL(searchElement(const QString &)), this, SLOT( searchStarted(const QString &)));
    connect(mActionBar, SIGNAL(newFolderToCreate()), this, SLOT(createNewFolder()));
    connect(mActionBar, SIGNAL(deleteElements(const UBFeaturesMimeData *)), this, SLOT(deleteElements(const UBFeaturesMimeData *)));
    connect(mActionBar, SIGNAL(deleteSelectedElements()), this, SLOT(deleteSelectedElements()));
    connect(mActionBar, SIGNAL(addToFavorite(const UBFeaturesMimeData *)), this, SLOT(addToFavorite(const UBFeaturesMimeData *)));
    connect(mActionBar, SIGNAL(removeFromFavorite(const UBFeaturesMimeData *)), this, SLOT(removeFromFavorite(const UBFeaturesMimeData *)));
    connect(mActionBar, SIGNAL(addElementsToFavorite() ), this, SLOT ( addElementsToFavorite()) );
    connect(mActionBar, SIGNAL(removeElementsFromFavorite()), this, SLOT (removeElementsFromFavorite()));

    connect(mActionBar, SIGNAL(rescanModel()), this, SLOT(rescanModel()));
    connect(pathListView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(currentSelected(const QModelIndex &)));
    connect(UBApplication::boardController, SIGNAL(displayMetadata(QMap<QString,QString>)), this, SLOT(onDisplayMetadata( QMap<QString,QString>)));
    connect(UBDownloadManager::downloadManager(), SIGNAL( addDownloadedFileToLibrary( bool, QUrl, QString, QByteArray, QString))
             , this, SLOT(onAddDownloadedFileToLibrary(bool, QUrl, QString,QByteArray, QString)));
    connect(centralWidget, SIGNAL(lockMainWidget(bool)), this, SLOT(lockIt(bool)));
    connect(centralWidget, SIGNAL(createNewFolderSignal(QString)), controller, SLOT(addNewFolder(QString)));
    connect(controller, SIGNAL(scanStarted()), centralWidget, SLOT(scanStarted()));
    connect(controller, SIGNAL(scanFinished()), centralWidget, SLOT(scanFinished()));
    connect(controller, SIGNAL(scanStarted()), mActionBar, SLOT(lockIt()));
    connect(controller, SIGNAL(scanFinished()), mActionBar, SLOT(unlockIt()));
    connect(controller, SIGNAL(maxFilesCountEvaluated(int)), centralWidget, SIGNAL(maxFilesCountEvaluated(int)));
    connect(controller, SIGNAL(featureAddedFromThread()), centralWidget, SIGNAL(increaseStatusBarValue()));
    connect(controller, SIGNAL(scanCategory(QString)), centralWidget, SIGNAL(scanCategory(QString)));
    connect(controller, SIGNAL(scanPath(QString)), centralWidget, SIGNAL(scanPath(QString)));
}

UBFeaturesWidget::~UBFeaturesWidget()
{
    if (NULL != imageGatherer)
        delete imageGatherer;
}

void UBFeaturesWidget::searchStarted(const QString &pattern)
{
    controller->searchStarted(pattern, centralWidget->listView());
}

void UBFeaturesWidget::currentSelected(const QModelIndex &current)
{
    if (!current.isValid()) {
        qWarning() << "SLOT:currentSelected, invalid index catched";
        return;
    }

    QString objName = sender()->objectName();

    if (objName.isEmpty()) {
        qWarning() << "incorrect sender";
    } else if (objName == objNamePathList) {
        //Calling to reset the model for listView. Maybe separate function needed
        controller->searchStarted("", centralWidget->listView());
    }

    UBFeature feature = controller->getFeature(current, objName);

    if ( feature.isFolder() ) {
        QString newPath = feature.getFullVirtualPath();

        controller->setCurrentElement(feature);
        controller->siftElements(newPath);

        centralWidget->switchTo(UBFeaturesCentralWidget::MainList);

        if ( feature.getType() == FEATURE_FAVORITE ) {
            mActionBar->setCurrentState( IN_FAVORITE );

        }  else if ( feature.getType() == FEATURE_CATEGORY && feature.getName() == "root" ) {
            mActionBar->setCurrentState( IN_ROOT );

        } else if (feature.getType() == FEATURE_TRASH) {
            mActionBar->setCurrentState(IN_TRASH);

        } else if (feature.getType() == FEATURE_SEARCH) {
            //The search feature behavior is not standard. If features list clicked - show empty element
            //else show existing saved features search QWebView
            if (sender()->objectName() == objNameFeatureList) {
                centralWidget->showElement(feature, UBFeaturesCentralWidget::FeaturesWebView);
            } else if (sender()->objectName() == objNamePathList) {
                centralWidget->switchTo(UBFeaturesCentralWidget::FeaturesWebView);
            }

        } else  {
            mActionBar->setCurrentState(IN_FOLDER);
        }

//    } else if (feature.getType() == FEATURE_SEARCH) {
//        centralWidget->showElement(feature, UBFeaturesCentralWidget::FeaturesWebView);

    }

    else if (UBSettings::settings()->libraryShowDetailsForLocalItems->get().toBool() == true) {
        centralWidget->showElement(feature, UBFeaturesCentralWidget::FeaturePropertiesList);
        mActionBar->setCurrentState( IN_PROPERTIES );
    }
    mActionBar->cleanText();
}

void UBFeaturesWidget::createNewFolder()
{
    centralWidget->showAdditionalData(UBFeaturesCentralWidget::NewFolderDialog, UBFeaturesCentralWidget::Modal);
    emit sendFileNameList(controller->getFileNamesInFolders());
}

void UBFeaturesWidget::deleteElements( const UBFeaturesMimeData * mimeData )
{
    if (!mimeData->features().count() )
        return;

    QList<UBFeature> featuresList = mimeData->features();

    foreach ( UBFeature curFeature, featuresList ) {
        if ( curFeature.inTrash()) {
            controller->deleteItem(curFeature.getFullPath());

        } else {
           controller->moveToTrash(curFeature);
        }
    }

    controller->refreshModels();
}

void UBFeaturesWidget::deleteSelectedElements()
{
    QModelIndexList selected = centralWidget->listView()->selectionModel()->selectedIndexes();

    QList<UBFeature> featureasToMove;
    for (int i = 0; i < selected.count(); i++)
    {
        featureasToMove.append(controller->getFeature(selected.at(i), objNameFeatureList));
    }

    foreach (UBFeature feature, featureasToMove)
    {
        if (feature.isDeletable()) {
            if (feature.inTrash()) {
                controller->deleteItem(feature);
            } else {
                controller->moveToTrash(feature, true);
            }
        }
    }

    controller->refreshModels();
}

void UBFeaturesWidget::rescanModel()
{
    controller->rescanModel();
}

void UBFeaturesWidget::lockIt(bool pLock)
{
    mActionBar->setEnabled(!pLock);
    pathListView->setEnabled(!pLock);
    centralWidget->setLockedExcludingAdditional(pLock);
}

void UBFeaturesWidget::addToFavorite( const UBFeaturesMimeData * mimeData )
{
    if ( !mimeData->hasUrls() )
        return;

    QList<QUrl> urls = mimeData->urls();
    foreach ( QUrl url, urls ) {
        controller->addToFavorite(url);
    }

    controller->refreshModels();
}

void UBFeaturesWidget::removeFromFavorite( const UBFeaturesMimeData * mimeData )
{
    if ( !mimeData->hasUrls() )
        return;

    QList<QUrl> urls = mimeData->urls();

    foreach( QUrl url, urls ) {
        controller->removeFromFavorite(url);
    }
}

void UBFeaturesWidget::onDisplayMetadata( QMap<QString,QString> metadata )
{
    QString previewImageUrl = ":images/libpalette/notFound.png";

    QString widgetsUrl = QUrl::fromEncoded(metadata["Url"].toLatin1()).toString()/*metadata.value("Url", QString())*/;
    QString widgetsThumbsUrl = QUrl::fromEncoded(metadata["thumbnailUrl"].toLatin1()).toString();

    QString strType = UBFileSystemUtils::mimeTypeFromFileName(widgetsUrl);
    UBMimeType::Enum thumbType = UBFileSystemUtils::mimeTypeFromString(strType);

    switch (static_cast<int>(thumbType)) {
    case UBMimeType::Audio:
        previewImageUrl = ":images/libpalette/soundIcon.svg";
        break;

    case UBMimeType::Video:
        previewImageUrl = ":images/libpalette/movieIcon.svg";
        break;

    case UBMimeType::Flash:
        previewImageUrl = ":images/libpalette/FlashIcon.svg";
        break;

    case UBMimeType::RasterImage:
    case UBMimeType::VectorImage:
        previewImageUrl = widgetsUrl;
        break;
    }

    if (!widgetsThumbsUrl.isNull()) {
        previewImageUrl = ":/images/libpalette/loading.png";
        if (!imageGatherer)
            imageGatherer = new UBDownloadHttpFile(0, this);

        connect(imageGatherer, SIGNAL(downloadFinished(int, bool, QUrl, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(onPreviewLoaded(int, bool, QUrl, QUrl, QString, QByteArray, QPointF, QSize, bool)));

        // We send here the request and store its reply in order to be able to cancel it if needed
        imageGatherer->get(QUrl(widgetsThumbsUrl), QPoint(0,0), QSize(), false);
    }

    UBFeature feature( "/root", QImage(previewImageUrl), QString(), widgetsUrl, FEATURE_ITEM );
    feature.setMetadata( metadata );

    centralWidget->showElement(feature, UBFeaturesCentralWidget::FeaturePropertiesList);
    mActionBar->setCurrentState( IN_PROPERTIES );
}


void UBFeaturesWidget::onPreviewLoaded(int id, bool pSuccess, QUrl sourceUrl, QUrl originalUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    Q_UNUSED(id);
    Q_UNUSED(pSuccess);
    Q_UNUSED(originalUrl);
    Q_UNUSED(isBackground);
    Q_UNUSED(pSize);
    Q_UNUSED(pPos);
    Q_UNUSED(sourceUrl);
    Q_UNUSED(pContentTypeHeader)

    QImage img;
    img.loadFromData(pData);
    QPixmap pix = QPixmap::fromImage(img);
    centralWidget->setPropertiesPixmap(pix);
    centralWidget->setPropertiesThumbnail(pix);
}

void UBFeaturesWidget::onAddDownloadedFileToLibrary(bool pSuccess, QUrl sourceUrl, QString pContentHeader, QByteArray pData, QString pTitle)
{
    if (pSuccess) {
        qDebug() << pData.length();
        controller->addDownloadedFile(sourceUrl, pData, pContentHeader, pTitle);
        controller->refreshModels();
    }
}

void UBFeaturesWidget::addElementsToFavorite()
{
    if ( centralWidget->currentView() == UBFeaturesCentralWidget::FeaturePropertiesList ) {
        UBFeature feature = centralWidget->getCurElementFromProperties();
        if ( feature != UBFeature() && !UBApplication::isFromWeb(feature.getFullPath().toString())) {
            controller->addToFavorite( feature.getFullPath() );
        }

    } else if ( centralWidget->currentView() == UBFeaturesCentralWidget::MainList ) {
        QModelIndexList selected = centralWidget->listView()->selectionModel()->selectedIndexes();
        for ( int i = 0; i < selected.size(); ++i ) {
            UBFeature feature = selected.at(i).data( Qt::UserRole + 1 ).value<UBFeature>();
            controller->addToFavorite(feature.getFullPath());
       }
    }

    controller->refreshModels();
}

void UBFeaturesWidget::removeElementsFromFavorite()
{
    QModelIndexList selected = centralWidget->listView()->selectionModel()->selectedIndexes();
    QList <QUrl> items;
    for ( int i = 0; i < selected.size(); ++i )  {
        UBFeature feature = selected.at(i).data( Qt::UserRole + 1 ).value<UBFeature>();
        items.append( feature.getFullPath() );
    }

    foreach ( QUrl url, items )  {
        controller->removeFromFavorite(url, true);
    }

    controller->refreshModels();
}

void UBFeaturesWidget::switchToListView()
{
//    stackedWidget->setCurrentIndex(ID_LISTVIEW);
//    currentStackedWidget = ID_LISTVIEW;
}

void UBFeaturesWidget::switchToProperties()
{
//    stackedWidget->setCurrentIndex(ID_PROPERTIES);
//    currentStackedWidget = ID_PROPERTIES;
}

void UBFeaturesWidget::switchToWebView()
{
//    stackedWidget->setCurrentIndex(ID_WEBVIEW);
//    currentStackedWidget = ID_WEBVIEW;
}

QStringList UBFeaturesMimeData::formats() const
{
    return QMimeData::formats();
}

void UBFeaturesWidget::importImage(const QImage &image, const QString &fileName)
{
    controller->importImage(image, fileName);
}

UBFeaturesListView::UBFeaturesListView( QWidget* parent, const char* name )
    : QListView(parent)
{
    setObjectName(name);
}

void UBFeaturesListView::dragEnterEvent( QDragEnterEvent *event )
{
    if ( event->mimeData()->hasUrls() || event->mimeData()->hasImage() )
        event->acceptProposedAction();
}

void UBFeaturesListView::dragMoveEvent( QDragMoveEvent *event )
{
    const UBFeaturesMimeData *fMimeData = qobject_cast<const UBFeaturesMimeData*>(event->mimeData());
    QModelIndex index = indexAt(event->pos());
    UBFeature onFeature = model()->data(index, Qt::UserRole + 1).value<UBFeature>();
    if (fMimeData) {
        if (!index.isValid() || !onFeature.isFolder()) {
            event->ignore();
            return;
        }
        foreach (UBFeature curFeature, fMimeData->features()) {
            if (curFeature == onFeature) {
                event->ignore();
                return;
            }
        }
     }

    if ( event->mimeData()->hasUrls() || event->mimeData()->hasImage() ) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void UBFeaturesListView::dropEvent( QDropEvent *event )
{
    QObject *eventSource = event->source();
    if (eventSource && eventSource->objectName() == UBFeaturesWidget::objNameFeatureList) {
        event->setDropAction( Qt::MoveAction );
    }

    QListView::dropEvent( event );
}

void UBFeaturesListView::thumbnailSizeChanged( int value )
{
    setIconSize(QSize(value, value));
    setGridSize(QSize(value + 20, value + 20 ));

    UBSettings::settings()->featureSliderPosition->set(value);
}

UBFeaturesNavigatorWidget::UBFeaturesNavigatorWidget(QWidget *parent, const char *name) :
    QWidget(parent), mListView(0), mListSlider(0)

{
    name = "UBFeaturesNavigatorWidget";

    setObjectName(name);
//    SET_STYLE_SHEET()

    mListView = new UBFeaturesListView(this, UBFeaturesWidget::objNameFeatureList);

    mListSlider = new QSlider(Qt::Horizontal, this);

    mListSlider->setMinimum(UBFeaturesWidget::minThumbnailSize);
    mListSlider->setMaximum(UBFeaturesWidget::maxThumbnailSize);
    mListSlider->setValue(UBFeaturesWidget::minThumbnailSize);
    mListSlider->setMinimumHeight(20);

    mListView->setParent(this);
    QVBoxLayout *mainLayer = new QVBoxLayout(this);

    mainLayer->addWidget(mListView, 1);
    mainLayer->addWidget(mListSlider, 0);
    mainLayer->setContentsMargins(0, 0, 0, 0);

    connect(mListSlider, SIGNAL(valueChanged(int)), mListView, SLOT(thumbnailSizeChanged(int)));
}

void UBFeaturesNavigatorWidget::setSliderPosition(int pValue)
{
    mListSlider->setValue(pValue);
}

UBFeaturesCentralWidget::UBFeaturesCentralWidget(QWidget *parent) : QWidget(parent)
{
    setObjectName("UBFeaturesCentralWidget");
    SET_STYLE_SHEET();

    QVBoxLayout *mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    //Maintains the view of the main part of the palette. Consists of
    //mNavigator
    //featureProperties
    //webVeiw
    mStackedWidget = new QStackedWidget(this);

    //Main features icon view with QSlider on the bottom
    mNavigator = new UBFeaturesNavigatorWidget(this);

    //Specifies the properties of a standalone element
    mFeatureProperties = new UBFeatureProperties(this);

    //Used to show search bar on the search widget
    webView = new UBFeaturesWebView(this);

    //filling stackwidget
    mStackedWidget->addWidget(mNavigator);
    mStackedWidget->addWidget(mFeatureProperties);
    mStackedWidget->addWidget(webView);
    mStackedWidget->setCurrentIndex(MainList);
    mStackedWidget->setContentsMargins(0, 0, 0, 0);


    mAdditionalDataContainer = new QStackedWidget(this);
    mAdditionalDataContainer->setObjectName("mAdditionalDataContainer");

    //New folder dialog
    UBFeaturesNewFolderDialog *dlg = new UBFeaturesNewFolderDialog(mAdditionalDataContainer);
    mAdditionalDataContainer->addWidget(dlg);
    mAdditionalDataContainer->setCurrentIndex(NewFolderDialog);

    connect(dlg, SIGNAL(createNewFolder(QString)), this, SLOT(createNewFolderSlot(QString)));
    connect(dlg, SIGNAL(closeDialog()), this, SLOT(hideAdditionalData()));
    connect(this, SIGNAL(sendFileNameList(QStringList)), dlg, SLOT(setFileNameList(QStringList)));

    //Progress bar to show scanning progress
    UBFeaturesProgressInfo *progressBar = new UBFeaturesProgressInfo();
    mAdditionalDataContainer->addWidget(progressBar);
    mAdditionalDataContainer->setCurrentIndex(ProgressBarWidget);

    connect(this, SIGNAL(maxFilesCountEvaluated(int)), progressBar, SLOT(setProgressMax(int)));
    connect(this, SIGNAL(increaseStatusBarValue()), progressBar, SLOT(increaseProgressValue()));
    connect(this, SIGNAL(scanCategory(QString)), progressBar, SLOT(setCommmonInfoText(QString)));
    connect(this, SIGNAL(scanPath(QString)), progressBar, SLOT(setDetailedInfoText(QString)));

    mLayout->addWidget(mStackedWidget, 1);
    mLayout->addWidget(mAdditionalDataContainer, 0);

    mAdditionalDataContainer->hide();
}

void UBFeaturesCentralWidget::showElement(const UBFeature &feature, StackElement pView)
{
    if (pView == FeaturesWebView) {
        webView->showElement(feature);
        mStackedWidget->setCurrentIndex(FeaturesWebView);
    } else if (pView == FeaturePropertiesList) {
        mFeatureProperties->showElement(feature);
        mStackedWidget->setCurrentIndex(FeaturePropertiesList);
    }
}

void UBFeaturesCentralWidget::switchTo(StackElement pView)
{
    mStackedWidget->setCurrentIndex(pView);
}

void UBFeaturesCentralWidget::setPropertiesPixmap(const QPixmap &pix)
{
    mFeatureProperties->setOrigPixmap(pix);
}

void UBFeaturesCentralWidget::setPropertiesThumbnail(const QPixmap &pix)
{
    mFeatureProperties->setThumbnail(pix);
}

UBFeature UBFeaturesCentralWidget::getCurElementFromProperties()
{
    return mFeatureProperties->getCurrentElement();
}

void UBFeaturesCentralWidget::showAdditionalData(AddWidget pWidgetType, AddWidgetState pState)
{
    if (!mAdditionalDataContainer->widget(pWidgetType)) {
        qDebug() << "can't find widget specified by UBFeaturesCentralWidget::showAdditionalData(AddWidget pWidgetType, AddWidgetState pState)";
        return;
    }

    mAdditionalDataContainer->setMaximumHeight(mAdditionalDataContainer->widget(pWidgetType)->sizeHint().height());

    mAdditionalDataContainer->setCurrentIndex(pWidgetType);
    mAdditionalDataContainer->show();
    emit lockMainWidget(pState == Modal ? true : false);
}

void UBFeaturesCentralWidget::setLockedExcludingAdditional(bool pLock)
{
//    Lock all the members excluding mAdditionalDataContainer
    mStackedWidget->setEnabled(!pLock);
}

void UBFeaturesCentralWidget::createNewFolderSlot(QString pStr)
{
    emit createNewFolderSignal(pStr);
    hideAdditionalData();
}

void UBFeaturesCentralWidget::hideAdditionalData()
{
    emit lockMainWidget(false);
    mAdditionalDataContainer->hide();
}

void UBFeaturesCentralWidget::scanStarted()
{
    showAdditionalData(ProgressBarWidget);
}

void UBFeaturesCentralWidget::scanFinished()
{
    hideAdditionalData();
}

UBFeaturesNewFolderDialog::UBFeaturesNewFolderDialog(QWidget *parent) : QWidget(parent)
  , acceptText(tr("Accept"))
  , cancelText(tr("Cancel"))
  , labelText(tr("Enter a new folder name"))
{
    this->setStyleSheet("QPushButton { background:white; }");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QVBoxLayout *labelLayout = new QVBoxLayout();
    labelLayout->setSizeConstraint(QLayout::SetMinimumSize);

    QLabel *mLabel = new QLabel(labelText, this);
    mLabel->setWordWrap(true);
    mLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    mLineEdit = new QLineEdit(this);

    mValidator = new QRegularExpressionValidator(QRegularExpression("[^\\/\\:\\?\\*\\|\\<\\>\\\"]{2,}"), this);
    mLineEdit->setValidator(mValidator);
    labelLayout->addWidget(mLabel);
    labelLayout->addWidget(mLineEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    acceptButton = new QPushButton(acceptText, this);
    QPushButton *cancelButton = new QPushButton(cancelText, this);
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(cancelButton);

    mainLayout->addLayout(labelLayout);
    mainLayout->addLayout(buttonLayout);

    acceptButton->setEnabled(false);

    connect(acceptButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(mLineEdit, SIGNAL(textEdited(QString)), this, SLOT(reactOnTextChanged(QString)));

    reactOnTextChanged(QString());
}

void UBFeaturesNewFolderDialog::setRegexp(const QRegularExpression pRegExp)
{
    mValidator->setRegularExpression(pRegExp);
}
bool UBFeaturesNewFolderDialog::validString(const QString &pStr)
{
    return mLineEdit->hasAcceptableInput() && !mFileNameList.contains(pStr, Qt::CaseSensitive);
}

void UBFeaturesNewFolderDialog::accept()
{
//     Setting all the constraints we need
    emit createNewFolder(mLineEdit->text());
    mLineEdit->clear();
}
void UBFeaturesNewFolderDialog::reject()
{
    mLineEdit->clear();
    emit closeDialog();
}
void UBFeaturesNewFolderDialog::setFileNameList(const QStringList &pLst)
{
    mFileNameList = pLst;
}
void UBFeaturesNewFolderDialog::reactOnTextChanged(const QString &pStr)
{
    if (validString(pStr)) {
        acceptButton->setEnabled(true);
        mLineEdit->setStyleSheet("background:white;");
    } else {
        acceptButton->setEnabled(false);
        mLineEdit->setStyleSheet("background:#FFB3C8;");
    }
}

UBFeaturesProgressInfo::UBFeaturesProgressInfo(QWidget *parent) :
    QWidget(parent),
    mProgressBar(0),
    mCommonInfoLabel(0),
    mDetailedInfoLabel(0)
{
    QVBoxLayout *mainLayer = new QVBoxLayout(this);

    mProgressBar = new QProgressBar(this);
//    setting defaults
    mProgressBar->setMinimum(0);
    mProgressBar->setMaximum(100000);
    mProgressBar->setValue(0);

    mProgressBar->setStyleSheet("background:white");

    mCommonInfoLabel = new QLabel(this);
    mDetailedInfoLabel = new QLabel(this);
    mDetailedInfoLabel->setAlignment(Qt::AlignRight);
    mCommonInfoLabel->hide();
    mDetailedInfoLabel->hide();

    mainLayer->addWidget(mCommonInfoLabel);
    mainLayer->addWidget(mDetailedInfoLabel);
    mainLayer->addWidget(mProgressBar);
}

void UBFeaturesProgressInfo::setCommmonInfoText(const QString &str)
{
    mProgressBar->setFormat(tr("Loading ") + str + " (%p%)");
}

void UBFeaturesProgressInfo::setDetailedInfoText(const QString &str)
{
    mDetailedInfoLabel->setText(str);
}

void UBFeaturesProgressInfo::setProgressMax(int pValue)
{
    mProgressBar->setMaximum(pValue);
}

void UBFeaturesProgressInfo::setProgressMin(int pValue)
{
    mProgressBar->setMinimum(pValue);
}

void UBFeaturesProgressInfo::increaseProgressValue()
{
    mProgressBar->setValue(mProgressBar->value() + 1);
}

void UBFeaturesProgressInfo::sendFeature(UBFeature pFeature)
{
    Q_UNUSED(pFeature);
}


UBFeaturesWebView::UBFeaturesWebView(QWidget* parent, const char* name):QWidget(parent)
    , mpView(NULL)
    , mpLayout(NULL)
    , mpSankoreAPI(NULL)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpView = new QWebEngineView(this);
    mpView->setObjectName("SearchEngineView");
    mpSankoreAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene());
    QWebChannel* channel = new QWebChannel(this);
    mpView->page()->setWebChannel(channel);
    mpView->page()->webChannel()->registerObject("sankore", mpSankoreAPI);
    UBWebController::injectScripts(mpView);

    mpLayout->addWidget(mpView);
    mpLayout->setContentsMargins(0, 0, 0, 0);
}

UBFeaturesWebView::~UBFeaturesWebView()
{
    if( NULL != mpSankoreAPI )
    {
        delete mpSankoreAPI;
        mpSankoreAPI = NULL;
    }
    if( NULL != mpView )
    {
        delete mpView;
        mpView = NULL;
    }
    if( NULL != mpLayout )
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBFeaturesWebView::showElement(const UBFeature &elem)
{
    QString qsWidgetName;
    QString path = elem.getFullPath().toLocalFile();

    QString qsConfigPath = QString("%0/config.xml").arg(path);

    if(QFile::exists(qsConfigPath))
    {
        QFile f(qsConfigPath);
        if(f.open(QIODevice::ReadOnly))
        {
            QDomDocument domDoc;
            domDoc.setContent(QString(f.readAll()));
            QDomElement root = domDoc.documentElement();

            QDomNode node = root.firstChild();
            while(!node.isNull())
            {
                if(node.toElement().tagName() == "content")
                {
                    QDomAttr srcAttr = node.toElement().attributeNode("src");
                    qsWidgetName = srcAttr.value();
                    break;
                }
                node = node.nextSibling();
            }
            f.close();
        }
    }

    mpView->load(QUrl::fromLocalFile(QString("%0/%1").arg(path).arg(qsWidgetName)));
}


UBFeatureProperties::UBFeatureProperties( QWidget *parent, const char *name ) : QWidget(parent)
    , mpLayout(NULL)
    , mpButtonLayout(NULL)
    , mpAddPageButton(NULL)
    , mpAddToLibButton(NULL)
    , mpObjInfoLabel(NULL)
    , mpObjInfos(NULL)
    , mpThumbnail(NULL)
    , mpOrigPixmap(NULL)
    , mpElement(NULL)
{
    setObjectName(name);

    // Create the GUI
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    maxThumbHeight = height() / 4;

    mpThumbnail = new QLabel();
    QPixmap icon(":images/libpalette/notFound.png");
    icon.scaledToWidth(THUMBNAIL_WIDTH);

    mpThumbnail->setPixmap(icon);
    mpThumbnail->setObjectName("DockPaletteWidgetBox");
    mpThumbnail->setStyleSheet("background:white;");
    mpThumbnail->setAlignment(Qt::AlignHCenter);
    mpLayout->addWidget(mpThumbnail, 0);

    mpButtonLayout = new QHBoxLayout();
    mpLayout->addLayout(mpButtonLayout, 0);

    mpAddPageButton = new UBFeatureItemButton();
    mpAddPageButton->setText(tr("Add to page"));
    mpButtonLayout->addWidget(mpAddPageButton);

    mpAddToLibButton = new UBFeatureItemButton();
    mpAddToLibButton->setText(tr("Add to library"));
    mpButtonLayout->addWidget(mpAddToLibButton);

    mpButtonLayout->addStretch(1);

    mpObjInfoLabel = new QLabel(tr("Object informations"));
    mpObjInfoLabel->setStyleSheet(QString("color: #888888; font-size : 18px; font-weight:bold;"));
    mpLayout->addWidget(mpObjInfoLabel, 0);

    mpObjInfos = new QTreeWidget(this);
    mpObjInfos->setColumnCount(2);
    mpObjInfos->header()->hide();
    mpObjInfos->setAlternatingRowColors(true);
    mpObjInfos->setRootIsDecorated(false);
    mpObjInfos->setObjectName("DockPaletteWidgetBox");
    mpObjInfos->setStyleSheet("background:white;");
    mpLayout->addWidget(mpObjInfos, 1);
    mpLayout->setContentsMargins(0, 0, 0, 0);

    connect( mpAddPageButton, SIGNAL(clicked()), this, SLOT(onAddToPage()) );
    connect( mpAddToLibButton, SIGNAL( clicked() ), this, SLOT(onAddToLib() ) );
}

UBFeatureProperties::~UBFeatureProperties()
{
    if ( mpOrigPixmap )
    {
        delete mpOrigPixmap;
        mpOrigPixmap = NULL;
    }
    if ( mpElement )
    {
        delete mpElement;
        mpElement = NULL;
    }
    if ( mpThumbnail )
    {
        delete mpThumbnail;
        mpThumbnail = NULL;
    }
    if ( mpButtonLayout )
    {
        delete mpButtonLayout;
        mpButtonLayout = NULL;
    }
    if ( mpAddPageButton )
    {
        delete mpAddPageButton;
        mpAddPageButton = NULL;
    }
    if ( mpAddToLibButton )
    {
        delete mpAddToLibButton;
        mpAddToLibButton = NULL;
    }
    if ( mpObjInfoLabel )
    {
        delete mpObjInfoLabel;
        mpObjInfoLabel = NULL;
    }
    if ( mpObjInfos )
    {
        delete mpObjInfos;
        mpObjInfos = NULL;
    }
}

void UBFeatureProperties::resizeEvent( QResizeEvent *event )
{
    Q_UNUSED(event);
    adaptSize();
}

void UBFeatureProperties::showEvent (QShowEvent *event )
{
    Q_UNUSED(event);
    adaptSize();
}

UBFeature UBFeatureProperties::getCurrentElement() const
{
    if ( mpElement )
        return *mpElement;

    return UBFeature();
}

void UBFeatureProperties::setOrigPixmap(const QPixmap &pix)
{

    if (mpOrigPixmap)
        delete mpOrigPixmap;

    mpOrigPixmap = new QPixmap(pix);
}

void UBFeatureProperties::setThumbnail(const QPixmap &pix)
{
    mpThumbnail->setPixmap(pix.scaledToWidth(THUMBNAIL_WIDTH));
    adaptSize();
}

void UBFeatureProperties::adaptSize()
{
    if( NULL != mpOrigPixmap )
    {
        if( width() < THUMBNAIL_WIDTH + 40 )
        {
            mpThumbnail->setPixmap( mpOrigPixmap->scaledToWidth( width() - 40 ) );
        }
        else
        {
            mpThumbnail->setPixmap( mpOrigPixmap->scaledToWidth( THUMBNAIL_WIDTH ) );
        }
    }
}

void UBFeatureProperties::showElement(const UBFeature &elem)
{
    if ( mpOrigPixmap )
    {
        delete mpOrigPixmap;
        mpOrigPixmap = NULL;
    }
    if ( mpElement )
    {
        delete mpElement;
        mpElement = NULL;
    }
    mpElement = new UBFeature(elem);
    mpOrigPixmap = new QPixmap(QPixmap::fromImage(elem.getThumbnail()));
    mpThumbnail->setPixmap(QPixmap::fromImage(elem.getThumbnail()).scaledToWidth(THUMBNAIL_WIDTH));
    populateMetadata();

    if ( UBApplication::isFromWeb( elem.getFullPath().toString() ) )
    {
        mpAddToLibButton->show();
    }
    else
    {
        mpAddToLibButton->hide();
    }
}

void UBFeatureProperties::populateMetadata()
{
    if(NULL != mpObjInfos){
        mpObjInfos->clear();
        QMap<QString, QString> metas = mpElement->getMetadata();
        QList<QString> lKeys = metas.keys();
        QList<QString> lValues = metas.values();

        for(int i=0; i< metas.size(); i++){
            QStringList values;
            values << lKeys.at(i);
            values << lValues.at(i);
            mpItem = new QTreeWidgetItem(values);
            mpObjInfos->addTopLevelItem(mpItem);
        }
        mpObjInfos->resizeColumnToContents(0);
    }
}

void UBFeatureProperties::onAddToPage()
{
    QWidget *w = parentWidget()->parentWidget()->parentWidget();
    UBFeaturesWidget* featuresWidget = qobject_cast<UBFeaturesWidget*>( w );
    if (featuresWidget)
        featuresWidget->getFeaturesController()->addItemToPage( *mpElement );
}

void UBFeatureProperties::onAddToLib()
{
    if ( UBApplication::isFromWeb(  mpElement->getFullPath().toString() ) )
    {
        sDownloadFileDesc desc;
        desc.isBackground = false;
        desc.modal = false;
        desc.dest = sDownloadFileDesc::library;
        desc.name = mpElement->getMetadata().value("Title", QString());
        qDebug() << desc.name;
        desc.srcUrl = mpElement->getFullPath().toString();
        QString str1 = mpElement->getFullPath().toString().normalized(QString::NormalizationForm_C);
        QString str2 = mpElement->getFullPath().toString().normalized(QString::NormalizationForm_D);
        QString str3 = mpElement->getFullPath().toString().normalized(QString::NormalizationForm_KC);
        QString str4 = mpElement->getFullPath().toString().normalized(QString::NormalizationForm_KD);
        qDebug() << desc.srcUrl << '\n'
                    << "str1" << str1 << '\n'
                    << "str2" << str2 << '\n'
                    << "str3" << str3 << '\n'
                    << "str4" << str4 << '\n';
        UBDownloadManager::downloadManager()->addFileToDownload(desc);
    }
}


void UBFeatureProperties::onSetAsBackground()
{
    QWidget *w = parentWidget()->parentWidget()->parentWidget();
    UBFeaturesWidget* featuresWidget = qobject_cast<UBFeaturesWidget*>( w );
    featuresWidget->getFeaturesController()->addItemAsBackground( *mpElement );
}



UBFeatureItemButton::UBFeatureItemButton(QWidget *parent, const char *name):QPushButton(parent)
{
    setObjectName(name);
    setStyleSheet(QString("background-color : #DDDDDD; color : #555555; border-radius : 6px; padding : 5px; font-weight : bold; font-size : 12px;"));
}

UBFeatureItemButton::~UBFeatureItemButton()
{
}

QVariant UBFeaturesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return featuresList->at(index.row()).getDisplayName();
    }

    else if (role == Qt::DecorationRole) {
        return QIcon( QPixmap::fromImage(featuresList->at(index.row()).getThumbnail()));

    } else if (role == Qt::UserRole) {
        return featuresList->at(index.row()).getVirtualPath();

    }    else if (role == Qt::UserRole + 1) {
        //return featuresList->at(index.row()).getType();
        UBFeature f = featuresList->at(index.row());
        return QVariant::fromValue( f );
    }

    return QVariant();
}

QMimeData* UBFeaturesModel::mimeData(const QModelIndexList &indexes) const
{
    UBFeaturesMimeData *mimeData = new UBFeaturesMimeData();
    QList <QUrl> urlList;
    QList <UBFeature> featuresList;
    QByteArray typeData;

    foreach (QModelIndex index, indexes) {

        if (index.isValid()) {
            UBFeature element = data(index, Qt::UserRole + 1).value<UBFeature>();
            urlList.push_back( element.getFullPath() );
            QString curPath = element.getFullPath().toLocalFile();
            featuresList.append(element);

            if (!typeData.isNull()) {
                typeData += UBFeaturesController::featureTypeSplitter();
            }
            typeData += QString::number(element.getType()).toLatin1();
        }
    }

    mimeData->setUrls(urlList);
    mimeData->setFeatures(featuresList);
    mimeData->setData(mimeSankoreFeatureTypes, typeData);

    return mimeData;
}

bool UBFeaturesModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)

    const UBFeaturesMimeData *fMimeData = qobject_cast<const UBFeaturesMimeData*>(mimeData);
    UBFeaturesController *curController = qobject_cast<UBFeaturesController *>(QObject::parent());

    bool dataFromSameModel = false;

    if (fMimeData)
        dataFromSameModel = true;

    if ((!mimeData->hasUrls() && !mimeData->hasImage()) )
        return false;
    if ( action == Qt::IgnoreAction )
        return true;
    if ( column > 0 )
        return false;

    UBFeature parentFeature;
    if (!parent.isValid()) {
        parentFeature = curController->getCurrentElement();
    } else {
        parentFeature = parent.data( Qt::UserRole + 1).value<UBFeature>();
    }

    if (dataFromSameModel) {
        QList<UBFeature> featList = fMimeData->features();
        for (int i = 0; i < featList.count(); i++) {
            UBFeature sourceElement;
            if (dataFromSameModel) {
                sourceElement = featList.at(i);
                moveData(sourceElement, parentFeature, Qt::MoveAction, true);
            }
        }
    } else if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        foreach (QUrl curUrl, urlList) {
            qDebug() << "URl catched is " << curUrl.toLocalFile();
            curController->moveExternalData(curUrl, parentFeature);
        }
    } else if (mimeData->hasImage()) {
        QImage image = qvariant_cast<QImage>( mimeData->imageData() );
        curController->importImage( image, parentFeature );

    }

    return true;
}

void UBFeaturesModel::addItem( const UBFeature &item )
{
    beginInsertRows( QModelIndex(), featuresList->size(), featuresList->size() );
    featuresList->append( item );
    endInsertRows();
}

void UBFeaturesModel::deleteFavoriteItem( const QString &path )
{
    for ( int i = 0; i < featuresList->size(); ++i )
    {
        if ( !QString::compare( featuresList->at(i).getFullPath().toString(), path, Qt::CaseInsensitive ) &&
            !QString::compare( featuresList->at(i).getVirtualPath(), "/root/favorites", Qt::CaseInsensitive ) )
        {
            removeRow( i, QModelIndex() );
            return;
        }
    }
}

void UBFeaturesModel::deleteItem( const QString &path )
{
    for ( int i = 0; i < featuresList->size(); ++i )
    {
        if ( !QString::compare( featuresList->at(i).getFullPath().toString(), path, Qt::CaseInsensitive ) )
        {
            removeRow( i, QModelIndex() );
            return;
        }
    }
}

void UBFeaturesModel::deleteItem(const UBFeature &feature)
{
    int i = featuresList->indexOf(feature);
    if (i == -1) {
        qDebug() << "no matches in deleting item from UBFEaturesModel";
        return;
    }
    removeRow(i, QModelIndex());
}

bool UBFeaturesModel::removeRows( int row, int count, const QModelIndex & parent )
{
    if ( row < 0 )
        return false;
    if ( row + count > featuresList->size() )
        return false;
    beginRemoveRows( parent, row, row + count - 1 );
    //featuresList->remove( row, count );
    featuresList->erase( featuresList->begin() + row, featuresList->begin() + row + count );
    endRemoveRows();
    return true;
}

bool UBFeaturesModel::removeRow(  int row, const QModelIndex & parent )
{
    if ( row < 0 )
        return false;
    if ( row >= featuresList->size() )
        return false;
    beginRemoveRows( parent, row, row );
    //featuresList->remove( row );
    featuresList->erase( featuresList->begin() + row );
    endRemoveRows();
    return true;
}

void UBFeaturesModel::moveData(const UBFeature &source, const UBFeature &destination
                               , Qt::DropAction action = Qt::CopyAction, bool deleteManualy)
{
    UBFeaturesController *curController = qobject_cast<UBFeaturesController *>(QObject::parent());
    if (!curController)
        return;

    QString sourcePath = source.getFullPath().toLocalFile();
    QString sourceVirtualPath = source.getVirtualPath();

    UBFeatureElementType sourceType = source.getType();
    QImage sourceIcon = source.getThumbnail();

    if (sourceType == FEATURE_INTERNAL) {
        qWarning() << "Built-in tools cannot be moved";
        return;
    }

    Q_ASSERT( QFileInfo( sourcePath ).exists() );

    QString name = QFileInfo( sourcePath ).fileName();
    QString destPath = destination.getFullPath().toLocalFile();

    QString destVirtualPath = destination.getFullVirtualPath();
    QString destFullPath = destPath + "/" + name;

    if ( sourcePath.compare(destFullPath, Qt::CaseInsensitive ) || destination.getType() != FEATURE_TRASH)
    {
        UBFileSystemUtils::copy(sourcePath, destFullPath);
        if (action == Qt::MoveAction) {
            curController->deleteItem( source.getFullPath() );
        }
    }

    //Passing all the source container ubdating dependancy pathes
    if (sourceType == FEATURE_FOLDER) {
        for (int i = 0; i < featuresList->count(); i++) {

            UBFeature &curFeature = (*featuresList)[i];

            QString curFeatureFullPath = curFeature.getFullPath().toLocalFile();
            QString curFeatureVirtualPath = curFeature.getVirtualPath();

            if (curFeatureFullPath.contains(sourcePath) && curFeatureFullPath != sourcePath) {

                UBFeature copyFeature = curFeature;
                QUrl newPath = QUrl::fromLocalFile(curFeatureFullPath.replace(sourcePath, destFullPath));
                QString newVirtualPath = curFeatureVirtualPath.replace(sourceVirtualPath, destVirtualPath);
                //when copying to trash don't change the real path
                if (destination.getType() != FEATURE_TRASH) {
                    // processing copy or move action for real FS
                    if (action == Qt::CopyAction) {
                        copyFeature.setFullPath(newPath);
                    } else {
                        curFeature.setFullPath(newPath);
                    }
                }
                // processing copy or move action for virtual FS
                if (action == Qt::CopyAction) {
                    copyFeature.setFullVirtualPath(newVirtualPath);
                } else {
                    curFeature.setFullVirtualPath(newVirtualPath);
                }

                if (action == Qt::CopyAction) {
                    addItem(copyFeature);
                }
            }
        }
    }

    UBFeature newElement( destVirtualPath + "/" + name, sourceIcon, name, QUrl::fromLocalFile(destFullPath), sourceType );
    addItem(newElement);

    if (deleteManualy) {
        deleteItem(source);
    }

// Commented because of crashes on mac. But works fine. It is not predictable behavior. 
// Please uncomment it if model will not refreshes
//   emit dataRestructured();. 
}

Qt::ItemFlags UBFeaturesModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags resultFlags = QAbstractItemModel::flags(index);
    if ( index.isValid() )
    {
        UBFeature item = index.data( Qt::UserRole + 1 ).value<UBFeature>();
        if ( item.getType() == FEATURE_INTERACTIVE
             || item.getType() == FEATURE_ITEM
             || item.getType() == FEATURE_AUDIO
             || item.getType() == FEATURE_VIDEO
             || item.getType() == FEATURE_IMAGE
             || item.getType() == FEATURE_FLASH
             || item.getType() == FEATURE_INTERNAL
             || item.getType() == FEATURE_FOLDER)

            resultFlags |= Qt::ItemIsDragEnabled;

        if ( item.isFolder() && !item.getVirtualPath().isNull() )
            resultFlags |= Qt::ItemIsDropEnabled;
    }

    return resultFlags;
}


QStringList UBFeaturesModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << "image/png" << "image/tiff" << "image/gif" << "image/jpeg";
    return types;
}

int UBFeaturesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !featuresList)
        return 0;
    else
        return featuresList->size();
}

bool UBFeaturesProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString path = index.data( Qt::UserRole ).toString();

    return filterRegularExpression().match(path).hasMatch();
}

bool UBFeaturesSearchProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    /*QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
    eUBLibElementType type = (eUBLibElementType)sourceModel()->data(index, Qt::UserRole + 1).toInt();*/

    UBFeature feature = sourceModel()->data(index, Qt::UserRole + 1).value<UBFeature>();
    bool isFile = feature.getType() == FEATURE_INTERACTIVE
            || feature.getType() == FEATURE_INTERNAL
            || feature.getType() == FEATURE_ITEM
            || feature.getType() == FEATURE_AUDIO
            || feature.getType() == FEATURE_VIDEO
            || feature.getType() == FEATURE_IMAGE;

    return isFile
            && feature.getFullVirtualPath().contains(mFilterPrefix)
            && filterRegularExpression().match( feature.getName() ).hasMatch();
}

bool UBFeaturesPathProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    UBFeature feature = sourceModel()->data(index, Qt::UserRole + 1).value<UBFeature>();

    // We want to display parent folders up to and including the current one
    return (feature.isFolder()
            && ( path.startsWith(feature.getFullVirtualPath() + "/")
                 || path == feature.getFullVirtualPath()));

}

QString    UBFeaturesItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
{
    Q_UNUSED(locale)

    QString text = value.toString();
    text = text.replace(".wgt", "");
    text = text.replace(".wgs", "");
    text = text.replace(".swf","");
    if (listView)
    {
        const QFontMetrics fm = listView->fontMetrics();
        const QSize iSize = listView->gridSize();
        return fm.elidedText( text, Qt::ElideRight, iSize.width() );
    }
    return text;
}

UBFeaturesPathItemDelegate::UBFeaturesPathItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    arrowPixmap = new QPixmap(":images/navig_arrow.png");
}

QString    UBFeaturesPathItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
{
    Q_UNUSED(value)
    Q_UNUSED(locale)

    return QString();
}

void UBFeaturesPathItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    UBFeature feature = index.data( Qt::UserRole + 1 ).value<UBFeature>();
    QRect rect = option.rect;
    if ( !feature.getFullPath().isEmpty() )
    {
        painter->drawPixmap( rect.left() - 10, rect.center().y() - 5, *arrowPixmap );
    }
    painter->drawImage( rect.left() + 5, rect.center().y() - 5, feature.getThumbnail().scaledToHeight( 30, Qt::SmoothTransformation ) );
}

UBFeaturesPathItemDelegate::~UBFeaturesPathItemDelegate()
{
    if ( arrowPixmap )
    {
        delete arrowPixmap;
        arrowPixmap = NULL;
    }
}
