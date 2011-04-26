//========================================================================
//
// Catalog.cc
//
// Copyright 1996-2007 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stddef.h>
#include "gmem.h"
#include "Object.h"
#include "XRef.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Link.h"
#include "Catalog.h"

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(XRef *xrefA) {
  Object catDict, pagesDict, pagesDictRef;
  Object obj, obj2;
  char *alreadyRead;
  int numPages0;
  int i;

  ok = gTrue;
  xref = xrefA;
  pages = NULL;
  pageRefs = NULL;
  numPages = pagesSize = 0;
  baseURI = NULL;

  xref->getCatalog(&catDict);
  if (!catDict.isDict()) {
    error(-1, "Catalog object is wrong type (%s)", catDict.getTypeName());
    goto err1;
  }

  // read page tree
  catDict.dictLookup("Pages", &pagesDict);
  // This should really be isDict("Pages"), but I've seen at least one
  // PDF file where the /Type entry is missing.
  if (!pagesDict.isDict()) {
    error(-1, "Top-level pages object is wrong type (%s)",
	  pagesDict.getTypeName());
    goto err2;
  }
  pagesDict.dictLookup("Count", &obj);
  // some PDF files actually use real numbers here ("/Count 9.0")
  if (!obj.isNum()) {
    error(-1, "Page count in top-level pages object is wrong type (%s)",
	  obj.getTypeName());
    goto err3;
  }
  pagesSize = numPages0 = (int)obj.getNum();
  obj.free();
  pages = (Page **)gmallocn(pagesSize, sizeof(Page *));
  pageRefs = (Ref *)gmallocn(pagesSize, sizeof(Ref));
  for (i = 0; i < pagesSize; ++i) {
    pages[i] = NULL;
    pageRefs[i].num = -1;
    pageRefs[i].gen = -1;
  }
  alreadyRead = (char *)gmalloc(xref->getNumObjects());
  memset(alreadyRead, 0, xref->getNumObjects());
  if (catDict.dictLookupNF("Pages", &pagesDictRef)->isRef() &&
      pagesDictRef.getRefNum() >= 0 &&
      pagesDictRef.getRefNum() < xref->getNumObjects()) {
    alreadyRead[pagesDictRef.getRefNum()] = 1;
  }
  pagesDictRef.free();
  numPages = readPageTree(pagesDict.getDict(), NULL, 0, alreadyRead);
  gfree(alreadyRead);
  if (numPages != numPages0) {
    error(-1, "Page count in top-level pages object is incorrect");
  }
  pagesDict.free();

  // read named destination dictionary
  catDict.dictLookup("Dests", &dests);

  // read root of named destination tree
  if (catDict.dictLookup("Names", &obj)->isDict())
    obj.dictLookup("Dests", &nameTree);
  else
    nameTree.initNull();
  obj.free();

  // read base URI
  if (catDict.dictLookup("URI", &obj)->isDict()) {
    if (obj.dictLookup("Base", &obj2)->isString()) {
      baseURI = obj2.getString()->copy();
    }
    obj2.free();
  }
  obj.free();

  // get the metadata stream
  catDict.dictLookup("Metadata", &metadata);

  // get the structure tree root
  catDict.dictLookup("StructTreeRoot", &structTreeRoot);

  // get the outline dictionary
  catDict.dictLookup("Outlines", &outline);

  // get the AcroForm dictionary
  catDict.dictLookup("AcroForm", &acroForm);

  catDict.free();
  return;

 err3:
  obj.free();
 err2:
  pagesDict.free();
 err1:
  catDict.free();
  dests.initNull();
  nameTree.initNull();
  ok = gFalse;
}

Catalog::~Catalog() {
  int i;

  if (pages) {
    for (i = 0; i < pagesSize; ++i) {
      if (pages[i]) {
	delete pages[i];
      }
    }
    gfree(pages);
    gfree(pageRefs);
  }
  dests.free();
  nameTree.free();
  if (baseURI) {
    delete baseURI;
  }
  metadata.free();
  structTreeRoot.free();
  outline.free();
  acroForm.free();
}

GString *Catalog::readMetadata() {
  GString *s;
  Dict *dict;
  Object obj;
  int c;

  if (!metadata.isStream()) {
    return NULL;
  }
  dict = metadata.streamGetDict();
  if (!dict->lookup("Subtype", &obj)->isName("XML")) {
    error(-1, "Unknown Metadata type: '%s'",
	  obj.isName() ? obj.getName() : "???");
  }
  obj.free();
  s = new GString();
  metadata.streamReset();
  while ((c = metadata.streamGetChar()) != EOF) {
    s->append(c);
  }
  metadata.streamClose();
  return s;
}

int Catalog::readPageTree(Dict *pagesDict, PageAttrs *attrs, int start,
			  char *alreadyRead) {
  Object kids;
  Object kid;
  Object kidRef;
  PageAttrs *attrs1, *attrs2;
  Page *page;
  int i, j;

  attrs1 = new PageAttrs(attrs, pagesDict);
  pagesDict->lookup("Kids", &kids);
  if (!kids.isArray()) {
    error(-1, "Kids object (page %d) is wrong type (%s)",
	  start+1, kids.getTypeName());
    goto err1;
  }
  for (i = 0; i < kids.arrayGetLength(); ++i) {
    kids.arrayGetNF(i, &kidRef);
    if (kidRef.isRef() &&
	kidRef.getRefNum() >= 0 &&
	kidRef.getRefNum() < xref->getNumObjects()) {
      if (alreadyRead[kidRef.getRefNum()]) {
	error(-1, "Loop in Pages tree");
	kidRef.free();
	continue;
      }
      alreadyRead[kidRef.getRefNum()] = 1;
    }
    kids.arrayGet(i, &kid);
    if (kid.isDict("Page")) {
      attrs2 = new PageAttrs(attrs1, kid.getDict());
      page = new Page(xref, start+1, kid.getDict(), attrs2);
      if (!page->isOk()) {
	++start;
	goto err3;
      }
      if (start >= pagesSize) {
	pagesSize += 32;
	pages = (Page **)greallocn(pages, pagesSize, sizeof(Page *));
	pageRefs = (Ref *)greallocn(pageRefs, pagesSize, sizeof(Ref));
	for (j = pagesSize - 32; j < pagesSize; ++j) {
	  pages[j] = NULL;
	  pageRefs[j].num = -1;
	  pageRefs[j].gen = -1;
	}
      }
      pages[start] = page;
      if (kidRef.isRef()) {
	pageRefs[start].num = kidRef.getRefNum();
	pageRefs[start].gen = kidRef.getRefGen();
      }
      ++start;
    // This should really be isDict("Pages"), but I've seen at least one
    // PDF file where the /Type entry is missing.
    } else if (kid.isDict()) {
      if ((start = readPageTree(kid.getDict(), attrs1, start, alreadyRead))
	  < 0)
	goto err2;
    } else {
      error(-1, "Kid object (page %d) is wrong type (%s)",
	    start+1, kid.getTypeName());
    }
    kid.free();
    kidRef.free();
  }
  delete attrs1;
  kids.free();
  return start;

 err3:
  delete page;
 err2:
  kid.free();
 err1:
  kids.free();
  delete attrs1;
  ok = gFalse;
  return -1;
}

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < numPages; ++i) {
    if (pageRefs[i].num == num && pageRefs[i].gen == gen)
      return i + 1;
  }
  return 0;
}

LinkDest *Catalog::findDest(GString *name) {
  LinkDest *dest;
  Object obj1, obj2;
  GBool found;

  // try named destination dictionary then name tree
  found = gFalse;
  if (dests.isDict()) {
    if (!dests.dictLookup(name->getCString(), &obj1)->isNull())
      found = gTrue;
    else
      obj1.free();
  }
  if (!found && nameTree.isDict()) {
    if (!findDestInTree(&nameTree, name, &obj1)->isNull())
      found = gTrue;
    else
      obj1.free();
  }
  if (!found)
    return NULL;

  // construct LinkDest
  dest = NULL;
  if (obj1.isArray()) {
    dest = new LinkDest(obj1.getArray());
  } else if (obj1.isDict()) {
    if (obj1.dictLookup("D", &obj2)->isArray())
      dest = new LinkDest(obj2.getArray());
    else
      error(-1, "Bad named destination value");
    obj2.free();
  } else {
    error(-1, "Bad named destination value");
  }
  obj1.free();
  if (dest && !dest->isOk()) {
    delete dest;
    dest = NULL;
  }

  return dest;
}

Object *Catalog::findDestInTree(Object *tree, GString *name, Object *obj) {
  Object names, name1;
  Object kids, kid, limits, low, high;
  GBool done, found;
  int cmp, i;

  // leaf node
  if (tree->dictLookup("Names", &names)->isArray()) {
    done = found = gFalse;
    for (i = 0; !done && i < names.arrayGetLength(); i += 2) {
      if (names.arrayGet(i, &name1)->isString()) {
	cmp = name->cmp(name1.getString());
	if (cmp == 0) {
	  names.arrayGet(i+1, obj);
	  found = gTrue;
	  done = gTrue;
	} else if (cmp < 0) {
	  done = gTrue;
	}
      }
      name1.free();
    }
    names.free();
    if (!found)
      obj->initNull();
    return obj;
  }
  names.free();

  // root or intermediate node
  done = gFalse;
  if (tree->dictLookup("Kids", &kids)->isArray()) {
    for (i = 0; !done && i < kids.arrayGetLength(); ++i) {
      if (kids.arrayGet(i, &kid)->isDict()) {
	if (kid.dictLookup("Limits", &limits)->isArray()) {
	  if (limits.arrayGet(0, &low)->isString() &&
	      name->cmp(low.getString()) >= 0) {
	    if (limits.arrayGet(1, &high)->isString() &&
		name->cmp(high.getString()) <= 0) {
	      findDestInTree(&kid, name, obj);
	      done = gTrue;
	    }
	    high.free();
	  }
	  low.free();
	}
	limits.free();
      }
      kid.free();
    }
  }
  kids.free();

  // name was outside of ranges of all kids
  if (!done)
    obj->initNull();

  return obj;
}
