/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
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


#ifndef UBGRAPHICSITEMCOLORUNDOCOMMAND_H
#define UBGRAPHICSITEMCOLORUNDOCOMMAND_H

#include "UBUndoCommand.h"
#include "UBGraphicsScene.h"
#include "domain/UBGraphicsStrokesGroup.h"
#include "domain/UBGraphicsTextItem.h"

#include <memory>
#include <vector>

class UBGraphicsItemColorUndoCommand : public UBUndoCommand
{
public:

    /**
     * flyweight structure to keep colors for dark and light backgrounds
     */
    struct ColorPair
    {
        const QColor onDark;
        const QColor onLight;

        ColorPair(const ColorPair& other)
        : onDark(other.onDark), onLight(other.onLight)
        {
        }

        ColorPair(const QColor& pOnDark, const QColor& pOnLight)
        : onDark(pOnDark), onLight(pOnLight)
        {

        }
#ifndef QT_NO_DEBUG_STREAM

        friend Q_GUI_EXPORT QDebug operator<<(QDebug ds, const ColorPair& cp)
        {
            ds << "(" << cp.onDark << ", " << cp.onLight << ")";
            return ds;
        }
#endif

    };

    /**
     * base-class to encapsulate the common data for undo-able changes of color
     */
    class ColorCommand
    {
    public:
        const ColorPair mNextCol;
        const ColorPair mPrevCol;

        ColorCommand() = delete;
        ColorCommand(const ColorCommand&) = delete;

        ColorCommand(const ColorPair& pNextCol, const ColorPair& pPrevCol)
        : mNextCol(pNextCol), mPrevCol(pPrevCol)
        {

        }

        virtual ~ColorCommand()
        {

        }

        virtual void setColor() = 0;
        virtual void setPreviousColor() = 0;
    };

    /**
     * change of color for a group of strokes
     */
    class StrokeColorCommand : public ColorCommand
    {
        UBGraphicsStrokesGroup* mStroke;
        const UBGraphicsScene& mScene;

        void setColor(const ColorPair& pColors)
        {
            mStroke->setColor(pColors.onDark, UBGraphicsStrokesGroup::colorType::colorOnDarkBackground);
            mStroke->setColor(pColors.onLight, UBGraphicsStrokesGroup::colorType::colorOnLightBackground);
            if (mScene.isDarkBackground())
            {
                mStroke->setColor(pColors.onDark, UBGraphicsStrokesGroup::colorType::currentColor);
            }
            else
            {
                mStroke->setColor(pColors.onLight, UBGraphicsStrokesGroup::colorType::currentColor);
            }
        }


    public:
        StrokeColorCommand() = delete;
        StrokeColorCommand(const StrokeColorCommand&) = delete;

        StrokeColorCommand(const ColorPair& ct, UBGraphicsStrokesGroup& pStroke, const UBGraphicsScene& pScene)
        : ColorCommand(ct, ColorPair(pStroke.color(UBGraphicsStrokesGroup::colorOnDarkBackground),
                                     pStroke.color(UBGraphicsStrokesGroup::colorOnLightBackground))),
        mStroke(&pStroke), mScene(pScene)
        {
        }

        virtual ~StrokeColorCommand()
        {

        }

        void setColor()
        {
            qDebug() << "nextCol: " << mNextCol << endl;
            setColor(mNextCol);
        }

        void setPreviousColor()
        {
            qDebug() << "prevCol: " << mPrevCol << endl;
            setColor(mPrevCol);
        };

    };

    /**
     * change of color for a text-item
     */
    class TextColorCommand : public ColorCommand
    {
        UBGraphicsTextItem* mText;
    public:
        TextColorCommand() = delete;
        TextColorCommand(const TextColorCommand&) = delete;

        TextColorCommand(const ColorPair ct, UBGraphicsTextItem& pText)
        : ColorCommand(ct,
                       ColorPair(pText.colorOnDarkBackground(),
                                 pText.colorOnLightBackground())),
        mText(&pText)
        {
        }

        virtual ~TextColorCommand()
        {

        }

        void setColor()
        {
            qDebug() << "nextCol: " << mNextCol << endl;

            mText->setColorOnDarkBackground(mNextCol.onDark);
            mText->setColorOnLightBackground(mNextCol.onLight);
        }

        void setPreviousColor()
        {
            qDebug() << "prevCol: " << mPrevCol << endl;

            mText->setColorOnDarkBackground(mPrevCol.onDark);
            mText->setColorOnLightBackground(mPrevCol.onLight);
        };

    };
    typedef std::vector< std::shared_ptr<ColorCommand> > ColorCommandsVector;


    UBGraphicsItemColorUndoCommand() = delete;
    UBGraphicsItemColorUndoCommand(const UBGraphicsItemColorUndoCommand&) = delete;
    UBGraphicsItemColorUndoCommand(UBGraphicsScene* _scene, const ColorCommandsVector& pCmds);

    ~UBGraphicsItemColorUndoCommand();

    virtual int getType() const
    {
        return UBUndoType::undotype_GRAPHICITEMCOLOR;
    }

protected:
    virtual void undo();
    virtual void redo();

private:
    void init(UBGraphicsScene* _scene);
    void updateLazyScene();

    UBGraphicsScene* mpScene;
    bool mHack;

    const ColorCommandsVector mColorCommands;
};


#endif // UBGRAPHICSITEMCOLORUNDOCOMMAND_H
