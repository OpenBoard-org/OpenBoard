// =========================================================================
//
// tinyxmldom.js - an XML DOM parser in JavaScript compressed for downloading
//
//	This is the classic DOM that has shipped with XML for <SCRIPT>
//  since the beginning. For a more standards-compliant DOM, you may
//  wish to use the standards-compliant W3C DOM that is included
//  with XML for <SCRIPT> versions 3.0 and above
//
//
// version 3.1
//
// =========================================================================
//
// Copyright (C) 2000 - 2002, 2003 Michael Houghton (mike@idle.org), Raymond Irving and David Joham (djoham@yahoo.com)
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Visit the XML for <SCRIPT> home page at http://xmljs.sourceforge.net
//

var whitespace = "\n\r\t "; var quotes = "\"'"; function convertEscapes(str) { var gt; gt = -1; while (str.indexOf("&lt;", gt + 1) > -1) { var gt = str.indexOf("&lt;", gt + 1); var newStr = str.substr(0, gt); newStr += "<"; newStr = newStr + str.substr(gt + 4, str.length); str = newStr;}
gt = -1; while (str.indexOf("&gt;", gt + 1) > -1) { var gt = str.indexOf("&gt;", gt + 1); var newStr = str.substr(0, gt); newStr += ">"; newStr = newStr + str.substr(gt + 4, str.length); str = newStr;}
gt = -1; while (str.indexOf("&amp;", gt + 1) > -1) { var gt = str.indexOf("&amp;", gt + 1); var newStr = str.substr(0, gt); newStr += "&"; newStr = newStr + str.substr(gt + 5, str.length); str = newStr;}
return str;}
function convertToEscapes(str) { var gt = -1; while (str.indexOf("&", gt + 1) > -1) { gt = str.indexOf("&", gt + 1); var newStr = str.substr(0, gt); newStr += "&amp;"; newStr = newStr + str.substr(gt + 1, str.length); str = newStr;}
gt = -1; while (str.indexOf("<", gt + 1) > -1) { var gt = str.indexOf("<", gt + 1); var newStr = str.substr(0, gt); newStr += "&lt;"; newStr = newStr + str.substr(gt + 1, str.length); str = newStr;}
gt = -1; while (str.indexOf(">", gt + 1) > -1) { var gt = str.indexOf(">", gt + 1); var newStr = str.substr(0, gt); newStr += "&gt;"; newStr = newStr + str.substr(gt + 1, str.length); str = newStr;}
return str;}
function _displayElement(domElement, strRet) { if(domElement==null) { return;}
if(!(domElement.nodeType=='ELEMENT')) { return;}
var tagName = domElement.tagName; var tagInfo = ""; tagInfo = "<" + tagName; var attributeList = domElement.getAttributeNames(); for(var intLoop = 0; intLoop < attributeList.length; intLoop++) { var attribute = attributeList[intLoop]; tagInfo = tagInfo + " " + attribute + "="; tagInfo = tagInfo + "\"" + domElement.getAttribute(attribute) + "\"";}
tagInfo = tagInfo + ">"; strRet=strRet+tagInfo; if(domElement.children!=null) { var domElements = domElement.children; for(var intLoop = 0; intLoop < domElements.length; intLoop++) { var childNode = domElements[intLoop]; if(childNode.nodeType=='COMMENT') { strRet = strRet + "<!--" + childNode.content + "-->";}
else if(childNode.nodeType=='TEXT') { var cont = trim(childNode.content,true,true); strRet = strRet + childNode.content;}
else if (childNode.nodeType=='CDATA') { var cont = trim(childNode.content,true,true); strRet = strRet + "<![CDATA[" + cont + "]]>";}
else { strRet = _displayElement(childNode, strRet);}
}
}
strRet = strRet + "</" + tagName + ">"; return strRet;}
function firstWhiteChar(str,pos) { if (isEmpty(str)) { return -1;}
while(pos < str.length) { if (whitespace.indexOf(str.charAt(pos))!=-1) { return pos;}
else { pos++;}
}
return str.length;}
function isEmpty(str) { return (str==null) || (str.length==0);}
function trim(trimString, leftTrim, rightTrim) { if (isEmpty(trimString)) { return "";}
if (leftTrim == null) { leftTrim = true;}
if (rightTrim == null) { rightTrim = true;}
var left=0; var right=0; var i=0; var k=0; if (leftTrim == true) { while ((i<trimString.length) && (whitespace.indexOf(trimString.charAt(i++))!=-1)) { left++;}
}
if (rightTrim == true) { k=trimString.length-1; while((k>=left) && (whitespace.indexOf(trimString.charAt(k--))!=-1)) { right++;}
}
return trimString.substring(left, trimString.length - right);}
function XMLDoc(source, errFn) { this.topNode=null; this.errFn = errFn; this.createXMLNode = _XMLDoc_createXMLNode; this.error = _XMLDoc_error; this.getUnderlyingXMLText = _XMLDoc_getUnderlyingXMLText; this.handleNode = _XMLDoc_handleNode; this.hasErrors = false; this.insertNodeAfter = _XMLDoc_insertNodeAfter; this.insertNodeInto = _XMLDoc_insertNodeInto; this.loadXML = _XMLDoc_loadXML; this.parse = _XMLDoc_parse; this.parseAttribute = _XMLDoc_parseAttribute; this.parseDTD = _XMLDoc_parseDTD; this.parsePI = _XMLDoc_parsePI; this.parseTag = _XMLDoc_parseTag; this.removeNodeFromTree = _XMLDoc_removeNodeFromTree; this.replaceNodeContents = _XMLDoc_replaceNodeContents; this.selectNode = _XMLDoc_selectNode; this.selectNodeText = _XMLDoc_selectNodeText; this.source = source; if (this.parse()) { if (this.topNode!=null) { return this.error("expected close " + this.topNode.tagName);}
else { return true;}
}
}
function _XMLDoc_createXMLNode(strXML) { return new XMLDoc(strXML, this.errFn).docNode;}
function _XMLDoc_error(str) { this.hasErrors=true; if(this.errFn){ this.errFn("ERROR: " + str);}else if(this.onerror){ this.onerror("ERROR: " + str);}
return 0;}
function _XMLDoc_getTagNameParams(tag,obj){ var elm=-1,e,s=tag.indexOf('['); var attr=[]; if(s>=0){ e=tag.indexOf(']'); if(e>=0)elm=tag.substr(s+1,(e-s)-1); else obj.error('expected ] near '+tag); tag=tag.substr(0,s); if(isNaN(elm) && elm!='*'){ attr=elm.substr(1,elm.length-1); attr=attr.split('='); if(attr[1]) { s=attr[1].indexOf('"'); attr[1]=attr[1].substr(s+1,attr[1].length-1); e=attr[1].indexOf('"'); if(e>=0) attr[1]=attr[1].substr(0,e); else obj.error('expected " near '+tag)
};elm=-1;}else if(elm=='*') elm=-1;}
return [tag,elm,attr[0],attr[1]]
}
function _XMLDoc_getUnderlyingXMLText() { var strRet = ""; strRet = strRet + "<?xml version=\"1.0\"?>"; if (this.docNode==null) { return;}
strRet = _displayElement(this.docNode, strRet); return strRet;}
function _XMLDoc_handleNode(current) { if ((current.nodeType=='COMMENT') && (this.topNode!=null)) { return this.topNode.addElement(current);}
else if ((current.nodeType=='TEXT') || (current.nodeType=='CDATA')) { if(this.topNode==null) { if (trim(current.content,true,false)=="") { return true;}
else { return this.error("expected document node, found: " + current);}
}
else { return this.topNode.addElement(current);}
}
else if ((current.nodeType=='OPEN') || (current.nodeType=='SINGLE')) { var success = false; if(this.topNode==null) { this.docNode = current; current.parent = null; success = true;}
else { success = this.topNode.addElement(current);}
if (success && (current.nodeType!='SINGLE')) { this.topNode = current;}
current.nodeType = "ELEMENT"; return success;}
else if (current.nodeType=='CLOSE') { if (this.topNode==null) { return this.error("close tag without open: " + current.toString());}
else { if (current.tagName!=this.topNode.tagName) { return this.error("expected closing " + this.topNode.tagName + ", found closing " + current.tagName);}
else { this.topNode = this.topNode.getParent();}
}
}
return true;}
function _XMLDoc_insertNodeAfter (referenceNode, newNode) { var parentXMLText = this.getUnderlyingXMLText(); var selectedNodeXMLText = referenceNode.getUnderlyingXMLText(); var originalNodePos = parentXMLText.indexOf(selectedNodeXMLText) + selectedNodeXMLText.length; var newXML = parentXMLText.substr(0,originalNodePos); newXML += newNode.getUnderlyingXMLText(); newXML += parentXMLText.substr(originalNodePos); var newDoc = new XMLDoc(newXML, this.errFn); return newDoc;}
function _XMLDoc_insertNodeInto (referenceNode, insertNode) { var parentXMLText = this.getUnderlyingXMLText(); var selectedNodeXMLText = referenceNode.getUnderlyingXMLText(); var endFirstTag = selectedNodeXMLText.indexOf(">") + 1; var originalNodePos = parentXMLText.indexOf(selectedNodeXMLText) + endFirstTag; var newXML = parentXMLText.substr(0,originalNodePos); newXML += insertNode.getUnderlyingXMLText(); newXML += parentXMLText.substr(originalNodePos); var newDoc = new XMLDoc(newXML, this.errFn); return newDoc;}
function _XMLDoc_loadXML(source){ this.topNode=null; this.hasErrors = false; this.source=source; return this.parse();}
function _XMLDoc_parse() { var pos = 0; err = false; while(!err) { var closing_tag_prefix = ''; var chpos = this.source.indexOf('<',pos); var open_length = 1; var open; var close; if (chpos ==-1) { break;}
open = chpos; var str = this.source.substring(pos, open); if (str.length!=0) { err = !this.handleNode(new XMLNode('TEXT',this, str));}
if (chpos == this.source.indexOf("<?",pos)) { pos = this.parsePI(this.source, pos + 2); if (pos==0) { err=true;}
continue;}
if (chpos == this.source.indexOf("<!DOCTYPE",pos)) { pos = this.parseDTD(this.source, chpos+ 9); if (pos==0) { err=true;}
continue;}
if(chpos == this.source.indexOf('<!--',pos)) { open_length = 4; closing_tag_prefix = '--';}
if (chpos == this.source.indexOf('<![CDATA[',pos)) { open_length = 9; closing_tag_prefix = ']]';}
chpos = this.source.indexOf(closing_tag_prefix + '>',chpos); if (chpos ==-1) { return this.error("expected closing tag sequence: " + closing_tag_prefix + '>');}
close = chpos + closing_tag_prefix.length; str = this.source.substring(open+1, close); var n = this.parseTag(str); if (n) { err = !this.handleNode(n);}
pos = close +1;}
return !err;}
function _XMLDoc_parseAttribute(src,pos,node) { while ((pos<src.length) && (whitespace.indexOf(src.charAt(pos))!=-1)) { pos++;}
if (pos >= src.length) { return pos;}
var p1 = pos; while ((pos < src.length) && (src.charAt(pos)!='=')) { pos++;}
var msg = "attributes must have values"; if(pos >= src.length) { return this.error(msg);}
var paramname = trim(src.substring(p1,pos++),false,true); while ((pos < src.length) && (whitespace.indexOf(src.charAt(pos))!=-1)) { pos++;}
if (pos >= src.length) { return this.error(msg);}
msg = "attribute values must be in quotes"; var quote = src.charAt(pos++); if (quotes.indexOf(quote)==-1) { return this.error(msg);}
p1 = pos; while ((pos < src.length) && (src.charAt(pos)!=quote)) { pos++;}
if (pos >= src.length) { return this.error(msg);}
if (!node.addAttribute(paramname,trim(src.substring(p1,pos++),false,true))) { return 0;}
return pos;}
function _XMLDoc_parseDTD(str,pos) { var firstClose = str.indexOf('>',pos); if (firstClose==-1) { return this.error("error in DTD: expected '>'");}
var closing_tag_prefix = ''; var firstOpenSquare = str.indexOf('[',pos); if ((firstOpenSquare!=-1) && (firstOpenSquare < firstClose)) { closing_tag_prefix = ']';}
while(true) { var closepos = str.indexOf(closing_tag_prefix + '>',pos); if (closepos ==-1) { return this.error("expected closing tag sequence: " + closing_tag_prefix + '>');}
pos = closepos + closing_tag_prefix.length +1; if (str.substring(closepos-1,closepos+2) != ']]>') { break;}
}
return pos;}
function _XMLDoc_parsePI(str,pos) { var closepos = str.indexOf('?>',pos); return closepos + 2;}
function _XMLDoc_parseTag(src) { if (src.indexOf('!--')==0) { return new XMLNode('COMMENT', this, src.substring(3,src.length-2));}
if (src.indexOf('![CDATA[')==0) { return new XMLNode('CDATA', this, src.substring(8,src.length-2));}
var n = new XMLNode(); n.doc = this; if (src.charAt(0)=='/') { n.nodeType = 'CLOSE'; src = src.substring(1);}
else { n.nodeType = 'OPEN';}
if (src.charAt(src.length-1)=='/') { if (n.nodeType=='CLOSE') { return this.error("singleton close tag");}
else { n.nodeType = 'SINGLE';}
src = src.substring(0,src.length-1);}
if (n.nodeType!='CLOSE') { n.attributes = new Array();}
if (n.nodeType=='OPEN') { n.children = new Array();}
src = trim(src,true,true); if (src.length==0) { return this.error("empty tag");}
var endOfName = firstWhiteChar(src,0); if (endOfName==-1) { n.tagName = src; return n;}
n.tagName = src.substring(0,endOfName); var pos = endOfName; while(pos< src.length) { pos = this.parseAttribute(src, pos, n); if (this.pos==0) { return null;}
}
return n;}
function _XMLDoc_removeNodeFromTree(node) { var parentXMLText = this.getUnderlyingXMLText(); var selectedNodeXMLText = node.getUnderlyingXMLText(); var originalNodePos = parentXMLText.indexOf(selectedNodeXMLText); var newXML = parentXMLText.substr(0,originalNodePos); newXML += parentXMLText.substr(originalNodePos + selectedNodeXMLText.length); var newDoc = new XMLDoc(newXML, this.errFn); return newDoc;}
function _XMLDoc_replaceNodeContents(referenceNode, newContents) { var newNode = this.createXMLNode("<X>" + newContents + "</X>"); referenceNode.children = newNode.children; return this;}
function _XMLDoc_selectNode(tagpath){ tagpath = trim(tagpath, true, true); var srcnode,node,tag,params,elm,rg; var tags,attrName,attrValue,ok; srcnode=node=((this.source)?this.docNode:this); if (!tagpath) return node; if(tagpath.indexOf('/')==0)tagpath=tagpath.substr(1); tagpath=tagpath.replace(tag,''); tags=tagpath.split('/'); tag=tags[0]; if(tag){ if(tagpath.indexOf('/')==0)tagpath=tagpath.substr(1); tagpath=tagpath.replace(tag,''); params=_XMLDoc_getTagNameParams(tag,this); tag=params[0];elm=params[1]; attrName=params[2];attrValue=params[3]; node=(tag=='*')? node.getElements():node.getElements(tag); if (node.length) { if(elm<0){ srcnode=node;var i=0; while(i<srcnode.length){ if(attrName){ if (srcnode[i].getAttribute(attrName)!=attrValue) ok=false; else ok=true;}else ok=true; if(ok){ node=srcnode[i].selectNode(tagpath); if(node) return node;}
i++;}
}else if (elm<node.length){ node=node[elm].selectNode(tagpath); if(node) return node;}
}
}
}
function _XMLDoc_selectNodeText(tagpath){ var node=this.selectNode(tagpath); if (node != null) { return node.getText();}
else { return null;}
}
function XMLNode(nodeType,doc, str) { if (nodeType=='TEXT' || nodeType=='CDATA' || nodeType=='COMMENT' ) { this.content = str;}
else { this.content = null;}
this.attributes = null; this.children = null; this.doc = doc; this.nodeType = nodeType; this.parent = ""; this.tagName = ""; this.addAttribute = _XMLNode_addAttribute; this.addElement = _XMLNode_addElement; this.getAttribute = _XMLNode_getAttribute; this.getAttributeNames = _XMLNode_getAttributeNames; this.getElementById = _XMLNode_getElementById; this.getElements = _XMLNode_getElements; this.getText = _XMLNode_getText; this.getParent = _XMLNode_getParent; this.getUnderlyingXMLText = _XMLNode_getUnderlyingXMLText; this.removeAttribute = _XMLNode_removeAttribute; this.selectNode = _XMLDoc_selectNode; this.selectNodeText = _XMLDoc_selectNodeText; this.toString = _XMLNode_toString;}
function _XMLNode_addAttribute(attributeName,attributeValue) { this.attributes['_' + attributeName] = attributeValue; return true;}
function _XMLNode_addElement(node) { node.parent = this; this.children[this.children.length] = node; return true;}
function _XMLNode_getAttribute(name) { if (this.attributes == null) { return null;}
return this.attributes['_' + name];}
function _XMLNode_getAttributeNames() { if (this.attributes == null) { var ret = new Array(); return ret;}
var attlist = new Array(); for (var a in this.attributes) { attlist[attlist.length] = a.substring(1);}
return attlist;}
function _XMLNode_getElementById(id) { var node = this; var ret; if (node.getAttribute("id") == id) { return node;}
else{ var elements = node.getElements(); var intLoop = 0; while (intLoop < elements.length) { var element = elements[intLoop]; ret = element.getElementById(id); if (ret != null) { break;}
intLoop++;}
}
return ret;}
function _XMLNode_getElements(byName) { if (this.children==null) { var ret = new Array(); return ret;}
var elements = new Array(); for (var i=0; i<this.children.length; i++) { if ((this.children[i].nodeType=='ELEMENT') && ((byName==null) || (this.children[i].tagName == byName))) { elements[elements.length] = this.children[i];}
}
return elements;}
function _XMLNode_getText() { if (this.nodeType=='ELEMENT') { if (this.children==null) { return null;}
var str = ""; for (var i=0; i < this.children.length; i++) { var t = this.children[i].getText(); str += (t == null ? "" : t);}
return str;}
else if (this.nodeType=='TEXT') { return convertEscapes(this.content);}
else { return this.content;}
}
function _XMLNode_getParent() { return this.parent;}
function _XMLNode_getUnderlyingXMLText() { var strRet = ""; strRet = _displayElement(this, strRet); return strRet;}
function _XMLNode_removeAttribute(attributeName) { if(attributeName == null) { return this.doc.error("You must pass an attribute name into the removeAttribute function");}
var attributes = this.getAttributeNames(); var intCount = attributes.length; var tmpAttributeValues = new Array(); for ( intLoop = 0; intLoop < intCount; intLoop++) { tmpAttributeValues[intLoop] = this.getAttribute(attributes[intLoop]);}
this.attributes = new Array(); for (intLoop = 0; intLoop < intCount; intLoop++) { if ( attributes[intLoop] != attributeName) { this.addAttribute(attributes[intLoop], tmpAttributeValues[intLoop]);}
}
return true;}
function _XMLNode_toString() { return "" + this.nodeType + ":" + (this.nodeType=='TEXT' || this.nodeType=='CDATA' || this.nodeType=='COMMENT' ? this.content : this.tagName);}
