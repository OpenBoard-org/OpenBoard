/*
 * UBColorPicker.h
 *
 *  Created on: Nov 19, 2008
 *      Author: luc
 */

#ifndef UBCOLORPICKER_H_
#define UBCOLORPICKER_H_

#include <QtGui>

class UBColorPicker : public QFrame
{

    Q_OBJECT;

    public:
        UBColorPicker(QWidget* parent);
        UBColorPicker(QWidget* parent, const QList<QColor>& colors, int pSelectedColorIndex = 0);
        virtual ~UBColorPicker();
        QList<QColor> getColors() const
        {
            return mColors;
        }

        void setColors(const QList<QColor>& pColors)
        {
            mColors = pColors;
            repaint();
        }

        int selectedColorIndex() const
        {
            return mSelectedColorIndex;
        }

        void setSelectedColorIndex(int pSelectedColorIndex)
        {
            mSelectedColorIndex = pSelectedColorIndex;
            repaint();
        }

    signals:
        void colorSelected(const QColor& color);

    protected:

        virtual void paintEvent ( QPaintEvent * event );
        virtual void mousePressEvent ( QMouseEvent * event );


    private:
        QList<QColor> mColors;
        int mSelectedColorIndex;
};

#endif /* UBCOLORPICKER_H_ */
