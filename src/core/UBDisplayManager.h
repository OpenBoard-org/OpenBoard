/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef UBDISPLAYMANAGER_H_
#define UBDISPLAYMANAGER_H_

#include <QtGui>
#include <QMap>

class UBBlackoutWidget;
class UBBoardView;

enum DisplayRole
{
    None = 0, Control, Display, Desktop, Previous1, Previous2, Previous3, Previous4, Previous5
};

class UBDisplayManager : public QObject
{
    Q_OBJECT;

    public:
        UBDisplayManager(QObject *parent = 0);
        virtual ~UBDisplayManager();

        int numScreens();

        int numPreviousViews();

        void setControlWidget(QWidget* pControlWidget);

        void setDisplayWidget(QWidget* pDisplayWidget);

        void setDesktopWidget(QWidget* pDesktopWidget);

        void setPreviousDisplaysWidgets(QList<UBBoardView*> pPreviousViews);

        QWidget* widget(DisplayRole role);

        bool hasControl()
        {
            return mScreensByRole.contains(Control);
        }

        bool hasDisplay()
        {
            return mScreensByRole.contains(Display);
        }

        bool hasPrevious()
        {
            return mScreensByRole.contains(Previous1);
        }

        bool useMultiScreen() { return mUseMultiScreen; }

        void setUseMultiScreen(bool pUse);

        int controleScreenIndex();

        QSize screenSize(DisplayRole role) const;
        QSize availableScreenSize(DisplayRole role) const;
        qreal physicalDpi(DisplayRole role) const;
        qreal logicalDpi(DisplayRole role) const;

        QRect controlGeometry();
        QPixmap grab(DisplayRole role, QRect rect = QRect(0, 0, -1, -1));
        QPixmap grabGlobal(QRect rect);

   signals:

           void screenLayoutChanged();
           void adjustDisplayViewsRequired();

   public slots:

        void reinitScreens(bool bswap);

        void adjustScreens(int screen);

        void blackout();

        void unBlackout();

        void setRoleToScreen(DisplayRole role, int screenIndex);

        void addOrRemoveScreen(QScreen* screen);

    private:

        void positionScreens();

        void initScreenIndexes();

        QList<UBBlackoutWidget*> mBlackoutWidgets;

        QMap<DisplayRole, QScreen*> mScreensByRole;
        QMap<DisplayRole, QWidget*> mWidgetsByRole;

        bool mUseMultiScreen;

};

#endif /* UBDISPLAYMANAGER_H_ */
