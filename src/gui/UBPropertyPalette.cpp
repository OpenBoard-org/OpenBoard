#include "UBPropertyPalette.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBPropertyPalette::UBPropertyPalette(QWidget *parent, const char *name):UBActionPalette(parent)
{
    setObjectName(name);
    mbGrip = false;
}

/**
 * \brief Constructor
 * @param orientation as the palette orientation
 * @param parent as the parent widget
 */
UBPropertyPalette::UBPropertyPalette(Qt::Orientation orientation, QWidget *parent):UBActionPalette(orientation, parent)
{
    mbGrip = false;
}

/**
 * \brief Destructor
 */
UBPropertyPalette::~UBPropertyPalette()
{

}

/**
 * \brief Handles the mouse release event
 */
void UBPropertyPalette::onMouseRelease()
{
//    qDebug() << "UBPropertyPalette::onMouseRelease() called (" << mMousePos.x() << "," << mMousePos.y();
//    QWidget* pW = NULL;
//    pW = childAt(mMousePos);

//    if(NULL != pW)
//    {
//        // A widget has been found under the mouse!
//        UBActionPaletteButton* pButton = dynamic_cast<UBActionPaletteButton*>(pW);
//        if(NULL != pButton)
//        {
//            pButton->click();
//        }
//    }

//    // Close the palette
//    close();
}
