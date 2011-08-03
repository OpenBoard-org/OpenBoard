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
#ifndef UBNAVIGATORPALETTE_H
#define UBNAVIGATORPALETTE_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimerEvent>
#include <QLabel>
#include <QString>

#include "UBDockPalette.h"
#include "UBDocumentNavigator.h"
#include "document/UBDocumentProxy.h"


class UBNavigatorPalette : public UBDockPalette
{
    Q_OBJECT
public:
    UBNavigatorPalette(QWidget* parent=0, const char* name="navigatorPalette");
    ~UBNavigatorPalette();

    void setDocument(UBDocumentProxy* document);
    void refresh();

public slots:
    void setPageNumber(int current, int total);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void timerEvent(QTimerEvent *event);

private:
    void updateTime();

    /** The thumbnails navigator widget */
    UBDocumentNavigator* mNavigator;
    /** The layout */
    QVBoxLayout* mLayout;
    QHBoxLayout* mHLayout;
    QLabel* mPageNbr;
    QLabel* mClock;
    QString mTimeFormat;
    int mTimerID;

private slots:
    void changeCurrentPage();
};


#endif // UBNAVIGATORPALETTE_H
