#ifndef UBFEATURESACTIONBAR_H
#define UBFEATURESACTIONBAR_H

#include <QWidget>
#include <QToolButton>
#include "UBLibActionBar.h"
#include "board/UBFeaturesController.h"

enum UBFeaturesActionBarState
{
	IN_ROOT,
	IN_FOLDER,
	IN_PROPERTIES,
	IN_FAVORITE
};

class UBFeaturesActionBar : public QWidget
{
	Q_OBJECT
public:
	UBFeaturesActionBar(UBFeaturesController *controller, QWidget* parent=0, const char* name="UBFeaturesActionBar");
    ~UBFeaturesActionBar();
	
	void setCurrentState( UBFeaturesActionBarState state );
signals:
	void searchElement(QString text);
	void newFolderToCreate();
private slots:
	void onSearchTextChanged(QString txt);
	void onActionNewFolder();
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