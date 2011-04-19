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
#include <limits.h>
#include "gmem.h"
#include "gfile.h"
#include "GList.h"
#include "Object.h"
#include "CharTypes.h"
#include "PDFDoc.h"
#include "XRef.h"
#include "Array.h"
#include "Dict.h"
#include "Page.h"
#include "Error.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Catalog.h"

//------------------------------------------------------------------------
// PageTreeNode
//------------------------------------------------------------------------

class PageTreeNode {
public:

  PageTreeNode(Ref refA, int countA, PageTreeNode *parentA);
  ~PageTreeNode();

  Ref ref;
  int count;
  PageTreeNode *parent;
  GList *kids;			// [PageTreeNode]
  PageAttrs *attrs;
};

PageTreeNode::PageTreeNode(Ref refA, int countA, PageTreeNode *parentA) {
  ref = refA;
  count = countA;
  parent = parentA;
  kids = NULL;
  attrs = NULL;
}

PageTreeNode::~PageTreeNode() {
  delete attrs;
  if (kids) {
    deleteGList(kids, PageTreeNode);
  }
}

//------------------------------------------------------------------------
// EmbeddedFile
//------------------------------------------------------------------------

class EmbeddedFile {
public:

  EmbeddedFile(Unicode *nameA, int nameLenA, Object *streamRefA);
  ~EmbeddedFile();

  Unicode *name;
  int nameLen;
  Object streamRef;
};

EmbeddedFile::EmbeddedFile(Unicode *nameA, int nameLenA,
			   Object *streamRefA) {
  name = nameA;
  nameLen = nameLenA;
  streamRefA->copy(&streamRef);
}

EmbeddedFile::~EmbeddedFile() {
  gfree(name);
  streamRef.free();
}

//------------------------------------------------------------------------
// Catalog
//------------------------------------------------------------------------

Catalog::Catalog(PDFDoc *docA) {
  Object catDict;
  Object obj, obj2;

  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  pageTree = NULL;
  pages = NULL;
  pageRefs = NULL;
  numPages = 0;
  baseURI = NULL;
  embeddedFiles = NULL;

  xref->getCatalog(&catDict);
  if (!catDict.isDict()) {
    error(errSyntaxError, -1, "Catalog object is wrong type ({0:s})",
	  catDict.getTypeName());
    goto err1;
  }

  // read page tree
  if (!readPageTree(&catDict)) {
    goto err1;
  }

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
  if (!baseURI || baseURI->getLength() == 0) {
    if (baseURI) {
      delete baseURI;
    }
    if (doc->getFileName()) {
      baseURI = makePathAbsolute(grabPath(doc->getFileName()->getCString()));
      if (baseURI->getChar(0) == '/') {
	baseURI->insert(0, "file://localhost");
      } else {
	baseURI->insert(0, "file://localhost/");
      }
    } else {
      baseURI = new GString("file://localhost/");
    }
  }

  // get the metadata stream
  catDict.dictLookup("Metadata", &metadata);

  // get the structure tree root
  catDict.dictLookup("StructTreeRoot", &structTreeRoot);

  // get the outline dictionary
  catDict.dictLookup("Outlines", &outline);

  // get the AcroForm dictionary
  catDict.dictLookup("AcroForm", &acroForm);

  // get the OCProperties dictionary
  catDict.dictLookup("OCProperties", &ocProperties);

  // get the list of embedded files
  readEmbeddedFileList(catDict.getDict());

  catDict.free();
  return;

 err1:
  catDict.free();
  dests.initNull();
  nameTree.initNull();
  ok = gFalse;
}

Catalog::~Catalog() {
  int i;

  if (pageTree) {
    delete pageTree;
  }
  if (pages) {
    for (i = 0; i < numPages; ++i) {
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
  ocProperties.free();
  if (embeddedFiles) {
    deleteGList(embeddedFiles, EmbeddedFile);
  }
}

Page *Catalog::getPage(int i) {
  if (!pages[i-1]) {
    loadPage(i);
  }
  return pages[i-1];
}

Ref *Catalog::getPageRef(int i) {
  if (!pages[i-1]) {
    loadPage(i);
  }
  return &pageRefs[i-1];
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
    error(errSyntaxWarning, -1, "Unknown Metadata type: '{0:s}'",
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

int Catalog::findPage(int num, int gen) {
  int i;

  for (i = 0; i < numPages; ++i) {
    if (!pages[i]) {
      loadPage(i+1);
    }
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
      error(errSyntaxWarning, -1, "Bad named destination value");
    obj2.free();
  } else {
    error(errSyntaxWarning, -1, "Bad named destination value");
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

GBool Catalog::readPageTree(Object *catDict) {
  Object topPagesRef, topPagesObj, countObj;
  int i;

  if (!catDict->dictLookupNF("Pages", &topPagesRef)->isRef()) {
    error(errSyntaxError, -1, "Top-level pages reference is wrong type ({0:s})",
	  topPagesRef.getTypeName());
    topPagesRef.free();
    return gFalse;
  }
  if (!topPagesRef.fetch(xref, &topPagesObj)->isDict()) {
    error(errSyntaxError, -1, "Top-level pages object is wrong type ({0:s})",
	  topPagesObj.getTypeName());
    topPagesObj.free();
    topPagesRef.free();
    return gFalse;
  }
  if (topPagesObj.dictLookup("Count", &countObj)->isInt()) {
    numPages = countObj.getInt();
    if (numPages == 0) {
      // Acrobat apparently scans the page tree if it sees a zero count
      numPages = countPageTree(&topPagesObj);
    }
  } else {
    // assume we got a Page node instead of a Pages node
    numPages = 1;
  }
  countObj.free();
  if (numPages < 0) {
    error(errSyntaxError, -1, "Invalid page count");
    topPagesObj.free();
    topPagesRef.free();
    numPages = 0;
    return gFalse;
  }
  pageTree = new PageTreeNode(topPagesRef.getRef(), numPages, NULL);
  topPagesObj.free();
  topPagesRef.free();
  pages = (Page **)greallocn(pages, numPages, sizeof(Page *));
  pageRefs = (Ref *)greallocn(pageRefs, numPages, sizeof(Ref));
  for (i = 0; i < numPages; ++i) {
    pages[i] = NULL;
    pageRefs[i].num = -1;
    pageRefs[i].gen = -1;
  }
  return gTrue;
}

int Catalog::countPageTree(Object *pagesObj) {
  Object kids, kid;
  int n, n2, i;

  if (!pagesObj->isDict()) {
    return 0;
  }
  if (pagesObj->dictLookup("Kids", &kids)->isArray()) {
    n = 0;
    for (i = 0; i < kids.arrayGetLength(); ++i) {
      kids.arrayGet(i, &kid);
      n2 = countPageTree(&kid);
      if (n2 < INT_MAX - n) {
	n += n2;
      } else {
	error(errSyntaxError, -1, "Page tree contains too many pages");
	n = INT_MAX;
      }
      kid.free();
    }
  } else {
    n = 1;
  }
  kids.free();
  return n;
}

void Catalog::loadPage(int pg) {
  loadPage2(pg, pg - 1, pageTree);
}

void Catalog::loadPage2(int pg, int relPg, PageTreeNode *node) {
  Object pageRefObj, pageObj, kidsObj, kidRefObj, kidObj, countObj;
  PageTreeNode *kidNode, *p;
  PageAttrs *attrs;
  int count, i;

  if (relPg >= node->count) {
    error(errSyntaxError, -1, "Internal error in page tree");
    pages[pg-1] = new Page(doc, pg);
    return;
  }

  // if this node has not been filled in yet, it's either a leaf node
  // or an unread internal node
  if (!node->kids) {

    // check for a loop in the page tree
    for (p = node->parent; p; p = p->parent) {
      if (node->ref.num == p->ref.num && node->ref.gen == p->ref.gen) {
	error(errSyntaxError, -1, "Loop in Pages tree");
	pages[pg-1] = new Page(doc, pg);
	return;
      }
    }

    // fetch the Page/Pages object
    pageRefObj.initRef(node->ref.num, node->ref.gen);
    if (!pageRefObj.fetch(xref, &pageObj)->isDict()) {
      error(errSyntaxError, -1, "Page tree object is wrong type ({0:s})",
	    pageObj.getTypeName());
      pageObj.free();
      pageRefObj.free();
      pages[pg-1] = new Page(doc, pg);
      return;
    }

    // merge the PageAttrs
    attrs = new PageAttrs(node->parent ? node->parent->attrs
			               : (PageAttrs *)NULL,
			  pageObj.getDict());

    // if "Kids" exists, it's an internal node
    if (pageObj.dictLookup("Kids", &kidsObj)->isArray()) {

      // save the PageAttrs
      node->attrs = attrs;

      // read the kids
      node->kids = new GList();
      for (i = 0; i < kidsObj.arrayGetLength(); ++i) {
	if (kidsObj.arrayGetNF(i, &kidRefObj)->isRef()) {
	  if (kidRefObj.fetch(xref, &kidObj)->isDict()) {
	    if (kidObj.dictLookup("Count", &countObj)->isInt()) {
	      count = countObj.getInt();
	    } else {
	      count = 1;
	    }
	    countObj.free();
	    node->kids->append(new PageTreeNode(kidRefObj.getRef(), count,
						node));
	  } else {
	    error(errSyntaxError, -1, "Page tree object is wrong type ({0:s})",
		  kidObj.getTypeName());
	  }
	  kidObj.free();
	} else {
	  error(errSyntaxError, -1,
		"Page tree reference is wrong type ({0:s})",
		kidRefObj.getTypeName());
	}
	kidRefObj.free();
      }

    } else {
      
      // create the Page object
      pageRefs[pg-1] = node->ref;
      pages[pg-1] = new Page(doc, pg, pageObj.getDict(), attrs);
      if (!pages[pg-1]->isOk()) {
	delete pages[pg-1];
	pages[pg-1] = new Page(doc, pg);
      }

    }

    kidsObj.free();
    pageObj.free();
    pageRefObj.free();
  }

  // recursively descend the tree
  if (node->kids) {
    for (i = 0; i < node->kids->getLength(); ++i) {
      kidNode = (PageTreeNode *)node->kids->get(i);
      if (relPg < kidNode->count) {
	loadPage2(pg, relPg, kidNode);
	break;
      }
      relPg -= kidNode->count;
    }

    // this will only happen if the page tree is invalid
    // (i.e., parent count > sum of children counts)
    if (i == node->kids->getLength()) {
      error(errSyntaxError, -1, "Invalid page count in page tree");
      pages[pg-1] = new Page(doc, pg);
    }
  }
}

void Catalog::readEmbeddedFileList(Dict *catDict) {
  Object obj1, obj2;

  if (catDict->lookup("Names", &obj1)->isDict()) {
    if (obj1.dictLookup("EmbeddedFiles", &obj2)->isDict()) {
      embeddedFiles = new GList();
      readEmbeddedFileTree(&obj2);
    }
    obj2.free();
  }
  obj1.free();
}

void Catalog::readEmbeddedFileTree(Object *node) {
  Object obj1, obj2, obj3, obj4, obj5;
  GString *s;
  Unicode *name;
  int nameLen, i, j;

  if (node->dictLookup("Kids", &obj1)->isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGet(i, &obj2)->isDict()) {
	readEmbeddedFileTree(&obj2);
      }
      obj2.free();
    }
  } else {
    obj1.free();
    if (node->dictLookup("Names", &obj1)->isArray()) {
      for (i = 0; i+1 < obj1.arrayGetLength(); ++i) {
	obj1.arrayGet(i, &obj2);
	obj1.arrayGet(i+1, &obj3);
	if (obj2.isString() && obj3.isDict()) {
	  if (obj3.dictLookup("EF", &obj4)->isDict()) {
	    if (obj4.dictLookupNF("F", &obj5)->isRef()) {
	      s = obj2.getString();
	      if ((s->getChar(0) & 0xff) == 0xfe &&
		  (s->getChar(1) & 0xff) == 0xff) {
		nameLen = (s->getLength() - 2) / 2;
		name = (Unicode *)gmallocn(nameLen, sizeof(Unicode));
		for (j = 0; j < nameLen; ++j) {
		  name[j] = ((s->getChar(2 + 2*j) & 0xff) << 8) |
		    (s->getChar(3 + 2*j) & 0xff);
		}
	      } else {
		nameLen = s->getLength();
		name = (Unicode *)gmallocn(nameLen, sizeof(Unicode));
		for (j = 0; j < nameLen; ++j) {
		  name[j] = pdfDocEncoding[s->getChar(j) & 0xff];
		}
	      }
	      embeddedFiles->append(new EmbeddedFile(name, nameLen, &obj5));
	    }
	    obj5.free();
	  }
	  obj4.free();
	}
	obj2.free();
	obj3.free();
      }
    }
  }
  obj1.free();
}

int Catalog::getNumEmbeddedFiles() {
  return embeddedFiles ? embeddedFiles->getLength() : 0;
}

Unicode *Catalog::getEmbeddedFileName(int idx) {
  return ((EmbeddedFile *)embeddedFiles->get(idx))->name;
}

int Catalog::getEmbeddedFileNameLength(int idx) {
  return ((EmbeddedFile *)embeddedFiles->get(idx))->nameLen;
}

Object *Catalog::getEmbeddedFileStreamObj(int idx, Object *strObj) {
  ((EmbeddedFile *)embeddedFiles->get(idx))->streamRef.fetch(xref, strObj);
  if (!strObj->isStream()) {
    strObj->free();
    return NULL;
  }
  return strObj;
}
