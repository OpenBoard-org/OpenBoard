
#ifndef UBWEBTOOLSPALLETTE_H_
#define UBWEBTOOLSPALLETTE_H_


#include "UBActionPalette.h"

class UBWebToolsPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBWebToolsPalette(QWidget *parent,bool tutorialMode);
        virtual ~UBWebToolsPalette();

};

#endif /* UBWEBTOOLSPALLETTE_H_ */
