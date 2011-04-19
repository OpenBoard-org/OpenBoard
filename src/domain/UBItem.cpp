#include "UBItem.h"

UBItem::UBItem()
    : mUuid(QUuid::createUuid())
    , mRenderingQuality(UBItem::RenderingQualityNormal)
{
    // NOOP
}

UBItem::~UBItem()
{
    // NOOP
}
