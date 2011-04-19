//========================================================================
//
// GList.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include "gmem.h"
#include "GList.h"

//------------------------------------------------------------------------
// GList
//------------------------------------------------------------------------

GList::GList() {
  size = 8;
  data = (void **)gmallocn(size, sizeof(void*));
  length = 0;
  inc = 0;
}

GList::GList(int sizeA) {
  size = sizeA ? sizeA : 8;
  data = (void **)gmallocn(size, sizeof(void*));
  length = 0;
  inc = 0;
}

GList::~GList() {
  gfree(data);
}

GList *GList::copy() {
  GList *ret;

  ret = new GList(length);
  ret->length = length;
  memcpy(ret->data, data, length * sizeof(void *));
  ret->inc = inc;
  return ret;
}

void GList::append(void *p) {
  if (length >= size) {
    expand();
  }
  data[length++] = p;
}

void GList::append(GList *list) {
  int i;

  while (length + list->length > size) {
    expand();
  }
  for (i = 0; i < list->length; ++i) {
    data[length++] = list->data[i];
  }
}

void GList::insert(int i, void *p) {
  if (length >= size) {
    expand();
  }
  if (i < 0) {
    i = 0;
  }
  if (i < length) {
    memmove(data+i+1, data+i, (length - i) * sizeof(void *));
  }
  data[i] = p;
  ++length;
}

void *GList::del(int i) {
  void *p;

  p = data[i];
  if (i < length - 1) {
    memmove(data+i, data+i+1, (length - i - 1) * sizeof(void *));
  }
  --length;
  if (size - length >= ((inc > 0) ? inc : size/2)) {
    shrink();
  }
  return p;
}

void GList::sort(int (*cmp)(const void *obj1, const void *obj2)) {
  qsort(data, length, sizeof(void *), cmp);
}

void GList::reverse() {
  void *t;
  int n, i;

  n = length / 2;
  for (i = 0; i < n; ++i) {
    t = data[i];
    data[i] = data[length - 1 - i];
    data[length - 1 - i] = t;
  }
}

void GList::expand() {
  size += (inc > 0) ? inc : size;
  data = (void **)greallocn(data, size, sizeof(void*));
}

void GList::shrink() {
  size -= (inc > 0) ? inc : size/2;
  data = (void **)greallocn(data, size, sizeof(void*));
}
