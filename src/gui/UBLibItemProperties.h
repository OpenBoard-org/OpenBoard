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
#ifndef UBLIBITEMPROPERTIES_H
#define UBLIBITEMPROPERTIES_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QLabel>
#include <QTextEdit>
#include <QToolButton>
#include <QAction>
#include <QShowEvent>
#include <QTreeWidget>

#include "board/UBLibraryController.h"

#define THUMBNAIL_WIDTH 400

class UBLibItemButton : public QPushButton
{
public:
    UBLibItemButton(QWidget* parent=0, const char* name="UBLibItemButton");
    ~UBLibItemButton();
};

class UBLibItemProperties : public QWidget
{
    Q_OBJECT
public:
    UBLibItemProperties(QWidget* parent=0, const char* name="UBLibItemProperties");
    ~UBLibItemProperties();

    void showElement(UBLibElement* elem);

signals:
    void showFolderContent();

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onAddToPage();
    void onAddToLib();
    void onSetAsBackground();
    void onBack();

private:
    void adaptSize();
    void populateMetadata();

    QVBoxLayout* mpLayout;
    QHBoxLayout* mpButtonLayout;
    UBLibItemButton* mpAddPageButton;
    UBLibItemButton* mpAddToLibButton;
    UBLibItemButton* mpSetAsBackgroundButton;
    QLabel* mpObjInfoLabel;
    QTreeWidget* mpObjInfos;
    QLabel* mpThumbnail;
    QPixmap* mpOrigPixmap;
    int maxThumbHeight;
    UBLibElement* mpElement;
    QTreeWidgetItem* mpItem;
};


#endif // UBLIBITEMPROPERTIES_H
