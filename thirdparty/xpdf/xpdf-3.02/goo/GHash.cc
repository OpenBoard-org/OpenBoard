//========================================================================
//
// GHash.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "gmem.h"
#include "GString.h"
#include "GHash.h"

//------------------------------------------------------------------------

struct GHashBucket {
  GString *key;
  union {
    void *p;
    int i;
  } val;
  GHashBucket *next;
};

struct GHashIter {
  int h;
  GHashBucket *p;
};

//------------------------------------------------------------------------

GHash::GHash(GBool deleteKeysA) {
  int h;

  deleteKeys = deleteKeysA;
  size = 7;
  tab = (GHashBucket **)gmallocn(size, sizeof(GHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  len = 0;
}

GHash::~GHash() {
  GHashBucket *p;
  int h;

  for (h = 0; h < size; ++h) {
    while (tab[h]) {
      p = tab[h];
      tab[h] = p->next;
      if (deleteKeys) {
	delete p->key;
      }
      delete p;
    }
  }
  gfree(tab);
}

void GHash::add(GString *key, void *val) {
  GHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GHashBucket;
  p->key = key;
  p->val.p = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GHash::add(GString *key, int val) {
  GHashBucket *p;
  int h;

  // expand the table if necessary
  if (len >= size) {
    expand();
  }

  // add the new symbol
  p = new GHashBucket;
  p->key = key;
  p->val.i = val;
  h = hash(key);
  p->next = tab[h];
  tab[h] = p;
  ++len;
}

void GHash::replace(GString *key, void *val) {
  GHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.p = val;
    delete key;
  } else {
    add(key, val);
  }
}

void GHash::replace(GString *key, int val) {
  GHashBucket *p;
  int h;

  if ((p = find(key, &h))) {
    p->val.i = val;
    delete key;
  } else {
    add(key, val);
  }
}

void *GHash::lookup(GString *key) {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GHash::lookupInt(GString *key) {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GHash::lookup(char *key) {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  return p->val.p;
}

int GHash::lookupInt(char *key) {
  GHashBucket *p;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  return p->val.i;
}

void *GHash::remove(GString *key) {
  GHashBucket *p;
  GHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GHash::removeInt(GString *key) {
  GHashBucket *p;
  GHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void *GHash::remove(char *key) {
  GHashBucket *p;
  GHashBucket **q;
  void *val;
  int h;

  if (!(p = find(key, &h))) {
    return NULL;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.p;
  delete p;
  --len;
  return val;
}

int GHash::removeInt(char *key) {
  GHashBucket *p;
  GHashBucket **q;
  int val;
  int h;

  if (!(p = find(key, &h))) {
    return 0;
  }
  q = &tab[h];
  while (*q != p) {
    q = &((*q)->next);
  }
  *q = p->next;
  if (deleteKeys) {
    delete p->key;
  }
  val = p->val.i;
  delete p;
  --len;
  return val;
}

void GHash::startIter(GHashIter **iter) {
  *iter = new GHashIter;
  (*iter)->h = -1;
  (*iter)->p = NULL;
}

GBool GHash::getNext(GHashIter **iter, GString **key, void **val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.p;
  return gTrue;
}

GBool GHash::getNext(GHashIter **iter, GString **key, int *val) {
  if (!*iter) {
    return gFalse;
  }
  if ((*iter)->p) {
    (*iter)->p = (*iter)->p->next;
  }
  while (!(*iter)->p) {
    if (++(*iter)->h == size) {
      delete *iter;
      *iter = NULL;
      return gFalse;
    }
    (*iter)->p = tab[(*iter)->h];
  }
  *key = (*iter)->p->key;
  *val = (*iter)->p->val.i;
  return gTrue;
}

void GHash::killIter(GHashIter **iter) {
  delete *iter;
  *iter = NULL;
}

void GHash::expand() {
  GHashBucket **oldTab;
  GHashBucket *p;
  int oldSize, h, i;

  oldSize = size;
  oldTab = tab;
  size = 2*size + 1;
  tab = (GHashBucket **)gmallocn(size, sizeof(GHashBucket *));
  for (h = 0; h < size; ++h) {
    tab[h] = NULL;
  }
  for (i = 0; i < oldSize; ++i) {
    while (oldTab[i]) {
      p = oldTab[i];
      oldTab[i] = oldTab[i]->next;
      h = hash(p->key);
      p->next = tab[h];
      tab[h] = p;
    }
  }
  gfree(oldTab);
}

GHashBucket *GHash::find(GString *key, int *h) {
  GHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

GHashBucket *GHash::find(char *key, int *h) {
  GHashBucket *p;

  *h = hash(key);
  for (p = tab[*h]; p; p = p->next) {
    if (!p->key->cmp(key)) {
      return p;
    }
  }
  return NULL;
}

int GHash::hash(GString *key) {
  char *p;
  unsigned int h;
  int i;

  h = 0;
  for (p = key->getCString(), i = 0; i < key->getLength(); ++p, ++i) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}

int GHash::hash(char *key) {
  char *p;
  unsigned int h;

  h = 0;
  for (p = key; *p; ++p) {
    h = 17 * h + (int)(*p & 0xff);
  }
  return (int)(h % size);
}
