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

#include "UBTeacherGuideWidgetsTools.h"


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

private:
    QList<QTreeWidgetItem*> getChildrenList(QTreeWidgetItem* widgetItem);
    QVector<tUBGEElementNode*> getPageAndCommentData();

    QVBoxLayout* mpLayout;
    QLabel* mpDocumentTitle;
    UBTGAdaptableText* mpPageTitle;
    UBTGAdaptableText* mpComment;
    QTreeWidget* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    UBAddItem* mpAddAnActionItem;
    UBAddItem* mpAddAMediaItem;
    UBAddItem* mpAddALinkItem;

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

public slots:
    void onAddItemClicked(QTreeWidgetItem* widget, int column);

private:
    void createMediaButtonItem();

    UBTGAdaptableText* mpPageTitle;
    UBTGAdaptableText* mpComment;
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonTitleLayout;
    QLabel* mpDocumentTile;
    QPushButton* mpModePushButton;
    QTreeWidget* mpTreeWidget;
    QTreeWidgetItem* mpRootWidgetItem;
    QTreeWidgetItem* mpMediaSwitchItem;

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

private:
    UBTeacherGuideEditionWidget* mpEditionWidget;
    UBTeacherGuidePresentationWidget* mpPresentationWidget;
    QVector<tUBGEElementNode*>currentData;

};

#endif // UBTEACHERGUIDEWIDGET_H
