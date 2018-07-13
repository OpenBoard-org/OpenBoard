#ifndef UBCREATETABLEPALETTE_H
#define UBCREATETABLEPALETTE_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QMouseEvent>

#include "gui/UBFloatingPalette.h"

class UBCreateTablePalette : public UBFloatingPalette
{
    Q_OBJECT
public:
    explicit UBCreateTablePalette(QWidget *parent = 0);
    ~UBCreateTablePalette();

    int lines() const;
    int columns() const;

    void setPos(QPoint pos);

    signals:
        void validationRequired();

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void keyPressEvent(QKeyEvent *);

private:

    QVBoxLayout* mVLayout;
    QHBoxLayout* mLinesLayout;
    QHBoxLayout* mColumnsLayout;
    QLabel* mLinesLabel;
    QSpinBox* mLinesInput;
    QLabel* mColumnsLabel;
    QSpinBox* mColumnsInput;
    QPushButton* mValidateButton;    
    QPixmap mClosePixmap;

    int mMinimumTableSize;
};

#endif // UBCREATETABLEPALETTE_H
