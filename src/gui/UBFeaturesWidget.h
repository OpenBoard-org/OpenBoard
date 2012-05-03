#ifndef UBFEATURESWIDGET_H
#define UBFEATURESWIDGET_H

#include <QWidget>
#include <QListView>
#include <QGraphicsView>
#include <QAbstractListModel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QSlider>
#include <QUrl>
#include <QStyledItemDelegate>
#include <QLocale>
#include <QGraphicsLinearLayout>
#include <QStackedWidget>
#include <QDropEvent>

#include "UBDockPaletteWidget.h"
//#include "UBLibActionBar.h"
#include "board/UBFeaturesController.h"
#include "api/UBWidgetUniboardAPI.h"
#include "UBFeaturesActionBar.h"
#include "UBRubberBand.h"


#define THUMBNAIL_WIDTH 400
#define ID_LISTVIEW 0
#define ID_PROPERTIES 1
#define ID_WEBVIEW 2

class UBListModel;

class UBFeaturesModel;
class UBFeaturesItemDelegate;
class UBFeaturesPathItemDelegate;
class UBFeaturesProxyModel;
class UBFeaturesSearchProxyModel;
class UBFeaturesPathProxyModel;
class UBFeaturesPathViewer;
class UBFeatureProperties;
class UBFeatureItemButton;
class UBFeaturesListView;
class UBFeaturesWebView;

class UBFeaturesWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
	UBFeaturesWidget(QWidget* parent=0, const char* name="UBFeaturesWidget");
    virtual ~UBFeaturesWidget();

	bool visibleInMode(eUBDockPaletteWidgetMode mode)
    {
        return mode == eUBDockPaletteWidget_BOARD
            || mode == eUBDockPaletteWidget_DESKTOP;
    }
	UBFeaturesController * getFeaturesController()const { return controller; };

	static const int minThumbnailSize = 20;
	static const int maxThumbnailSize = 100;
	static const int defaultThumbnailSize = 40;
private:
	void switchToListView();
	void switchToProperties();
	void switchToWebView();

	UBFeaturesController *controller;
	
	UBFeaturesItemDelegate *itemDelegate;
	UBFeaturesPathItemDelegate *pathItemDelegate;
	
	UBFeaturesModel *featuresModel;
	UBFeaturesProxyModel *featuresProxyModel;
	UBFeaturesSearchProxyModel *featuresSearchModel;
	UBFeaturesPathProxyModel *featuresPathModel;

	UBFeaturesListView *featuresListView;
	UBFeaturesListView *pathListView;

	QSlider *thumbSlider;
	QVBoxLayout *layout;
	//UBFeaturesPathViewer *pathViewer;
	QGraphicsScene *pathScene;
	UBFeaturesActionBar *mActionBar;
	UBFeatureProperties *featureProperties;
	UBFeaturesWebView *webView;
	QStackedWidget *stackedWidget;
	

	int currentStackedWidget;
	QModelIndex trashIndex;
private slots:
	void currentSelected( const QModelIndex & );
	//void currentPathChanged(const QString &);
	void currentPathChanged( const QModelIndex & );
	void searchStarted( const QString & );
	void createNewFolder();
	void deleteElements( const QMimeData & );
	void addToFavorite( const QMimeData & );
	void removeFromFavorite( const QMimeData & );
	void thumbnailSizeChanged( int );
protected:
	bool eventFilter(QObject *target, QEvent *event);
};

class UBFeaturesListView : public QListView
{
	Q_OBJECT
public:
	UBFeaturesListView( QWidget* parent=0, const char* name="UBFeaturesListView" );
    virtual ~UBFeaturesListView() {;}
protected:
	virtual void dragEnterEvent( QDragEnterEvent *event );
	virtual void dropEvent( QDropEvent *event );
	/*virtual void mousePressEvent( QMouseEvent *event );
	virtual void mouseMoveEvent( QMouseEvent *event );
	virtual void mouseReleaseEvent( QMouseEvent *event );*/
private:
	//UBRubberBand *rubberBand;
	//QPoint rubberOrigin;
};

class UBFeaturesWebView : public QWidget
{
    Q_OBJECT
public:
    UBFeaturesWebView(QWidget* parent = 0, const char* name = "UBFeaturesWebView");
    ~UBFeaturesWebView();

    void showElement(const UBFeature &elem);

private slots:
    void onLoadFinished(bool ok);

private:
    QWebView* mpView;
    QWebSettings* mpWebSettings;
    QVBoxLayout* mpLayout;
    UBWidgetUniboardAPI* mpSankoreAPI;
};

class UBFeatureProperties : public QWidget
{
    Q_OBJECT
public:
    UBFeatureProperties(QWidget* parent=0, const char* name="UBFeatureProperties");
    ~UBFeatureProperties();

    void showElement(const UBFeature &elem);


protected:
    //void resizeEvent(QResizeEvent *event);
    //void showEvent(QShowEvent *event);

private slots:
    void onAddToPage();
    //void onAddToLib();
    //void onSetAsBackground();
    //void onBack();

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonLayout;
    UBFeatureItemButton* mpAddPageButton;
    UBFeatureItemButton* mpAddToLibButton;
    UBFeatureItemButton* mpSetAsBackgroundButton;
    QLabel* mpObjInfoLabel;
    //QTreeWidget* mpObjInfos;
    QLabel* mpThumbnail;
    QPixmap* mpOrigPixmap;
    int maxThumbHeight;
    UBFeature *mpElement;
    //QTreeWidgetItem* mpItem;
};




class UBFeatureItemButton : public QPushButton
{
public:
    UBFeatureItemButton(QWidget* parent=0, const char* name="UBFeatureItemButton");
    ~UBFeatureItemButton();
};

class UBFeaturesModel : public QAbstractListModel
{
	Q_OBJECT
public:
    UBFeaturesModel( QObject *parent = 0 ) : QAbstractListModel(parent) {;}
    virtual ~UBFeaturesModel(){;}

	void addItem( const UBFeature &item );
	void deleteFavoriteItem( const QString &path );

	QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
	QMimeData *mimeData( const QModelIndexList &indexes ) const;
	QStringList mimeTypes() const;
	int rowCount( const QModelIndex &parent ) const;
	Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool removeRow(int row, const QModelIndex &parent = QModelIndex());
	
    Qt::DropActions supportedDropActions() const { return Qt::MoveAction | Qt::CopyAction; }

    void setFeaturesList(QList <UBFeature> *flist ) { featuresList = flist; }
private:
	QList <UBFeature> *featuresList;
};

class UBFeaturesProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
    UBFeaturesProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {;}
    virtual ~UBFeaturesProxyModel() {}
protected:
	virtual bool filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const;
};

class UBFeaturesSearchProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
    UBFeaturesSearchProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {;}
    virtual ~UBFeaturesSearchProxyModel() {}
protected:
	virtual bool filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const;
};

class UBFeaturesPathProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
    UBFeaturesPathProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {;}
    virtual ~UBFeaturesPathProxyModel() {}
    void setPath( const QString &p ) { path = p; }
protected:
	virtual bool filterAcceptsRow ( int sourceRow, const QModelIndex & sourceParent ) const;
private:
	QString path;
};

class UBFeaturesItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
    UBFeaturesItemDelegate(QWidget *parent = 0, const QListView *lw = 0) : QStyledItemDelegate(parent) { listView = lw; }
    ~UBFeaturesItemDelegate() {}
	//UBFeaturesItemDelegate(const QListView *lw = 0) { listView = lw; };
	//void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual QString	displayText ( const QVariant & value, const QLocale & locale ) const;
private:
	const QListView *listView;
};

class UBFeaturesPathItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	UBFeaturesPathItemDelegate(QWidget *parent = 0);
	~UBFeaturesPathItemDelegate();
	virtual QString	displayText ( const QVariant & value, const QLocale & locale ) const;
	void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
	QPixmap *arrowPixmap;
};

#endif // UBFEATURESWIDGET_H
