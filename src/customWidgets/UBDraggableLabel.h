#ifndef UBDRAGGABLELABEL_H
#define UBDRAGGABLELABEL_H

#include <QLabel>

class UBDraggableLabel : public QLabel
{
    Q_OBJECT
public:
    UBDraggableLabel(QWidget *parent = 0);
    ~UBDraggableLabel();
    void loadImage(QString imagePath);
signals:

public slots:

protected:
    QString mSourcePath;
    void mousePressEvent(QMouseEvent *event);

};

#endif // UBDRAGGABLELABEL_H
