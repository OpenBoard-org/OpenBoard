#ifndef UBFEATURESACTIONBAR_H
#define UBFEATURESACTIONBAR_H

#include <QWidget>
#include <QToolButton>
#include <QDropEvent>
#include "UBLibActionBar.h"
#include "board/UBFeaturesController.h"

enum UBFeaturesActionBarState
{
	IN_ROOT,
	IN_FOLDER,
	IN_PROPERTIES,
	IN_FAVORITE,
	IN_TRASH
};

class UBFeaturesActionBar : public QWidget
{
	Q_OBJECT
public:
	UBFeaturesActionBar(UBFeaturesController *controller, QWidget* parent=0, const char* name="UBFeaturesActionBar");
    ~UBFeaturesActionBar();
	
	void setCurrentState( UBFeaturesActionBarState state );
signals:
	void searchElement(const QString &text);
	void newFolderToCreate();
	void deleteElements( const QMimeData &data );
	void addToFavorite( const QMimeData &data );
	void removeFromFavorite( const QMimeData &data );
private slots:
	void onSearchTextChanged(QString txt);
	void onActionNewFolder();
protected:
	//void dragMoveEvent(QDragMoveEvent *event);
	void dragEnterEvent( QDragEnterEvent *event );
	void dropEvent( QDropEvent *event );
private:
	void setButtons();
	UBFeaturesController *featuresController;
	UBFeaturesActionBarState currentState;

	eButtonSet mCrntButtonSet;
    eButtonSet mPreviousButtonSet;

	QButtonGroup* mButtonGroup;
    QLineEdit* mSearchBar;
    QHBoxLayout* mLayout;
    QAction* mpFavoriteAction;
    QAction* mpSocialAction;
    QAction* mpDeleteAction;
    QAction* mpSearchAction;
    QAction* mpCloseAction;
    QAction* mpRemoveFavorite;
    QAction* mpNewFolderAction;
    UBActionButton* mpFavoriteBtn;
    UBActionButton* mpSocialBtn;
    UBActionButton* mpDeleteBtn;
    //UBActionButton* mpSearchBtn;
    UBActionButton* mpCloseBtn;
    UBActionButton* mpRemoveFavoriteBtn;
    UBActionButton* mpNewFolderBtn;

};


#endif