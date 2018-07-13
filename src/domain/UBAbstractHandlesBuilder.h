#ifndef UBABSTRACTHANDLESBUILDER_H
#define UBABSTRACTHANDLESBUILDER_H

#include "UBAbstractHandle.h"

class UBAbstractHandlesBuilder
{
public:
    static void buildHandles(QVector<UBAbstractHandle*>& handles){ Q_UNUSED(handles) }
};

class UB1HandleBuilder : public UBAbstractHandlesBuilder
{
public:
    static void buildHandles(QVector<UBAbstractHandle*>& handles);
};

class UB3HandlesBuilder : public UBAbstractHandlesBuilder
{
public:
    static void buildHandles(QVector<UBAbstractHandle*>& handles);
};

#endif // UBABSTRACTHANDLESBUILDER_H
