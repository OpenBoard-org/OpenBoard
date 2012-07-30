#ifndef UBFEATURESACTIONBAR_H
#define UBFEATURESACTIONBAR_H

#include <QWidget>
#include <QToolButton>
#include <QDropEvent>
#include "UBLibActionBar.h"
#include "board/UBFeaturesController.h"

class UBFeaturesMimeData;

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
    void deleteElements(const UBFeaturesMimeData *data);
    void addToFavorite(const UBFeaturesMimeData *data);
    void removeFromFavorite(const UBFeaturesMimeData *data);
    void addElementsToFavorite();
    void removeElementsFromFavorite();
    void deleteSelectedElements();
    void rescanModel();

private slots:
	void onSearchTextChanged(QString txt);
	void onActionNewFolder();
    void onActionFavorite();
    void onActionRemoveFavorite();
    void onActionTrash();
    void onActionRescanModel();

protected:
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
    QAction* mpRescanModelAction;
    QAction* mpDeleteAction;
    QAction* mpSearchAction;
    QAction* mpCloseAction;
    QAction* mpRemoveFavorite;
    QAction* mpNewFolderAction;
    UBActionButton* mpFavoriteBtn;
    UBActionButton* mpSocialBtn;
    UBActionButton* mpRescanModelBtn;
    UBActionButton* mpDeleteBtn;
    UBActionButton* mpCloseBtn;
    UBActionButton* mpRemoveFavoriteBtn;
    UBActionButton* mpNewFolderBtn;
};

#endif
