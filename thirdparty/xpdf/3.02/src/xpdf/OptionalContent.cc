//========================================================================
//
// OptionalContent.cc
//
// Copyright 2008 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "GString.h"
#include "GList.h"
#include "Error.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocEncoding.h"
#include "OptionalContent.h"

//------------------------------------------------------------------------

#define ocPolicyAllOn  1
#define ocPolicyAnyOn  2
#define ocPolicyAnyOff 3
#define ocPolicyAllOff 4

//------------------------------------------------------------------------

// Max depth of nested visibility expressions.  This is used to catch
// infinite loops in the visibility expression object structure.
#define visibilityExprRecursionLimit 50

// Max depth of nested display nodes.  This is used to catch infinite
// loops in the "Order" object structure.
#define displayNodeRecursionLimit 50

//------------------------------------------------------------------------

OptionalContent::OptionalContent(PDFDoc *doc) {
  Object *ocProps;
  Object ocgList, defView, obj1, obj2;
  Ref ref1;
  OptionalContentGroup *ocg;
  int i;

  xref = doc->getXRef();
  ocgs = new GList();
  display = NULL;

  if ((ocProps = doc->getCatalog()->getOCProperties())->isDict()) {
    if (ocProps->dictLookup("OCGs", &ocgList)->isArray()) {

      //----- read the OCG list
      for (i = 0; i < ocgList.arrayGetLength(); ++i) {
	if (ocgList.arrayGetNF(i, &obj1)->isRef()) {
	  ref1 = obj1.getRef();
	  obj1.fetch(xref, &obj2);
	  if ((ocg = OptionalContentGroup::parse(&ref1, &obj2))) {
	    ocgs->append(ocg);
	  }
	  obj2.free();
	}
	obj1.free();
      }

      //----- read the default viewing OCCD
      if (ocProps->dictLookup("D", &defView)->isDict()) {

	//----- initial state
	if (defView.dictLookup("OFF", &obj1)->isArray()) {
	  for (i = 0; i < obj1.arrayGetLength(); ++i) {
	    if (obj1.arrayGetNF(i, &obj2)->isRef()) {
	      ref1 = obj2.getRef();
	      if ((ocg = findOCG(&ref1))) {
		ocg->setState(gFalse);
	      } else {
		error(errSyntaxError, -1,
		      "Invalid OCG reference in OFF array in default viewing OCCD");
	      }
	    }
	    obj2.free();
	  }
	}
	obj1.free();

	//----- display order
	if (defView.dictLookup("Order", &obj1)->isArray()) {
	  display = OCDisplayNode::parse(&obj1, this, xref);
	}
	obj1.free();

      } else {
	error(errSyntaxError, -1, "Missing or invalid default viewing OCCD");
      }
      defView.free();

    }
    ocgList.free();
  }

  if (!display) {
    display = new OCDisplayNode();
  }
}

OptionalContent::~OptionalContent() {
  deleteGList(ocgs, OptionalContentGroup);
  delete display;
}

int OptionalContent::getNumOCGs() {
  return ocgs->getLength();
}

OptionalContentGroup *OptionalContent::getOCG(int idx) {
  return (OptionalContentGroup *)ocgs->get(idx);
}

OptionalContentGroup *OptionalContent::findOCG(Ref *ref) {
  OptionalContentGroup *ocg;
  int i;

  for (i = 0; i < ocgs->getLength(); ++i) {
    ocg = (OptionalContentGroup *)ocgs->get(i);
    if (ocg->matches(ref)) {
      return ocg;
    }
  }
  return NULL;
}

GBool OptionalContent::evalOCObject(Object *obj, GBool *visible) {
  OptionalContentGroup *ocg;
  int policy;
  Ref ref;
  Object obj2, obj3, obj4, obj5;
  int i;

  if (obj->isNull()) {
    return gFalse;
  }
  if (obj->isRef()) {
    ref = obj->getRef();
    if ((ocg = findOCG(&ref))) {
      *visible = ocg->getState();
      return gTrue;
    }
  }
  obj->fetch(xref, &obj2);
  if (obj2.isDict("OCMD")) {
    if (obj2.dictLookup("VE", &obj3)->isArray()) {
      *visible = evalOCVisibilityExpr(&obj3, 0);
      obj3.free();
    } else {
      obj3.free();
      policy = ocPolicyAnyOn;
      if (obj2.dictLookup("P", &obj3)->isName()) {
	if (obj3.isName("AllOn")) {
	  policy = ocPolicyAllOn;
	} else if (obj3.isName("AnyOn")) {
	  policy = ocPolicyAnyOn;
	} else if (obj3.isName("AnyOff")) {
	  policy = ocPolicyAnyOff;
	} else if (obj3.isName("AllOff")) {
	  policy = ocPolicyAllOff;
	}
      }
      obj3.free();
      obj2.dictLookupNF("OCGs", &obj3);
      ocg = NULL;
      if (obj3.isRef()) {
	ref = obj3.getRef();
	ocg = findOCG(&ref);
      }
      if (ocg) {
	*visible = (policy == ocPolicyAllOn || policy == ocPolicyAnyOn) ?
	             ocg->getState() : !ocg->getState();
      } else {
	*visible = gTrue;
	if (obj3.fetch(xref, &obj4)->isArray()) {
	  for (i = 0; i < obj4.arrayGetLength(); ++i) {
	    obj4.arrayGetNF(i, &obj5);
	    if (obj5.isRef()) {
	      ref = obj5.getRef();
	      if ((ocg = findOCG(&ref))) {
		switch (policy) {
		case ocPolicyAllOn:
		  *visible = *visible && ocg->getState();
		  break;
		case ocPolicyAnyOn:
		  *visible = *visible || ocg->getState();
		  break;
		case ocPolicyAnyOff:
		  *visible = *visible || !ocg->getState();
		  break;
		case ocPolicyAllOff:
		  *visible = *visible && !ocg->getState();
		  break;
		}
	      }
	    }
	    obj5.free();
	  }
	}
	obj4.free();
      }
      obj3.free();
    }
    obj2.free();
    return gTrue;
  }
  obj2.free();
  return gFalse;
}

GBool OptionalContent::evalOCVisibilityExpr(Object *expr, int recursion) {
  OptionalContentGroup *ocg;
  Object expr2, op, obj;
  Ref ref;
  GBool ret;
  int i;

  if (recursion > visibilityExprRecursionLimit) {
    error(errSyntaxError, -1,
	  "Loop detected in optional content visibility expression");
    return gTrue;
  }
  if (expr->isRef()) {
    ref = expr->getRef();
    if ((ocg = findOCG(&ref))) {
      return ocg->getState();
    }
  }
  expr->fetch(xref, &expr2);
  if (!expr2.isArray() || expr2.arrayGetLength() < 1) {
    error(errSyntaxError, -1,
	  "Invalid optional content visibility expression");
    expr2.free();
    return gTrue;
  }
  expr2.arrayGet(0, &op);
  if (op.isName("Not")) {
    if (expr2.arrayGetLength() == 2) {
      expr2.arrayGetNF(1, &obj);
      ret = !evalOCVisibilityExpr(&obj, recursion + 1);
      obj.free();
    } else {
      error(errSyntaxError, -1,
	    "Invalid optional content visibility expression");
      ret = gTrue;
    }
  } else if (op.isName("And")) {
    ret = gTrue;
    for (i = 1; i < expr2.arrayGetLength() && ret; ++i) {
      expr2.arrayGetNF(i, &obj);
      ret = evalOCVisibilityExpr(&obj, recursion + 1);
      obj.free();
    }
  } else if (op.isName("Or")) {
    ret = gFalse;
    for (i = 1; i < expr2.arrayGetLength() && !ret; ++i) {
      expr2.arrayGetNF(i, &obj);
      ret = evalOCVisibilityExpr(&obj, recursion + 1);
      obj.free();
    }
  } else {
    error(errSyntaxError, -1,
	  "Invalid optional content visibility expression");
    ret = gTrue;
  }
  op.free();
  expr2.free();
  return ret;
}

//------------------------------------------------------------------------

OptionalContentGroup *OptionalContentGroup::parse(Ref *refA, Object *obj) {
  Unicode *nameA;
  int nameLenA;
  Object obj1, obj2, obj3;
  GString *s;
  OCUsageState viewStateA, printStateA;
  int i;

  if (!obj->isDict()) {
    return NULL;
  }
  if (!obj->dictLookup("Name", &obj1)->isString()) {
    error(errSyntaxError, -1, "Missing or invalid Name in OCG");
    obj1.free();
    return NULL;
  }
  s = obj1.getString();
  if ((s->getChar(0) & 0xff) == 0xfe &&
      (s->getChar(1) & 0xff) == 0xff) {
    nameLenA = (s->getLength() - 2) / 2;
    nameA = (Unicode *)gmallocn(nameLenA, sizeof(Unicode));
    for (i = 0; i < nameLenA; ++i) {
      nameA[i] = ((s->getChar(2 + 2*i) & 0xff) << 8) |
	         (s->getChar(3 + 2*i) & 0xff);
    }
  } else {
    nameLenA = s->getLength();
    nameA = (Unicode *)gmallocn(nameLenA, sizeof(Unicode));
    for (i = 0; i < nameLenA; ++i) {
      nameA[i] = pdfDocEncoding[s->getChar(i) & 0xff];
    }
  }
  obj1.free();

  viewStateA = printStateA = ocUsageUnset;
  if (obj->dictLookup("Usage", &obj1)->isDict()) {
    if (obj1.dictLookup("View", &obj2)->isDict()) {
      if (obj2.dictLookup("ViewState", &obj3)->isName()) {
	if (obj3.isName("ON")) {
	  viewStateA = ocUsageOn;
	} else {
	  viewStateA = ocUsageOff;
	}
      }
      obj3.free();
    }
    obj2.free();
    if (obj1.dictLookup("Print", &obj2)->isDict()) {
      if (obj2.dictLookup("PrintState", &obj3)->isName()) {
	if (obj3.isName("ON")) {
	  printStateA = ocUsageOn;
	} else {
	  printStateA = ocUsageOff;
	}
      }
      obj3.free();
    }
    obj2.free();
  }
  obj1.free();

  return new OptionalContentGroup(refA, nameA, nameLenA,
				  viewStateA, printStateA);
}

OptionalContentGroup::OptionalContentGroup(Ref *refA, Unicode *nameA,
					   int nameLenA,
					   OCUsageState viewStateA,
					   OCUsageState printStateA) {
  ref = *refA;
  name = nameA;
  nameLen = nameLenA;
  viewState = viewStateA;
  printState = printStateA;
  state = gTrue;
}

OptionalContentGroup::~OptionalContentGroup() {
  gfree(name);
}

GBool OptionalContentGroup::matches(Ref *refA) {
  return refA->num == ref.num && refA->gen == ref.gen;
}

//------------------------------------------------------------------------

OCDisplayNode *OCDisplayNode::parse(Object *obj, OptionalContent *oc,
				    XRef *xref, int recursion) {
  Object obj2, obj3;
  Ref ref;
  OptionalContentGroup *ocgA;
  OCDisplayNode *node, *child;
  int i;

  if (recursion > displayNodeRecursionLimit) {
    error(errSyntaxError, -1, "Loop detected in optional content order");
    return NULL;
  }
  if (obj->isRef()) {
    ref = obj->getRef();
    if ((ocgA = oc->findOCG(&ref))) {
      return new OCDisplayNode(ocgA);
    }
  }
  obj->fetch(xref, &obj2);
  if (!obj2.isArray()) {
    obj2.free();
    return NULL;
  }
  i = 0;
  if (obj2.arrayGetLength() >= 1) {
    if (obj2.arrayGet(0, &obj3)->isString()) {
      node = new OCDisplayNode(obj3.getString());
      i = 1;
    } else {
      node = new OCDisplayNode();
    }
    obj3.free();
  } else {
    node = new OCDisplayNode();
  }
  for (; i < obj2.arrayGetLength(); ++i) {
    obj2.arrayGetNF(i, &obj3);
    if ((child = OCDisplayNode::parse(&obj3, oc, xref, recursion + 1))) {
      if (!child->ocg && !child->name && node->getNumChildren() > 0) {
	node->getChild(node->getNumChildren() - 1)->
	          addChildren(child->takeChildren());
	delete child;
      } else {
	node->addChild(child);
      }
    }
    obj3.free();
  }
  obj2.free();
  return node;
}

OCDisplayNode::OCDisplayNode() {
  name = NULL;
  nameLen = 0;
  ocg = NULL;
  children = NULL;
}

OCDisplayNode::OCDisplayNode(GString *nameA) {
  int i;

  if ((nameA->getChar(0) & 0xff) == 0xfe &&
      (nameA->getChar(1) & 0xff) == 0xff) {
    nameLen = (nameA->getLength() - 2) / 2;
    name = (Unicode *)gmallocn(nameLen, sizeof(Unicode));
    for (i = 0; i < nameLen; ++i) {
      name[i] = ((nameA->getChar(2 + 2*i) & 0xff) << 8) |
	        (nameA->getChar(3 + 2*i) & 0xff);
    }
  } else {
    nameLen = nameA->getLength();
    name = (Unicode *)gmallocn(nameLen, sizeof(Unicode));
    for (i = 0; i < nameLen; ++i) {
      name[i] = pdfDocEncoding[nameA->getChar(i) & 0xff];
    }
  }
  ocg = NULL;
  children = NULL;
}

OCDisplayNode::OCDisplayNode(OptionalContentGroup *ocgA) {
  nameLen = ocgA->getNameLength();
  if (nameLen) {
    name = (Unicode *)gmallocn(nameLen, sizeof(Unicode));
    memcpy(name, ocgA->getName(), nameLen * sizeof(Unicode));
  } else {
    name = NULL;
  }
  ocg = ocgA;
  children = NULL;
}

void OCDisplayNode::addChild(OCDisplayNode *child) {
  if (!children) {
    children = new GList();
  }
  children->append(child);
}

void OCDisplayNode::addChildren(GList *childrenA) {
  if (!children) {
    children = new GList();
  }
  children->append(childrenA);
  delete childrenA;
}

GList *OCDisplayNode::takeChildren() {
  GList *childrenA;

  childrenA = children;
  children = NULL;
  return childrenA;
}

OCDisplayNode::~OCDisplayNode() {
  gfree(name);
  if (children) {
    deleteGList(children, OCDisplayNode);
  }
}

int OCDisplayNode::getNumChildren() {
  if (!children) {
    return 0;
  }
  return children->getLength();
}

OCDisplayNode *OCDisplayNode::getChild(int idx) {
  return (OCDisplayNode *)children->get(idx);
}
