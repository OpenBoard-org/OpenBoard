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
#ifndef UBLIBNAVIGATORWIDGET_H
#define UBLIBNAVIGATORWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QSlider>
#include <QDropEvent>

#include "UBLibraryWidget.h"
#include "core/UBSetting.h"

class UBLibNavigatorWidget : public QWidget
{
    Q_OBJECT
public:
    UBLibNavigatorWidget(QWidget* parent=0, const char* name="UBLibNavigatorWidget");
    ~UBLibNavigatorWidget();
    void dropMe(const QMimeData* _data);

    UBLibraryWidget* libraryWidget(){return mLibWidget;}

signals:
    void propertiesRequested(UBLibElement* elem);
    void displaySearchEngine(UBLibElement* elem);
    void updateNavigBar(UBChainedLibElement* elem);

private slots:
    void onNavigbarUpate(UBLibElement* pElem);
    void onPathItemClicked(UBChainedLibElement *elem);
    void onPropertiesRequested(UBLibElement* elem);
    void updateThumbnailsSize(int newSize);
    void onDisplaySearchEngine(UBLibElement* elem);

private:
    void removeNextChainedElements(UBChainedLibElement* fromElem);

    QVBoxLayout* mLayout;
    UBLibraryWidget* mLibWidget;
    QSlider* mSlider;
    UBSetting* mSliderWidthSetting;
};

#endif // UBLIBNAVIGATORWIDGET_H
