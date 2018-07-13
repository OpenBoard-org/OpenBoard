#include "UBAbstractHandlesBuilder.h"

#include "UBDiagonalHandle.h"
#include "UBVerticalHandle.h"
#include "UBHorizontalHandle.h"

void UB1HandleBuilder::buildHandles(QVector<UBAbstractHandle *> &handles)
{
    //before clean the vector
    if(handles.size() > 0){
        for(int i = 0; i < handles.size(); i++){
            delete handles.at(i);
        }

        handles.clear();
    }

    UBDiagonalHandle *dh = new UBDiagonalHandle();

    dh->hide();

    handles.push_back(dh);
}

void UB3HandlesBuilder::buildHandles(QVector<UBAbstractHandle *> &handles)
{
    //before clean the vector
    if(handles.size() > 0){
        for(int i = 0; i < handles.size(); i++){
            delete handles.at(i);
        }

        handles.clear();
    }

    UBHorizontalHandle *hh = new UBHorizontalHandle();
    UBVerticalHandle *vh = new UBVerticalHandle();
    UBDiagonalHandle *dh = new UBDiagonalHandle();

    vh->setId(1);
    dh->setId(2);

    hh->hide();
    vh->hide();
    dh->hide();

    handles.push_back(hh);
    handles.push_back(vh);
    handles.push_back(dh);
}
