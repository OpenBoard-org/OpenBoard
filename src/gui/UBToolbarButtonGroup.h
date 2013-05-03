/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBTOOLBARBUTTONGROUP_H_
#define UBTOOLBARBUTTONGROUP_H_


#include <QtGui>

class UBToolbarButtonGroup : public QWidget
{
    Q_OBJECT;

    public:
        UBToolbarButtonGroup(QToolBar *toolbar, const QList<QAction*> &actions = QList<QAction*>());
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

    public slots:
        void setCurrentIndex(int index);
        void colorPaletteChanged();
        void displayText(QVariant display);

    private slots:
        void selected(QAction *action);

    signals:
        void activated(int index);
        void currentIndexChanged(int index);
};

#endif /* UBTOOLBARBUTTONGROUP_H_ */
