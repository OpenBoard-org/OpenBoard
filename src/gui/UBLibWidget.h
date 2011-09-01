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
#ifndef UBLIBWIDGET_H
#define UBLIBWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QLabel>

#include "UBDockPaletteWidget.h"
#include "UBLibNavigatorWidget.h"
#include "UBLibItemProperties.h"
#include "UBLibActionBar.h"

#define ID_NAVIGATOR    0
#define ID_PROPERTIES   1

class UBLibWidget : public UBDockPaletteWidget
{
    Q_OBJECT
public:
    UBLibWidget(QWidget* parent=0, const char* name="UBLibWidget");
    ~UBLibWidget();

    UBLibActionBar* actionBar(){return mActionBar;}
    UBLibNavigatorWidget* libNavigator() {return mNavigator;};

signals:
    void resized();

protected:
    void dragEnterEvent(QDragEnterEvent* pEvent);
    void dropEvent(QDropEvent *pEvent);
    void dragMoveEvent(QDragMoveEvent* pEvent);
    void dragLeaveEvent(QDragLeaveEvent* pEvent);

private slots:
    void showProperties(UBLibElement* elem);
    void showFolder();

private:
    void processMimeData(const QMimeData* pData);
    int customMargin();
    int border();

    /** The layout */
    QVBoxLayout* mLayout;
    /** The stacked layout */
    QStackedWidget* mStackedWidget;
    /** The Navigator widget */
    UBLibNavigatorWidget* mNavigator;
    /** The Properties widget */
    UBLibItemProperties* mProperties;
    /** UBLibActionBar */
    UBLibActionBar* mActionBar;
    /** The current stack widget index*/
    int miCrntStackWidget;
};

#endif // UBLIBWIDGET_H
