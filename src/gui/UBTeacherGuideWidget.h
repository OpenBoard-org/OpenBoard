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

#ifndef UBTEACHERGUIDEWIDGET_H
#define UBTEACHERGUIDEWIDGET_H

class QTreeWidget;
class QHeaderView;
class QLabel;
class QVBoxLayout;
class QPushButton;
class UBDocumentProxy;

#include "UBTeacherGuideWidgetsTools.h"
#include "UBTGWidgetTreeDelegate.h"

typedef enum
{
    tUBTGZeroPageMode_EDITION,
    tUBTGZeroPageMode_PRESENTATION
}tUBTGZeroPageMode;

/***************************************************************************
 *               class    UBTeacherGuideEditionWidget                      *
 ***************************************************************************/
class UBTeacherGuideEditionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UBTeacherGuideEditionWidget(QWidget* parent = 0, const char* name="UBTeacherGuideEditionWidget");
    ~UBTeacherGuideEditionWidget();
    void cleanData();
    QVector<tUBGEElementNode*> getData();

public slots:
    void onAddItemClicked(QTreeWidgetItem* widget, int column);
    void onActiveSceneChanged();
    void showEvent(QShowEvent* event);

private:
    QList<QTreeWidgetItem*> getChildrenList(QTreeWidgetItem* widgetItem);
    QVector<tUBGEElementNode*> getPageAndCommentData();

    QVBoxLayout* mpLayout;
    QLabel* mpDocumentTitle;
    QLabel* mpPageNumberLabel;
    UBTGAdaptableText* mpPageTitle;
    UBTGAdaptableText* mpComment;
    QFrame* mpSeparator;
    QTreeWidget* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    UBAddItem* mpAddAnActionItem;
    UBAddItem* mpAddAMediaItem;
    UBAddItem* mpAddALinkItem;
    UBTGWidgetTreeDelegate* mpTreeDelegate;

};


/***************************************************************************
 *           class    UBTeacherGuidePresentationWidget                     *
 ***************************************************************************/
class UBTeacherGuidePresentationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UBTeacherGuidePresentationWidget(QWidget* parent, const char* name = "UBTeacherGuidePresentationName");
    ~UBTeacherGuidePresentationWidget();
    void showData(QVector<tUBGEElementNode*>data);
    void cleanData();

public slots:
    void onAddItemClicked(QTreeWidgetItem* widget, int column);
    void onActiveSceneChanged();

private:
    bool eventFilter(QObject* object, QEvent* event);

    void createMediaButtonItem();

    UBTGAdaptableText* mpPageTitle;
    UBTGAdaptableText* mpComment;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonTitleLayout;
    QLabel* mpDocumentTitle;
    QLabel* mpPageNumberLabel;
    QFrame* mpSeparator;
    QPushButton* mpModePushButton;
    UBTGDraggableTreeItem* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    QTreeWidgetItem* mpMediaSwitchItem;

};

/***************************************************************************
 *                  class    UBTeacherGuidePageZeroWidget                  *
 ***************************************************************************/
class UBTeacherGuidePageZeroWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UBTeacherGuidePageZeroWidget(QWidget* parent, const char* name = "UBTeacherGuidePageZeroEditionWidget");
    ~UBTeacherGuidePageZeroWidget();

    QVector<tUBGEElementNode*> getData();


public slots:
    void onActiveSceneChanged();
    void switchToMode(tUBTGZeroPageMode mode = tUBTGZeroPageMode_EDITION);

private:
    void fillComboBoxes();
    void loadData();
    void hideEvent(QHideEvent* event);
    bool eventFilter(QObject* object, QEvent* event);

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonTitleLayout;
    QPushButton* mpModePushButton;
    QLabel* mpPageNumberLabel;
    UBTGAdaptableText* mpSessionTitle;
    QFrame* mpSeparatorSessionTitle;

    QLabel* mpAuthorsLabel;
    UBTGAdaptableText* mpAuthors;
    QFrame* mpSeparatorAuthors;

    QLabel* mpCreationLabel;
    QLabel* mpLastModifiedLabel;
    QLabel* mpGoalsLabel;
    UBTGAdaptableText* mpGoals;
    QFrame* mpSeparatorGoals;

    QLabel* mpIndexLabel;
    QLabel* mpKeywordsLabel;
    UBTGAdaptableText* mpKeywords;

    QLabel* mpSchoolLevelItemLabel;
    QComboBox* mpSchoolLevelBox;
    QLabel* mpSchoolLevelValueLabel;

    QLabel* mpSchoolBranchItemLabel;
    QComboBox* mpSchoolBranchBox;
    QLabel* mpSchoolBranchValueLabel;

    QLabel* mpSchoolTypeItemLabel;
    QComboBox* mpSchoolTypeBox;
    QLabel* mpSchoolTypeValueLabel;
    QFrame* mpSeparatorIndex;

    QLabel* mpLicenceLabel;
    QComboBox* mpLicenceBox;
    QLabel* mpLicenceValueLabel;
    QLabel* mpLicenceIcon;
    QHBoxLayout* mpLicenceLayout;

    QMap<QString,QString> mGradeLevelsMap;
    QMap<QString,QStringList> mSubjects;

private slots:
    void onSchoolLevelChanged(QString schoolLevel);
    void persistData();
};

/***************************************************************************
 *                    class    UBTeacherGuideWidget                        *
 ***************************************************************************/

class UBTeacherGuideWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit UBTeacherGuideWidget(QWidget* parent = 0, const char* name="UBTeacherGuideWidget");
    ~UBTeacherGuideWidget();

public slots:
    void changeMode();
    void showPresentationMode();
    void connectToStylusPalette();
    void onActiveSceneChanged();

private:
    UBTeacherGuidePageZeroWidget* mpPageZeroWidget;
    UBTeacherGuideEditionWidget* mpEditionWidget;
    UBTeacherGuidePresentationWidget* mpPresentationWidget;
    QVector<tUBGEElementNode*>mCurrentData;

};

#endif // UBTEACHERGUIDEWIDGET_H
