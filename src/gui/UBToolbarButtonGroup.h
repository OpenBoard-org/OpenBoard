/*
 * Copyright (C) 2015-2016 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBTOOLBARBUTTONGROUP_H_
#define UBTOOLBARBUTTONGROUP_H_


#include <QtGui>
#include <QWidget>
#include <QToolBar>
#include <QToolButton>
#include <QActionGroup>
#include <qcolordialog.h>
#include <QSpinBox>

class UBToolbarButtonGroup : public QWidget
{
    Q_OBJECT;

    public:
        UBToolbarButtonGroup(QToolBar *toolbar, const QList<QAction*> &actions = QList<QAction*>(), bool isColorToolBar=false, bool isLineWidthToolBar = false);
        virtual ~UBToolbarButtonGroup();

        void setIcon(const QIcon &icon, int index);
        void setColor(const QColor &color, int index);
        int currentIndex() const;

    protected:
        void paintEvent(QPaintEvent *);

    private:
        QToolButton         *mToolButton; // The first button in the toolbar
        QString              mLabel;
        QList<QAction*>      mActions;
        QList<QToolButton*>  mButtons;
        int                  mCurrentIndex;
        bool                 mDisplayLabel;
        QActionGroup*        mActionGroup;
        QColorDialog        *mColorDialog; // Issue 27/02/2018 - OpenBoard - CUSTOM COLOR.
        bool                 isColorTool;  // Issue 27/02/2018 - OpenBoard - CUSTOM COLOR.
        bool                 isLineWidthTool; // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        QDoubleSpinBox      *lineWidthSpinBox; // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        qreal                customPenLineWidth; // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        qreal                customMarkerLineWidth; // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        bool launched;

    public slots:
        void setCurrentIndex(int index);
        void colorPaletteChanged();
        void displayText(QVariant display);
        void updateCustomColor(QColor selectedColor); // Issue 27/02/2018 - OpenBoard - CUSTOM COLOR.
        void updateLineWidthSpinBox(); // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        void updatePenMarkerWidth(double value); // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH

    private slots:
        void selected(QAction *action);
        void customColorHandle(); // Issue 27/02/2018 - OpenBoard - CUSTOM COLOR.
        void lineWidthHandlePredefined(); // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
        void lineWidthHandleCustom(); // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH

    signals:
        void activated(int index);
        void currentIndexChanged(int index);
        void customColorUpdated();
        void clickOnLineWidthButton(); // Issue 05/03/2018 - OpenBoard - CUSTOM WIDTH
};

#endif /* UBTOOLBARBUTTONGROUP_H_ */
