
#ifndef UBPAGENUMBERPALLETTE_H_
#define UBPAGENUMBERPALLETTE_H_

class QHBoxLayout;
class QPushButton;

#include "UBFloatingPalette.h"


class UBPageNumberPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBPageNumberPalette(QWidget *parent = 0);
        virtual ~UBPageNumberPalette();

    protected:
        int radius();

    private:
        QHBoxLayout *mLayout;
        QPushButton *mButton;

    private slots:
        void buttonClicked(bool checked);

    public slots:
        void setPageNumber(int current, int total);

    signals:
        void clicked();

};

#endif /* UBPAGENUMBERPALLETTE_H_ */
