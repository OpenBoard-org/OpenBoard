#include "UBFeaturesWidget.h"
#include "domain/UBAbstractWidget.h"
#include "gui/UBThumbnailWidget.h"
#include "gui/UBLibraryWidget.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBDownloadManager.h"
#include "globals/UBGlobals.h"

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
	featuresListView->setModel( featuresProxyModel );

	featuresListView->setResizeMode( QListView::Adjust );
	featuresListView->setViewMode( QListView::IconMode );
	itemDelegate = new UBFeaturesItemDelegate( this, featuresListView );
	featuresListView->setItemDelegate( itemDelegate );

	featuresListView->setIconSize( QSize(40, 40) );
	featuresListView->setGridSize( QSize(70, 70) );

	//pathListView->setStyleSheet( QString("background: #EEEEEE; border-radius : 10px; border : 2px solid #999999;") );
	pathListView->setModel( featuresPathModel );
	pathListView->setViewMode( QListView::IconMode );
	pathListView->setIconSize( QSize(30, 30) );
	pathListView->setGridSize( QSize(50, 30) );
	pathListView->setFixedHeight( 60 );
	pathItemDelegate = new UBFeaturesPathItemDelegate( this );
	pathListView->setItemDelegate( pathItemDelegate );
	pathListView->setSelectionMode( QAbstractItemView::NoSelection );
	pathListView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    pathListView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	//pathListView->setMovement( QListView::Static );
	pathListView->setDragDropMode( QAbstractItemView::DragDrop );

	pathScene = new QGraphicsScene(this);
	//pathViewer = new UBFeaturesPathViewer( QPixmap(":images/libpalette/home.png"), controller->getRootPath(), pathScene,  this );
	featureProperties = new UBFeatureProperties(this);
	
	//layout->addWidget( pathViewer );
	//pathViewer->show();
	//layout->addWidget( featuresListView );
	layout->addWidget( pathListView );
	layout->addWidget( stackedWidget );

	stackedWidget->addWidget( featuresListView );
	stackedWidget->addWidget( featureProperties );
	stackedWidget->setCurrentIndex(ID_LISTVIEW);
    currentStackedWidget = ID_LISTVIEW;

	mActionBar = new UBFeaturesActionBar(controller, this);
	layout->addWidget(mActionBar);

	/*connect(featuresListView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex &, const QModelIndex & )),
		this, SLOT(currentSelected(const QModelIndex &)));*/
	connect( featuresListView, SIGNAL(clicked ( const QModelIndex & ) ),
		this, SLOT( currentSelected(const QModelIndex &) ) );
	connect( mActionBar, SIGNAL( searchElement(const QString &) ), this, SLOT( const searchStarted(QString &) ) );
	connect( mActionBar, SIGNAL( newFolderToCreate() ), this, SLOT( createNewFolder()  ) );
	connect( mActionBar, SIGNAL( deleteElements(const QMimeData &) ), this, SLOT( deleteElements(const QMimeData &) ) ); 
	connect( mActionBar, SIGNAL( addToFavorite(const QMimeData &) ), this, SLOT( addToFavorite(const QMimeData &) ) );
	connect( mActionBar, SIGNAL( removeFromFavorite(const QMimeData &) ), this, SLOT( removeFromFavorite(const QMimeData &) ) );
	connect( pathListView, SIGNAL(clicked( const QModelIndex & ) ),
		this, SLOT( currentPathChanged( const QModelIndex & ) ) );
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
			QString newPath = feature.getUrl() + "/" + feature.getName();
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
			else
			{
				mActionBar->setCurrentState( IN_FOLDER );
			}
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
		QString newPath = feature.getUrl() + "/" + feature.getName();

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
		if ( !elem.getUrl().isEmpty() && !elem.getUrl().isNull() )
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

UBFeaturesListView::UBFeaturesListView( QWidget* parent, const char* name ) : QListView(parent)
{
	setObjectName(name);
}

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

	connect(mpAddPageButton, SIGNAL(clicked()), this, SLOT(onAddToPage()));

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
        //populateMetadata();

    if ( UBApplication::isFromWeb( elem.getUrl() ) )
	{
        mpAddToLibButton->show();
        /*if(elem->metadatas()["Type"].toLower().contains("image"))
		{
            mpSetAsBackgroundButton->show();
        }
		else
		{
            mpSetAsBackgroundButton->hide();
        }*/
    }
	else
	{
        mpAddToLibButton->hide();
        if (UBFileSystemUtils::mimeTypeFromFileName( elem.getUrl() ).contains("image"))
		{
            mpSetAsBackgroundButton->show();
        }
		else
		{
            mpSetAsBackgroundButton->hide();
        }
    }
}

void UBFeatureProperties::onAddToPage()
{
    if ( UBApplication::isFromWeb( mpElement->getUrl() ) )
	{
        sDownloadFileDesc desc;
        desc.isBackground = false;
        desc.modal = true;
        desc.name = QFileInfo( mpElement->getName() ).fileName();
        desc.url = mpElement->getUrl();
        UBDownloadManager::downloadManager()->addFileToDownload(desc);

    }
	else
	{
		QWidget *w = parentWidget()->parentWidget();
        UBFeaturesWidget* featuresWidget = dynamic_cast<UBFeaturesWidget*>( w );
        featuresWidget->getFeaturesController()->addItemToPage( *mpElement );
    }
}

UBFeatureProperties::~UBFeatureProperties()
{
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
		return featuresList->at(index.row()).getUrl();
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
			if ( element.getType() == FEATURE_INTERNAL )
			{
				urlList.push_back( QUrl( element.getFullPath() ) );
			}
			else if ( element.getType() == FEATURE_INTERACTIVE || element.getType() == FEATURE_ITEM )
			{
				urlList.push_back( QUrl::fromLocalFile(element.getFullPath()) );
			}
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

    if ( !parent.isValid() )
	{
		return false;
        /*if (row < 0)
            endRow = featuresList->size();
        else
            endRow = qMin( row, featuresList->size() );*/
    } 
	else
        endRow = parent.row();

	UBFeature parentFeature = parent.data( Qt::UserRole + 1).value<UBFeature>();

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
		if ( !QString::compare( featuresList->at(i).getFullPath(), path, Qt::CaseInsensitive ) &&
			!QString::compare( featuresList->at(i).getUrl(), "/root/favorites", Qt::CaseInsensitive ) )
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
	featuresList->remove( row, count );
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
	featuresList->remove( row );
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
		if ( item.isFolder() && !item.getFullPath().isNull() )
			return defaultFlags | Qt::ItemIsDropEnabled;
		else return defaultFlags;
	}
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
	QString virtualFullPath = feature.getUrl() + "/" + feature.getName();
	
	return feature.isFolder() && path.startsWith( virtualFullPath );
}

QString	UBFeaturesItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
{
    Q_UNUSED(locale)

    QString text = value.toString();
	if (listView)
	{
		const QFontMetrics fm = listView->fontMetrics();
		const QSize iSize = listView->iconSize();

		if ( iSize.width() > 0 && fm.width(text) > iSize.width() )
		{
			while (fm.width(text) > iSize.width())
				text.resize(text.size()-1);
			text += "...";
		}
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
