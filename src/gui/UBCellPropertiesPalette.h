#ifndef UBCELLPROPERTIESPALETTE_H
#define UBCELLPROPERTIESPALETTE_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMouseEvent>

#include "gui/UBFloatingPalette.h"

class UBCellPropertiesPalette : public UBFloatingPalette
{
    Q_OBJECT
public:
    explicit UBCellPropertiesPalette(QWidget *parent = 0);
    ~UBCellPropertiesPalette();

    int width() const;
    void setWidth(int value);

    void setPos(QPoint pos);

    signals:
        void validationRequired();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void keyPressEvent(QKeyEvent *);

private:

    QVBoxLayout* mVLayout;
    QHBoxLayout* mWidthLayout;
    QLabel* mWidthLabel;
    QSpinBox* mWidthInput;
    QPushButton* mValidateButton;
    QPixmap mClosePixmap;

    int mMinimumTableSize;
};


#endif // UBCELLPROPERTIESPALETTE_H
