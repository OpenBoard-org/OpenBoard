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
#include "core/UBSettings.h"
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
class UBFeaturesNavigatorWidget;
class UBFeaturesMimeData;

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
    UBFeaturesController * getFeaturesController() const { return controller; }
    void importImage(const QImage &image, const QString &fileName = QString());

	static const int minThumbnailSize = 20;
	static const int maxThumbnailSize = 100;
	static const int defaultThumbnailSize = 40;

public:
    int scrollbarHorisontalPadding() const { return 10;}
    int scrollbarVerticalIndent() const { return 0;}

private slots:
    void onPreviewLoaded(int id, bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground);
    void currentSelected( const QModelIndex & );
    void searchStarted( const QString & );
    void createNewFolder();
    void deleteElements( const UBFeaturesMimeData * );
    void addToFavorite( const UBFeaturesMimeData  *);
    void removeFromFavorite( const UBFeaturesMimeData * );
    void onDisplayMetadata( QMap<QString,QString> );
    void onAddDownloadedFileToLibrary(bool, QUrl, QString, QByteArray);
    void addElementsToFavorite();
    void removeElementsFromFavorite();
    void deleteSelectedElements();
    void rescanModel();

private:
    void switchToListView();
    void switchToProperties();
    void switchToWebView();

private:
    UBFeaturesController *controller;
    UBFeaturesNavigatorWidget *mNavigator;
    UBFeaturesListView *pathListView;
    QVBoxLayout *layout;
    UBFeaturesActionBar *mActionBar;
    UBFeatureProperties *featureProperties;
    UBFeaturesWebView *webView;
    QStackedWidget *stackedWidget;
    int currentStackedWidget;
    UBDownloadHttpFile* imageGatherer;

};


class UBFeaturesMimeData : public QMimeData
{
    Q_OBJECT

public:
    virtual QStringList formats() const;
    QList<UBFeature> features() const {return mFeatures;}
    void setFeatures(const QList<UBFeature> &fList) {mFeatures = fList;}

private:
    QList<UBFeature> mFeatures;
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
    virtual void dragMoveEvent( QDragMoveEvent *event );

private slots:
    void thumbnailSizeChanged(int);
};


// class created to have the same style for slider and QListView itself
class UBFeaturesNavigatorWidget: public QWidget
{
    Q_OBJECT

public:
    UBFeaturesNavigatorWidget(QWidget *parent, const char* name = "");
    UBFeaturesListView *listView() {return mListView;}
    void setSliderPosition(int pValue);

private:
    UBFeaturesListView *mListView;
    QSlider *mListSlder;

};


class UBFeaturesWebView : public QWidget
{
    Q_OBJECT
public:
    UBFeaturesWebView(QWidget* parent = 0, const char* name = "UBFeaturesWebView");
    virtual ~UBFeaturesWebView();

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
    UBFeature getCurrentElement() const;
    void setOrigPixmap(QPixmap &pix);
    void setThumbnail(QPixmap &pix);

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onAddToPage();
    void onAddToLib();
    void onSetAsBackground();
    //void onBack();

private:
	void populateMetadata();
    void adaptSize();

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonLayout;
    UBFeatureItemButton *mpAddPageButton;
    UBFeatureItemButton *mpAddToLibButton;
    UBFeatureItemButton *mpSetAsBackgroundButton;
    QLabel* mpObjInfoLabel;
    QTreeWidget* mpObjInfos;
    QLabel* mpThumbnail;
    QPixmap* mpOrigPixmap;
    int maxThumbHeight;
    UBFeature *mpElement;
    QTreeWidgetItem* mpItem;
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

signals:
    void dataRestructured();

public:
    UBFeaturesModel(QList<UBFeature> *pFeaturesList, QObject *parent = 0) : QAbstractListModel(parent), featuresList(pFeaturesList) {;}
    virtual ~UBFeaturesModel(){;}

	void addItem( const UBFeature &item );
	void deleteFavoriteItem( const QString &path );
    void deleteItem( const QString &path );
    void deleteItem(const UBFeature &feature);

	QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
	QMimeData *mimeData( const QModelIndexList &indexes ) const;
	QStringList mimeTypes() const;
	int rowCount( const QModelIndex &parent ) const;
	Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent);
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
	bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    //bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    //bool insertRow(int row, const QModelIndex &parent = QModelIndex());

    void moveData(const UBFeature &source, const UBFeature &destination, Qt::DropAction action, bool deleteManualy = false);
    Qt::DropActions supportedDropActions() const { return Qt::MoveAction | Qt::CopyAction; }
//    void setFeaturesList(QList <UBFeature> *flist ) { featuresList = flist; }

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
    UBFeaturesItemDelegate(QObject *parent = 0, const QListView *lw = 0) : QStyledItemDelegate(parent) { listView = lw; }
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
    UBFeaturesPathItemDelegate(QObject *parent = 0);
	~UBFeaturesPathItemDelegate();
	virtual QString	displayText ( const QVariant & value, const QLocale & locale ) const;
	void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
	QPixmap *arrowPixmap;
};

#endif // UBFEATURESWIDGET_H
