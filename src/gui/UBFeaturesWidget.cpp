#include "UBFeaturesWidget.h"
#include "domain/UBAbstractWidget.h"
#include "gui/UBThumbnailWidget.h"
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
	featuresListView->setDragDropMode( QAbstractItemView::InternalMove );
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

	mActionBar = new UBLibActionBar(this);
	layout->addWidget(mActionBar);

	/*connect(featuresListView->selectionModel(), SIGNAL(currentChanged ( const QModelIndex &, const QModelIndex & )),
		this, SLOT(currentSelected(const QModelIndex &)));*/
	connect( featuresListView, SIGNAL(clicked ( const QModelIndex & ) ),
		this, SLOT( currentSelected(const QModelIndex &) ) );
	connect( mActionBar, SIGNAL( searchElement(QString) ), this, SLOT( searchStarted(QString) ) );
	connect( pathListView, SIGNAL(clicked( const QModelIndex & ) ),
		this, SLOT( currentPathChanged( const QModelIndex & ) ) );
}

void UBFeaturesWidget::searchStarted( QString pattern )
{
	if ( pattern.isEmpty() )
	{
		featuresListView->setModel( featuresProxyModel );
		featuresProxyModel->invalidate();
	}
	else
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
		if ( feature.getType() == UBFeatureElementType::FEATURE_FOLDER || feature.getType() == UBFeatureElementType::FEATURE_CATEGORY)
		{
			QString newPath = feature.getUrl() + "/" + feature.getName();
			//pathViewer->addPathElement( feature.getThumbnail(), newPath );

			model->setFilterFixedString( newPath );
			model->invalidate();
			switchToListView();

			featuresPathModel->setPath( newPath );
			featuresPathModel->invalidate();
		}
		else
		{
			featureProperties->showElement( feature );
			switchToProperties();
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



void UBFeaturesWidget::currentPathChanged(const QString &path)
{
	int newDepth = path.count("/");
	pathViewer->truncatePath(newDepth);
	featuresListView->setModel( featuresProxyModel );
	featuresProxyModel->setFilterFixedString(path);
	featuresProxyModel->invalidate();
	switchToListView();
}



UBFeaturesWidget::~UBFeaturesWidget()
{
}

UBFeaturesListView::UBFeaturesListView( QWidget* parent, const char* name ) : QListView(parent)
{
	setObjectName(name);
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

UBFeaturesPathViewer::UBFeaturesPathViewer(const QPixmap &root, const QString &rootPath, QGraphicsScene *sc, QWidget* parent, const char* name) : QGraphicsView(sc, parent)
{
	setObjectName(name);

	/*setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());
*/
	layout = new QGraphicsLinearLayout();

	container = new QGraphicsWidget();
	container->setMaximumWidth( width() - 20 );
	container->setLayout( layout );
	scene()->addItem( container );
	
	UBFolderWidget* pIconLabel = new UBFolderWidget();
    pIconLabel->setStyleSheet(QString("background-color: transparent;"));
	pIconLabel->setPixmap( root );
	pIconLabel->setPath(rootPath);
	connect( pIconLabel, SIGNAL( clicked(const QString &) ), parent, SLOT( currentPathChanged(const QString &) ) );

	QGraphicsProxyWidget *iconWidget = scene()->addWidget( pIconLabel ) ;;
	layout->addItem( iconWidget );
	setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	setAlignment( Qt::AlignLeft );
	setFixedHeight( 70 );
	arrowPixmap = new QPixmap(":images/navig_arrow.png");
}

void UBFeaturesPathViewer::addPathElement(const QPixmap &p, const QString &s)
{
	UBFolderWidget* pIconLabel = new UBFolderWidget();
	pIconLabel->setStyleSheet(QString("background-color: transparent;"));
	pIconLabel->setPixmap( *arrowPixmap );

	QGraphicsProxyWidget *iconWidget = scene()->addWidget( pIconLabel );
	layout->addItem( iconWidget );

	
	pIconLabel = new UBFolderWidget();
	
	pIconLabel->setStyleSheet(QString("background-color: transparent;"));
	pIconLabel->setPixmap( p.scaledToHeight( height() - 30, Qt::SmoothTransformation) );
	pIconLabel->setPath(s);
	connect( pIconLabel, SIGNAL( clicked(const QString &) ), parent(), SLOT( currentPathChanged(const QString &) ) );

	iconWidget = scene()->addWidget( pIconLabel );
	layout->addItem( iconWidget );
	scene()->invalidate();
}

void UBFeaturesPathViewer::truncatePath(int number)
{
	QList <QGraphicsItem*> items = scene()->items();
	int itemsToDel = items.size() - number * 2;
	for ( QList <QGraphicsItem*>::iterator it = items.begin() ; it != items.begin() + itemsToDel; ++it )
	{
		scene()->removeItem( (*it) );
		QGraphicsLayoutItem *layoutItem = dynamic_cast<QGraphicsLayoutItem *>(*it);
		Q_ASSERT(layout);
		layout->removeItem(layoutItem);
		delete layoutItem;
	}
	scene()->invalidate();
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
			urlList.push_back( QUrl::fromLocalFile( element.getFullPath() ) );
		}
	}
	mimeData->setUrls( urlList );

    return mimeData;
}

bool UBFeaturesModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if ( !mimeData->hasUrls() )
		return false;
	if ( action == Qt::IgnoreAction )
		return true;
	if ( column > 0 )
		return false;

	int endRow;

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
			element = UBFeaturesController::moveItemToFolder( url, parentFeature );
		}
		else
		{
			element = UBFeaturesController::copyItemToFolder( url, parentFeature );
		}
		beginInsertRows( QModelIndex(), featuresList->size(), featuresList->size() );
		featuresList->push_back( element );
		endInsertRows();
	}
	return true;
}

bool UBFeaturesModel::removeRows( int row, int count, const QModelIndex & parent )
{
	if ( row < 0 )
		return false;
	if ( row + count >= featuresList->size() )
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
		if ( item.getType() == UBFeatureElementType::FEATURE_INTERACTIVE || 
			item.getType() == UBFeatureElementType::FEATURE_ITEM )
			return Qt::ItemIsDragEnabled | defaultFlags;
		if ( item.getType() == UBFeatureElementType::FEATURE_FOLDER ||
			item.getType() == UBFeatureElementType::FEATURE_CATEGORY && item.getFullPath() != "")
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
	bool isFile = feature.getType() == UBFeatureElementType::FEATURE_INTERACTIVE ||
		feature.getType() == UBFeatureElementType::FEATURE_ITEM;
	
	return isFile && filterRegExp().exactMatch( feature.getName() );
}

bool UBFeaturesPathProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex & sourceParent )const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
	/*QString name = sourceModel()->data(index, Qt::DisplayRole).toString();
	eUBLibElementType type = (eUBLibElementType)sourceModel()->data(index, Qt::UserRole + 1).toInt();*/

	UBFeature feature = sourceModel()->data(index, Qt::UserRole + 1).value<UBFeature>();
	bool isFolder = feature.getType() == UBFeatureElementType::FEATURE_CATEGORY ||
		feature.getType() == UBFeatureElementType::FEATURE_FOLDER;
	QString virtualFullPath = feature.getUrl() + "/" + feature.getName();
	
	return isFolder && path.startsWith( virtualFullPath );
}

QString	UBFeaturesItemDelegate::displayText ( const QVariant & value, const QLocale & locale ) const
{
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
	return "";
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