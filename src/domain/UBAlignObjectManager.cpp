/*
 * Copyright (C) 2010-2014 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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

#include "UBAlignObjectManager.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"
#include "domain/UBItem.h"

UBAlignObjectManager::UBAlignObjectManager()
{
}

void UBAlignObjectManager::alignToLeft() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> leftPoints(selectedItems.size());
        QGraphicsItem* reference = 0;

        QGraphicsItem *item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        //find the left point (for compute the delta at the end of the function)
        int mostLeft = p.boundingRect().left();

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            //find the left point (for compute the delta at the end of the function)
            int left = p.boundingRect().left();

            if(left < mostLeft){
                mostLeft = left;
                reference = item;
            }

            leftPoints[i] = left;
        }

        UBApplication::undoStack->beginMacro("leftAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dx = mostLeft - leftPoints.at(i);

                selectedItems.at(i)->moveBy(dx, 0);

                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}

void UBAlignObjectManager::alignToRight() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> rightPoints(selectedItems.size());

        QGraphicsItem * item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        //find the left point (for compute the delta at the end of the function)
        int mostRight = p.boundingRect().right();

        QGraphicsItem * reference = 0;

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            //find the left point (for compute the delta at the end of the function)
            int right = p.boundingRect().right();

            if(right > mostRight){
                mostRight = right;
                reference = item;
            }

            rightPoints[i] = right;
        }

        UBApplication::undoStack->beginMacro("rightAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dx = mostRight - rightPoints.at(i);

                selectedItems.at(i)->moveBy(dx, 0);

                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}

void UBAlignObjectManager::alignToTop() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> topPoints(selectedItems.size());

        QGraphicsItem *item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        int mostTop = p.boundingRect().top();

        QGraphicsItem *reference = 0;

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            int top = p.boundingRect().top();

            if(top < mostTop){
                mostTop = top;
                reference = item;
            }

            topPoints[i] = top;
        }

        UBApplication::undoStack->beginMacro("topAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dy = mostTop - topPoints.at(i);

                selectedItems.at(i)->moveBy(0, dy);


                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}

void UBAlignObjectManager::alignToBottom() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> bottomPoints(selectedItems.size());

        //The first element is used to initialize the ref bottom
        QGraphicsItem *item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        int mostBottom = p.boundingRect().bottom();

        QGraphicsItem *reference = 0;

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            int bottom = p.boundingRect().bottom();

            if(bottom > mostBottom){
                mostBottom = bottom;
                reference = item;
            }

            bottomPoints[i] = bottom;
        }

        UBApplication::undoStack->beginMacro("bottomAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dy = mostBottom - bottomPoints.at(i);

                selectedItems.at(i)->moveBy(0, dy);

                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}

void UBAlignObjectManager::verticalAlign() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> verticalPoints(selectedItems.size());

        QGraphicsItem *item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        int x_max = p.boundingRect().right();
        int x_min = p.boundingRect().left();

        int mostVertical = x_min + (x_max - x_min) / 2;

        QGraphicsItem *reference = 0;

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            x_max = p.boundingRect().right();
            x_min = p.boundingRect().left();

            int x_vertical = x_min + (x_max - x_min) / 2;

            if(x_vertical < mostVertical){
                mostVertical = x_vertical;
                reference = item;
            }

            verticalPoints[i] = x_vertical;
        }

        UBApplication::undoStack->beginMacro("verticalAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dx = mostVertical - verticalPoints.at(i);

                selectedItems.at(i)->moveBy(dx, 0);

                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}

void UBAlignObjectManager::horizontalAlign() const
{
    QList<QGraphicsItem *> selectedItems = UBApplication::boardController->activeScene()->selectedItems();

    if(selectedItems.size() > 0){
        QVector<int> verticalPoints(selectedItems.size());

        QGraphicsItem *item = selectedItems.at(0);

        QPolygon p = item->transform().mapToPolygon(item->boundingRect().toRect());

        p.translate(item->pos().toPoint());

        int y_max = p.boundingRect().bottom();
        int y_min = p.boundingRect().top();

        int mostVertical = y_min + (y_max - y_min) / 2;

        QGraphicsItem *reference = 0;

        for(int i = 0; i < selectedItems.size(); i++){
            item = selectedItems.at(i);

            p = item->transform().mapToPolygon(item->boundingRect().toRect());

            p.translate(item->pos().toPoint());

            y_max = p.boundingRect().bottom();
            y_min = p.boundingRect().top();

            int y_vertical = y_min + (y_max - y_min) / 2;

            if(y_vertical < mostVertical){
                mostVertical = y_vertical;
                reference = item;
            }

            verticalPoints[i] = y_vertical;
        }

        UBApplication::undoStack->beginMacro("horizontalAlign");

        for(int i = 0; i < selectedItems.size(); i++){
            if(selectedItems.at(i) != reference){
                UBGraphicsItem *item = dynamic_cast<UBGraphicsItem*>(selectedItems.at(i));

                if(item){
                    item->Delegate()->startUndoStep();
                }

                qreal dy = mostVertical - verticalPoints.at(i);

                selectedItems.at(i)->moveBy(0, dy);

                if(item){
                    item->Delegate()->commitUndoStep();
                }
            }
        }

        UBApplication::undoStack->endMacro();
    }
}
