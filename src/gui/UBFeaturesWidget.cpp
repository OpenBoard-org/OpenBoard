#include <QDomDocument>

#include "UBFeaturesWidget.h"
#include "domain/UBAbstractWidget.h"
#include "gui/UBThumbnailWidget.h"
#include "gui/UBLibraryWidget.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBDownloadManager.h"
#include "globals/UBGlobals.h"
#include "board/UBBoardController.h"

UBFeaturesWidget::UBFeaturesWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
{
    setObjectName(name);
    mName = "FeaturesWidget";
    mVisibleState = true;

	SET_STYLE_SHEET();
    //setAttribute(Qt::WA_StyledBackground, true);
    //setStyleSheet(UBApplication::globalStyleSheet());

    mIconToLeft = QPixmap(":images/library_open.png");
    mIconToRight = QPixmap(":images/library_close.png");
    setAcceptDrops(true);
	
	stackedWidget = new QStackedWidget(this);
	layout = new QVBoxLayout(this);

	controller = new UBFeaturesController(this);

	featuresModel = new UBFeaturesModel(this);
	featuresModel->setFeaturesList( controller->getFeatures() );
	featuresModel->setSupportedDragActions( Qt::CopyAction | Qt::MoveAction );
	featuresListView = new UBFeaturesListView(this);
	pathListView = new UBFeaturesListView(this);


	featuresProxyModel = new UBFeaturesProxyModel(this);
	featuresProxyModel->setFilterFixedString( controller->getRootPath() );
	featuresProxyModel->setSourceModel( featuresModel );
	featuresProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

	featuresSearchModel = new UBFeaturesSearchProxyModel(this);
	featuresSearchModel->setSourceModel( featuresModel );
	featuresSearchModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

	featuresPathModel = new UBFeaturesPathProxyModel(this);
	featuresPathModel->setPath( controller->getRootPath() );
	featuresPathModel->setSourceModel( featuresModel );
	

	//featuresListView->setStyleSheet( QString("background: #EEEEEE;border-radius: 10px;border: 2px solid #999999;") );
	featuresListView->setDragDropMode( QAbstractItemView::DragDrop );
	featuresListView->setSelectionMode( QAbstractItemView::ContiguousSelection );
	featuresListView->setModel( featuresProxyModel );

	featuresListView->setResizeMode( QListView::Adjust );
	featuresListView->setViewMode( QListView::IconMode );
	itemDelegate = new UBFeaturesItemDelegate( this, featuresListView );
	featuresListView->setItemDelegate( itemDelegate );
	//featuresListView->setSelectionRectVisible(false);

	featuresListView->setIconSize( QSize(defaultThumbnailSize, defaultThumbnailSize) );
	featuresListView->setGridSize( QSize(defaultThumbnailSize * 1.75, defaultThumbnailSize * 1.75) );

	//pathListView->setStyleSheet( QString("background: #EEEEEE; border-radius : 10px; border : 2px solid #999999;") );
	pathListView->setModel( featuresPathModel );
	pathListView->setViewMode( QListView::IconMode );
	pathListView->setIconSize( QSize(defaultThumbnailSize - 10, defaultThumbnailSize - 10) );
	pathListView->setGridSize( QSize(defaultThumbnailSize + 10, defaultThumbnailSize - 10) );
	pathListView->setFixedHeight( 60 );
	pathItemDelegate = new UBFeaturesPathItemDelegate( this );
	pathListView->setItemDelegate( pathItemDelegate );
	pathListView->setSelectionMode( QAbstractItemView::NoSelection );
	pathListView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pathListView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	pathListView->setFlow( QListView::LeftToRight );
	pathListView->setWrapping(false);
	
	//pathListView->setResizeMode( QListView::Adjust );
	//pathListView->setMovement( QListView::Static );
	pathListView->setDragDropMode( QAbstractItemView::DropOnly );

	pathScene = new QGraphicsScene(this);
	//pathViewer = new UBFeaturesPathViewer( QPixmap(":images/libpalette/home.png"), controller->getRootPath(), pathScene,  this );
	featureProperties = new UBFeatureProperties(this);
	webView = new UBFeaturesWebView(this);
	
	//layout->addWidget( pathViewer );
	//pathViewer->show();
	//layout->addWidget( featuresListView );
	layout->addWidget( pathListView );
	layout->addWidget( stackedWidget );

	stackedWidget->addWidget( featuresListView );
	stackedWidget->addWidget( featureProperties );
	stackedWidget->addWidget( webView );
	stackedWidget->setCurrentIndex(ID_LISTVIEW);
    currentStackedWidget = ID_LISTVIEW;

	mActionBar = new UBFeaturesActionBar(controller, this);
	thumbSlider = new QSlider( Qt::Horizontal, featuresListView );
	thumbSlider->setMinimum( minThumbnailSize );
	thumbSlider->setMaximum( maxThumbnailSize );
	thumbSlider->setValue( defaultThumbnailSize );
	//qDebug() << "init" << featuresListView->height();
	thumbSlider->move( 0, featuresListView->height()  );
	thumbSlider->resize( thumbSlider->width(), thumbSlider->height() + 4 );
	thumbSlider->show();
	featuresListView->installEventFilter(this);
	//layout->addWidget( thumbSlider );
	layout->addWidget( mActionBar );

	/*connect(featuresListView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex &, const QModelIndex & )),
		this, SLOT(currentSelected(const QModelIndex &)));*/
	connect( featuresListView, SIGNAL(clicked ( const QModelIndex & ) ),
		this, SLOT( currentSelected(const QModelIndex &) ) );
	connect( mActionBar, SIGNAL( searchElement(const QString &) ), this, SLOT( searchStarted(const QString &) ) );
	connect( mActionBar, SIGNAL( newFolderToCreate() ), this, SLOT( createNewFolder()  ) );
	connect( mActionBar, SIGNAL( deleteElements(const QMimeData &) ), this, SLOT( deleteElements(const QMimeData &) ) ); 
	connect( mActionBar, SIGNAL( addToFavorite(const QMimeData &) ), this, SLOT( addToFavorite(const QMimeData &) ) );
	connect( mActionBar, SIGNAL( removeFromFavorite(const QMimeData &) ), this, SLOT( removeFromFavorite(const QMimeData &) ) );
	connect( pathListView, SIGNAL(clicked( const QModelIndex & ) ),
		this, SLOT( currentPathChanged( const QModelIndex & ) ) );
	connect( thumbSlider, SIGNAL( sliderMoved(int) ), this, SLOT(thumbnailSizeChanged( int ) ) );
	connect( UBApplication::boardController, SIGNAL( displayMetadata( QMap<QString,QString> ) ), 
		this, SLOT( onDisplayMetadata( QMap<QString,QString> ) ) );
}

bool UBFeaturesWidget::eventFilter( QObject *target, QEvent *event )
{
	if ( target == featuresListView && event->type() == QEvent::Resize )
	{
		thumbSlider->move( 10, featuresListView->height() - thumbSlider->height() - 10 );
		thumbSlider->resize( featuresListView->width() - 20, thumbSlider->height() );
		//qDebug() << featuresListView->height();
		//return true;
	}
	return UBDockPaletteWidget::eventFilter(target, event);
}

void UBFeaturesWidget::searchStarted( const QString &pattern )
{
	if ( pattern.isEmpty() )
	{
		featuresListView->setModel( featuresProxyModel );
		featuresProxyModel->invalidate();
	}
	else if ( pattern.size() > 2 )
	{
		featuresSearchModel->setFilterWildcard( "*" + pattern + "*" );
		featuresListView->setModel( featuresSearchModel );
		featuresSearchModel->invalidate();
	}
}

void UBFeaturesWidget::currentSelected(const QModelIndex &current)
{
	if (current.isValid())
	{
		QSortFilterProxyModel *model = dynamic_cast<QSortFilterProxyModel *>( featuresListView->model() );
		/*QString name = model->data(current).toString();
		QString path = model->data(current, Qt::UserRole).toString();
		eUBLibElementType type = (eUBLibElementType)model->data(current, Qt::UserRole + 1).toInt();*/
		UBFeature feature = model->data(current, Qt::UserRole + 1).value<UBFeature>();

		if ( feature.isFolder() )
		{
			QString newPath = feature.getFullVirtualPath();
			//pathViewer->addPathElement( feature.getThumbnail(), newPath );
			controller->setCurrentElement( feature );

			model->setFilterFixedString( newPath );
			model->invalidate();
			switchToListView();

			featuresPathModel->setPath( newPath );
			featuresPathModel->invalidate();
			if ( feature.getType() == FEATURE_FAVORITE )
			{
				mActionBar->setCurrentState( IN_FAVORITE );
			}
			else if (feature.getType() == FEATURE_TRASH)
			{
				mActionBar->setCurrentState( IN_TRASH );
			}
			else
			{
				mActionBar->setCurrentState( IN_FOLDER );
			}
		}
		else if ( feature.getType() == FEATURE_SEARCH )
		{
			webView->showElement( feature );
			switchToWebView();
		}
		else
		{
			featureProperties->showElement( feature );
			switchToProperties();
			mActionBar->setCurrentState( IN_PROPERTIES );
		}
		
	}
}

void UBFeaturesWidget::currentPathChanged(const QModelIndex &index)
{
	if ( index.isValid() )	
	{
		UBFeature feature = featuresPathModel->data(index, Qt::UserRole + 1).value<UBFeature>();
		QString newPath = feature.getFullVirtualPath();

		featuresPathModel->setPath( newPath );
		featuresPathModel->invalidate();

		featuresListView->setModel( featuresProxyModel );
		featuresProxyModel->setFilterFixedString(newPath);
		featuresProxyModel->invalidate();
		switchToListView();
		controller->setCurrentElement( feature );
		if ( feature.getType() == FEATURE_CATEGORY && feature.getName() == "root" )
		{
			mActionBar->setCurrentState( IN_ROOT );
		}
		else if (feature.getType() == FEATURE_FAVORITE)
		{
			mActionBar->setCurrentState( IN_FAVORITE );
		}
		else if (feature.getType() == FEATURE_TRASH)
		{
			mActionBar->setCurrentState( IN_TRASH );
		}
		else
		{
			mActionBar->setCurrentState( IN_FOLDER );
		}
	}
}

void UBFeaturesWidget::createNewFolder()
{
	UBNewFolderDlg dlg;
    if(QDialog::Accepted == dlg.exec())
    {
		UBFeature newFolder = controller->newFolder( dlg.folderName() );
		featuresModel->addItem( newFolder );
		featuresProxyModel->invalidate();
    }
}

void UBFeaturesWidget::deleteElements( const QMimeData & mimeData )
{
	if ( !mimeData.hasUrls() )
		return;
	QList<QUrl> urls = mimeData.urls();
	
	foreach ( QUrl url, urls )
	{
		if ( controller->isTrash( url ) )
		{
			controller->deleteItem( url );
		}
		else
		{
			UBFeature elem = controller->moveItemToFolder( url, controller->getTrashElement() );
			controller->removeFromFavorite( url );
			featuresModel->addItem( elem );
			featuresModel->deleteFavoriteItem( UBFeaturesController::fileNameFromUrl( url ) );
		}
	}
	QSortFilterProxyModel *model = dynamic_cast<QSortFilterProxyModel *>( featuresListView->model() );
	model->invalidate();
}

void UBFeaturesWidget::addToFavorite( const QMimeData & mimeData )
{
	if ( !mimeData.hasUrls() )
		return;
	QList<QUrl> urls = mimeData.urls();
	
	foreach ( QUrl url, urls )
	{
		UBFeature elem = controller->addToFavorite( url );
		if ( !elem.getVirtualPath().isEmpty() && !elem.getVirtualPath().isNull() )
			featuresModel->addItem( elem );
	}
	QSortFilterProxyModel *model = dynamic_cast<QSortFilterProxyModel *>( featuresListView->model() );
	model->invalidate();
}

void UBFeaturesWidget::removeFromFavorite( const QMimeData & mimeData )
{
	if ( !mimeData.hasUrls() )
		return;
	QList<QUrl> urls = mimeData.urls();
	foreach( QUrl url, urls )
	{
		controller->removeFromFavorite( url );
	}
}

void UBFeaturesWidget::thumbnailSizeChanged( int value )
{
	featuresListView->setIconSize( QSize( value, value ) );
	featuresListView->setGridSize( QSize( value * 1.75, value * 1.75 ) );
}

void UBFeaturesWidget::onDisplayMetadata( QMap<QString,QString> metadata )
{
	UBFeature feature( QString(), QPixmap(":images/libpalette/notFound.png"), QString(), metadata["Url"], FEATURE_ITEM );
	feature.setMetadata( metadata );

	featureProperties->showElement( feature );
	switchToProperties();
	mActionBar->setCurrentState( IN_PROPERTIES );
}

void UBFeaturesWidget::switchToListView()
{
	stackedWidget->setCurrentIndex(ID_LISTVIEW);
	currentStackedWidget = ID_LISTVIEW;
}

void UBFeaturesWidget::switchToProperties()
{
	stackedWidget->setCurrentIndex(ID_PROPERTIES);
	currentStackedWidget = ID_PROPERTIES;
}

void UBFeaturesWidget::switchToWebView()
{
	stackedWidget->setCurrentIndex(ID_WEBVIEW);
	currentStackedWidget = ID_WEBVIEW;
}

/*

void UBFeaturesWidget::currentPathChanged(const QString &path)
{
	int newDepth = path.count("/");
	pathViewer->truncatePath(newDepth);
	featuresListView->setModel( featuresProxyModel );
	featuresProxyModel->setFilterFixedString(path);
	featuresProxyModel->invalidate();
	switchToListView();
}
*/


UBFeaturesWidget::~UBFeaturesWidget()
{
}

UBFeaturesListView::UBFeaturesListView( QWidget* parent, const char* name ) 
: QListView(parent)
{
	setObjectName(name);
	//rubberBand = new UBRubberBand( QRubberBand::Rectangle, this ); 
}

/*
void UBFeaturesListView::mousePressEvent( QMouseEvent *event )
{
	rubberOrigin = event->pos();
	rubberBand->setGeometry( QRect( rubberOrigin, QSize() ) );
	//qDebug()  << rubberOrigin.x() << rubberOrigin.y();
	rubberBand->show();
	QListView::mousePressEvent(event);
}

void UBFeaturesListView::mouseMoveEvent( QMouseEvent *event )
{
	QPoint current = event->pos();
	rubberBand->setGeometry( QRect( rubberOrigin, current ).normalized() );

	//setSelection( rubberBand->rect(), QItemSelectionModel::Select );
	QListView::mouseMoveEvent(event);
}

void UBFeaturesListView::mouseReleaseEvent( QMouseEvent *event )
{
	rubberBand->hide();
	QListView::mouseReleaseEvent(event);
}

*/
void UBFeaturesListView::dragEnterEvent( QDragEnterEvent *event )
{
	if ( event->mimeData()->hasUrls() )
		event->acceptProposedAction();
}

void UBFeaturesListView::dropEvent( QDropEvent *event )
{
	if( event->source() || dynamic_cast<UBFeaturesListView *>( event->source() ) )
	{
		event->setDropAction( Qt::MoveAction );
	}
	QListView::dropEvent( event );
}


UBFeaturesWebView::UBFeaturesWebView(QWidget* parent, const char* name):QWidget(parent)
    , mpView(NULL)
    , mpWebSettings(NULL)
    , mpLayout(NULL)
    , mpSankoreAPI(NULL)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpView = new QWebView(this);
    mpView->setObjectName("SearchEngineView");
    mpSankoreAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene());
    mpView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mpSankoreAPI);

    mpWebSettings = QWebSettings::globalSettings();
    mpWebSettings->setAttribute(QWebSettings::JavaEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::PluginsEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    mpWebSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, true);

    mpLayout->addWidget(mpView);

    connect(mpView, SIGNAL(loadFinished(bool)), this, SLOT(onLoadFinished(bool)));
}

UBFeaturesWebView::~UBFeaturesWebView()
{
    if(NULL != mpSankoreAPI){
        delete mpSankoreAPI;
        mpSankoreAPI = NULL;
    }
    if(NULL != mpView){
        delete mpView;
        mpView = NULL;
    }
    if(NULL != mpLayout){
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

void UBFeaturesWebView::onLoadFinished(bool ok)
{
    if(ok && NULL != mpSankoreAPI){
        mpView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mpSankoreAPI);
    }
}


UBFeatureProperties::UBFeatureProperties( QWidget *parent, const char *name ) : QWidget(parent)
    , mpLayout(NULL)
    , mpButtonLayout(NULL)
    , mpAddPageButton(NULL)
    , mpAddToLibButton(NULL)
    , mpSetAsBackgroundButton(NULL)
    , mpObjInfoLabel(NULL)
    , mpThumbnail(NULL)
    , mpOrigPixmap(NULL)
    , mpElement(NULL)
    , mpObjInfos(NULL)
{
	setObjectName(name);

    SET_STYLE_SHEET();
    //setStyleSheet(UBApplication::globalStyleSheet());

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

    mpSetAsBackgroundButton = new UBFeatureItemButton();
    mpSetAsBackgroundButton->setText(tr("Set as background"));
    mpButtonLayout->addWidget(mpSetAsBackgroundButton);

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

	connect( mpAddPageButton, SIGNAL(clicked()), this, SLOT(onAddToPage()) );
    connect( mpSetAsBackgroundButton, SIGNAL( clicked() ), this, SLOT( onSetAsBackground() ) );

}

void UBFeatureProperties::showElement( const UBFeature &elem )
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
    mpElement = new UBFeature( elem );
	mpOrigPixmap = new QPixmap( elem.getThumbnail() );
	mpThumbnail->setPixmap(elem.getThumbnail().scaledToWidth(THUMBNAIL_WIDTH));
    populateMetadata();

    if ( UBApplication::isFromWeb( elem.getFullPath().toString() ) )
	{
        mpAddToLibButton->show();
		if( elem.getMetadata()["Type"].toLower().contains("image") )
		{
            mpSetAsBackgroundButton->show();
        }
		else
		{
            mpSetAsBackgroundButton->hide();
        }
    }
	else
	{
        mpAddToLibButton->hide();
        if (UBFileSystemUtils::mimeTypeFromFileName( elem.getFullPath().toLocalFile() ).contains("image"))
		{
            mpSetAsBackgroundButton->show();
        }
	    else
		{
            mpSetAsBackgroundButton->hide();
        }
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
	QWidget *w = parentWidget()->parentWidget();
    UBFeaturesWidget* featuresWidget = dynamic_cast<UBFeaturesWidget*>( w );
    featuresWidget->getFeaturesController()->addItemToPage( *mpElement );
}

void UBFeatureProperties::onSetAsBackground()
{
    QWidget *w = parentWidget()->parentWidget();
    UBFeaturesWidget* featuresWidget = dynamic_cast<UBFeaturesWidget*>( w );
    featuresWidget->getFeaturesController()->addItemAsBackground( *mpElement );
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

	if (role == Qt::DisplayRole)
		return featuresList->at(index.row()).getName();
	else if (role == Qt::DecorationRole)
	{
		return QIcon( featuresList->at(index.row()).getThumbnail() );
	}
	else if (role == Qt::UserRole)
	{
		return featuresList->at(index.row()).getVirtualPath();
	}
	else if (role == Qt::UserRole + 1)
	{
		//return featuresList->at(index.row()).getType();
		UBFeature f = featuresList->at(index.row());
		return QVariant::fromValue( f );
	}

	return QVariant();
}

QMimeData* UBFeaturesModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *mimeData = new QMimeData();
	QList <QUrl> urlList;

	foreach (QModelIndex index, indexes)
	{
		if ( index.isValid() )
		{
			UBFeature element = data( index, Qt::UserRole + 1 ).value<UBFeature>();
            urlList.push_back( element.getFullPath() );
			/*if ( element.getType() == FEATURE_INTERNAL )
			{
				urlList.push_back( QUrl( element.getFullPath() ) );
			}
			else if ( element.getType() == FEATURE_INTERACTIVE || element.getType() == FEATURE_ITEM )
			{
				urlList.push_back( element.getFullPath() );
			}*/
		}
	}
	mimeData->setUrls( urlList );

    return mimeData;
}

bool UBFeaturesModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row)

    if ( !mimeData->hasUrls() )
		return false;
	if ( action == Qt::IgnoreAction )
		return true;
	if ( column > 0 )
		return false;

    int endRow = 0;

	UBFeature parentFeature;
    if ( !parent.isValid() )
	{
		parentFeature = dynamic_cast<UBFeaturesWidget *>(QObject::parent())->getFeaturesController()->getCurrentElement();
    } 
	else
	{
		parentFeature = parent.data( Qt::UserRole + 1).value<UBFeature>();
	}

	QList<QUrl> urls = mimeData->urls();
	
	foreach ( QUrl url, urls )
	{
		UBFeature element;
		
		if ( action == Qt::MoveAction )
		{
			element = dynamic_cast<UBFeaturesWidget *>(QObject::parent())->getFeaturesController()->moveItemToFolder( url, parentFeature );
		}
		else
		{
			element = dynamic_cast<UBFeaturesWidget *>(QObject::parent())->getFeaturesController()->copyItemToFolder( url, parentFeature );
		}
		addItem( element );
	}
	return true;
}

void UBFeaturesModel::addItem( const UBFeature &item )
{
	beginInsertRows( QModelIndex(), featuresList->size(), featuresList->size() );
	featuresList->push_back( item );
	endInsertRows();
}

void UBFeaturesModel::deleteFavoriteItem( const QString &path )
{
	for ( int i = 0; i < featuresList->size(); ++i )
	{
		if ( !QString::compare( featuresList->at(i).getUrl(), path, Qt::CaseInsensitive ) &&
			!QString::compare( featuresList->at(i).getVirtualPath(), "/root/favorites", Qt::CaseInsensitive ) )
		{
			removeRow( i, QModelIndex() );
			return;
		}
	}
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

Qt::ItemFlags UBFeaturesModel::flags( const QModelIndex &index ) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
	if ( index.isValid() )
	{
		UBFeature item = index.data( Qt::UserRole + 1 ).value<UBFeature>();
        if ( item.getType() == FEATURE_INTERACTIVE ||
            item.getType() == FEATURE_ITEM ||
			item.getType() == FEATURE_INTERNAL )
			return Qt::ItemIsDragEnabled | defaultFlags;
		if ( item.isFolder() && !item.getVirtualPath().isNull() )
			return defaultFlags | Qt::ItemIsDropEnabled;
		else return defaultFlags | Qt::ItemIsDropEnabled;
	}
	/*if ( index.isValid() )
	{
		UBFeature item = index.data( Qt::UserRole + 1 ).value<UBFeature>();
		switch( item.getType() )
		{
		case FEATURE_CATEGORY:
		case FEATURE_FOLDER:
		case FEATURE_FAVORITE:
		case FEATURE_TRASH:
			return Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
		case FEATURE_INTERACTIVE:
		case FEATURE_INTERNAL:
		case FEATURE_ITEM:		
			return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		default:;
		}
	}*/
	return defaultFlags | Qt::ItemIsDropEnabled;
}


QStringList UBFeaturesModel::mimeTypes() const
{
	QStringList types;
    types << "text/uri-list";
    return types;
}

int UBFeaturesModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
        return 0;
    else
        return featuresList->size();
}


bool UBFeaturesProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	QString path = index.data( Qt::UserRole ).toString();
	
	return filterRegExp().exactMatch(path);
}

bool UBFeaturesSearchProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	/*QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
	eUBLibElementType type = (eUBLibElementType)sourceModel()->data(index, Qt::UserRole + 1).toInt();*/

	UBFeature feature = sourceModel()->data(index, Qt::UserRole + 1).value<UBFeature>();
    bool isFile = feature.getType() == FEATURE_INTERACTIVE ||
		feature.getType() == FEATURE_INTERNAL ||
        feature.getType() == FEATURE_ITEM;
	
	return isFile && filterRegExp().exactMatch( feature.getName() );
}

bool UBFeaturesPathProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	/*QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
	eUBLibElementType type = (eUBLibElementType)sourceModel()->data(index, Qt::UserRole + 1).toInt();*/

	UBFeature feature = sourceModel()->data(index, Qt::UserRole + 1).value<UBFeature>();
	
	return feature.isFolder() && path.startsWith( feature.getFullVirtualPath() );
}

QString	UBFeaturesItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
{
    Q_UNUSED(locale)

    QString text = value.toString();
	if (listView)
	{
		const QFontMetrics fm = listView->fontMetrics();
		const QSize iSize = listView->iconSize();
		return elidedText( fm, iSize.width(), Qt::ElideRight, text );
	}
	return text;
}

UBFeaturesPathItemDelegate::UBFeaturesPathItemDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{
	arrowPixmap = new QPixmap(":images/navig_arrow.png");
}

QString	UBFeaturesPathItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
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
	painter->drawPixmap( rect.left() + 5, rect.center().y() - 5, feature.getThumbnail().scaledToHeight( 30, Qt::SmoothTransformation ) );
}

UBFeaturesPathItemDelegate::~UBFeaturesPathItemDelegate()
{
	if ( arrowPixmap )
	{
		delete arrowPixmap;
		arrowPixmap = NULL;
	}
}
