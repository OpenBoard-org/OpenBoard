#include "UBEditable.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "domain/UBGraphicsScene.h"

UBAbstractEditable::UBAbstractEditable()
{
    mEditMode = false;
}

UBAbstractEditable::~UBAbstractEditable()
{

}

void UBAbstractEditable::addHandle(UBAbstractHandle *handle)
{
    mHandles.push_back(handle);
}

void UBAbstractEditable::showEditMode(bool show)
{
    if(!show){
        for(int i = 0; i < mHandles.size(); i++){
            mHandles.at(i)->hide();
        }
    }else{
        for(int i = 0; i < mHandles.size(); i++){
            mHandles.at(i)->show();
        }
    }

    mEditMode = show;
}

void UBAbstractEditable::deactivateEditionMode()
{
    //nop
}

bool UBAbstractEditable::isInEditMode() const
{
    return mEditMode;
}
