/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBACTIONPALETTE_H_
#define UBACTIONPALETTE_H_

#include <QtGui>
#include <QPoint>
#include <QButtonGroup>
#include <QToolButton>

#include "UBFloatingPalette.h"

class UBActionPaletteButton;

class UBActionPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBActionPalette(Qt::Orientation orientation, QWidget* parent = 0);
        UBActionPalette(Qt::Corner corner, QWidget* parent = 0, Qt::Orientation orient = Qt::Vertical);
        UBActionPalette(QWidget* parent = 0);

        virtual ~UBActionPalette();

        void setButtonIconSize(const QSize& size);
        void setToolButtonStyle(Qt::ToolButtonStyle);

     // todo (C++ 20):
     // void changeActions(auto& codeBlock);
        template <typename F>
        void changeActions(F codeBlock){
            codeBlock();
            actionChanged();
        }

        QList<QAction*> actions();
        void groupActions();
        virtual UBActionPaletteButton* addAction(QAction* action);

        void setClosable(bool closable);
        void setAutoClose(bool autoClose)
        {
            mAutoClose = autoClose;
        }

        void setCustomCloseProcessing(bool customCloseProcessing)
        {
            m_customCloseProcessing = customCloseProcessing;
        }
        bool m_customCloseProcessing;

        virtual int border();
        virtual void clearLayout();
        QSize buttonSize();

    public slots:
        void close();


    signals:
        void closed();
        void customMouseReleased();

    protected:
        virtual void paintEvent(QPaintEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent * event);
        virtual void init(Qt::Orientation orientation);

        virtual void updateLayout();

        QList<UBActionPaletteButton*> mButtons;
        QButtonGroup* mButtonGroup;
        QList<QAction*> mActions;

        bool mIsClosable;
        Qt::ToolButtonStyle mToolButtonStyle;
        bool mAutoClose;
        QSize mButtonSize;
        QPoint mMousePos;
        UBActionPaletteButton *createPaletteButton(QAction* action, QWidget *parent);

    protected slots:
        void buttonClicked();
        void actionChanged();
};


class UBActionPaletteButton : public QToolButton
{
    Q_OBJECT

    public:
        UBActionPaletteButton(QAction* action, QWidget * parent = 0);
        virtual ~UBActionPaletteButton();

    signals:
        void doubleClicked();

    protected:
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        virtual bool hitButton(const QPoint &pos) const;

};

#endif /* UBACTIONPALETTE_H_ */
