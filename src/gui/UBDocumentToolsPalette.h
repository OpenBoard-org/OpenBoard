
#ifndef UBDOCUMENTTOOLSPALLETTE_H_
#define UBDOCUMENTTOOLSPALLETTE_H_

#include <QButtonGroup>
#include <QUrl>
#include <QMap>


#include "UBActionPalette.h"


class UBDocumentToolsPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBDocumentToolsPalette(QWidget *parent = 0);
        virtual ~UBDocumentToolsPalette();
        bool isEmpty() { return actions().count() == 0; }

};

#endif /* UBDOCUMENTTOOLSPALLETTE_H_ */
