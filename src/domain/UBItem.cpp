#include "UBItem.h"

#include "core/memcheck.h"

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
