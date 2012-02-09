/**
* DD_roundies, this adds rounded-corner CSS in standard browsers and VML sublayers in IE that accomplish a similar appearance when comparing said browsers.
* Author: Drew Diller
* Email: drew.diller@gmail.com
* URL: http://www.dillerdesign.com/experiment/DD_roundies/
* Version: 0.0.2a
* Licensed under the MIT License: http://dillerdesign.com/experiment/DD_roundies/#license
*
* Usage:
* DD_roundies.addRule('#doc .container', '10px 5px'); // selector and multiple radii
* DD_roundies.addRule('.box', 5, true); // selector, radius, and optional addition of border-radius code for standard browsers.
* 
* Just want the PNG fixing effect for IE6, and don't want to also use the DD_belatedPNG library?  Don't give any additional arguments after the CSS selector.
* DD_roundies.addRule('.your .example img');
**/

var DD_roundies = {

	ns: 'DD_roundies',
	
	IE6: false,
	IE7: false,
	IE8: false,
	IEversion: function() {
		if (document.documentMode != 8 && document.namespaces && !document.namespaces[this.ns]) {
			this.IE6 = true;
			this.IE7 = true;
		}
		else if (document.documentMode == 8) {
			this.IE8 = true;
		}
	},
	querySelector: document.querySelectorAll,
	selectorsToProcess: [],
	imgSize: {},
	
	createVmlNameSpace: function() { /* enable VML */
		if (this.IE6 || this.IE7) {
			document.namespaces.add(this.ns, 'urn:schemas-microsoft-com:vml');
		}
		if (this.IE8) {
			document.writeln('<?import namespace="' + this.ns + '" implementation="#default#VML" ?>');
		}
	},
	
	createVmlStyleSheet: function() { /* style VML, enable behaviors */
		/*
			Just in case lots of other developers have added
			lots of other stylesheets using document.createStyleSheet
			and hit the 31-limit mark, let's not use that method!
			further reading: http://msdn.microsoft.com/en-us/library/ms531194(VS.85).aspx
		*/
		var style = document.createElement('style');
		document.documentElement.firstChild.insertBefore(style, document.documentElement.firstChild.firstChild);
		if (style.styleSheet) { /* IE */
			try {
				var styleSheet = style.styleSheet;
				styleSheet.addRule(this.ns + '\\:*', '{behavior:url(#default#VML)}');
				this.styleSheet = styleSheet;
			} catch(err) {}
		}
		else {
			this.styleSheet = style;
		}
	},
	
	/**
	* Method to use from afar - refer to it whenever.
	* Example for IE only: DD_roundies.addRule('div.boxy_box', '10px 5px');
	* Example for IE, Firefox, and WebKit: DD_roundies.addRule('div.boxy_box', '10px 5px', true);
	* @param {String} selector - REQUIRED - a CSS selector, such as '#doc .container'
	* @param {Integer} radius - REQUIRED - the desired radius for the box corners
	* @param {Boolean} standards - OPTIONAL - true if you also wish to output -moz-border-radius/-webkit-border-radius/border-radius declarations
	**/
	addRule: function(selector, rad, standards) {
		if (typeof rad == 'undefined' || rad === null) {
			rad = 0;
		}
		if (rad.constructor.toString().search('Array') == -1) {
			rad = rad.toString().replace(/[^0-9 ]/g, '').split(' ');
		}
		for (var i=0; i<4; i++) {
			rad[i] = (!rad[i] && rad[i] !== 0) ? rad[Math.max((i-2), 0)] : rad[i];
		}
		if (this.styleSheet) {
			if (this.styleSheet.addRule) { /* IE */
				var selectors = selector.split(','); /* multiple selectors supported, no need for multiple calls to this anymore */
				for (var i=0; i<selectors.length; i++) {
					this.styleSheet.addRule(selectors[i], 'behavior:expression(DD_roundies.roundify.call(this, [' + rad.join(',') + ']))'); /* seems to execute the function without adding it to the stylesheet - interesting... */
				}
			}
			else if (standards) {
				var moz_implementation = rad.join('px ') + 'px';
				this.styleSheet.appendChild(document.createTextNode(selector + ' {border-radius:' + moz_implementation + '; -moz-border-radius:' + moz_implementation + ';}'));
				this.styleSheet.appendChild(document.createTextNode(selector + ' {-webkit-border-top-left-radius:' + rad[0] + 'px ' + rad[0] + 'px; -webkit-border-top-right-radius:' + rad[1] + 'px ' + rad[1] + 'px; -webkit-border-bottom-right-radius:' + rad[2] + 'px ' + rad[2] + 'px; -webkit-border-bottom-left-radius:' + rad[3] + 'px ' + rad[3] + 'px;}'));
			}
		}
		else if (this.IE8) {
			this.selectorsToProcess.push({'selector':selector, 'radii':rad});
		}
	},
	
	readPropertyChanges: function(el) {
		switch (event.propertyName) {
			case 'style.border':
			case 'style.borderWidth':
			case 'style.padding':
				this.applyVML(el);
				break;
			case 'style.borderColor':
				this.vmlStrokeColor(el);
				break;
			case 'style.backgroundColor':
			case 'style.backgroundPosition':
			case 'style.backgroundRepeat':
				this.applyVML(el);
				break;
			case 'style.display':
				el.vmlBox.style.display = (el.style.display == 'none') ? 'none' : 'block';
				break;
			case 'style.filter':
				this.vmlOpacity(el);
				break;
			case 'style.zIndex':
				el.vmlBox.style.zIndex = el.style.zIndex;
				break;
		}
	},
	
	applyVML: function(el) {
		el.runtimeStyle.cssText = '';
		this.vmlFill(el);
		this.vmlStrokeColor(el);
		this.vmlStrokeWeight(el);
		this.vmlOffsets(el);
		this.vmlPath(el);
		this.nixBorder(el);
		this.vmlOpacity(el);
	},
	
	vmlOpacity: function(el) {
		if (el.currentStyle.filter.search('lpha') != -1) {
			var trans = el.currentStyle.filter;
			trans = parseInt(trans.substring(trans.lastIndexOf('=')+1, trans.lastIndexOf(')')), 10)/100;
			for (var v in el.vml) {
				el.vml[v].filler.opacity = trans;
			}
		}
	},
	
	vmlFill: function(el) {
		if (!el.currentStyle) {
			return;
		} else {
			var elStyle = el.currentStyle;
		}
		el.runtimeStyle.backgroundColor = '';
		el.runtimeStyle.backgroundImage = '';
		var noColor = (elStyle.backgroundColor == 'transparent');
		var noImg = true;
		if (elStyle.backgroundImage != 'none' || el.isImg) {
			if (!el.isImg) {
				el.vmlBg = elStyle.backgroundImage;
				el.vmlBg = el.vmlBg.substr(5, el.vmlBg.lastIndexOf('")')-5);
			}
			else {
				el.vmlBg = el.src;
			}
			var lib = this;
			if (!lib.imgSize[el.vmlBg]) { /* determine size of loaded image */
				var img = document.createElement('img');
				img.attachEvent('onload', function() {
					this.width = this.offsetWidth; /* weird cache-busting requirement! */
					this.height = this.offsetHeight;
					lib.vmlOffsets(el);
				});
				img.className = lib.ns + '_sizeFinder';
				img.runtimeStyle.cssText = 'behavior:none; position:absolute; top:-10000px; left:-10000px; border:none;'; /* make sure to set behavior to none to prevent accidental matching of the helper elements! */
				img.src = el.vmlBg;
				img.removeAttribute('width');
				img.removeAttribute('height');
				document.body.insertBefore(img, document.body.firstChild);
				lib.imgSize[el.vmlBg] = img;
			}
			el.vml.image.filler.src = el.vmlBg;
			noImg = false;
		}
		el.vml.image.filled = !noImg;
		el.vml.image.fillcolor = 'none';
		el.vml.color.filled = !noColor;
		el.vml.color.fillcolor = elStyle.backgroundColor;
		el.runtimeStyle.backgroundImage = 'none';
		el.runtimeStyle.backgroundColor = 'transparent';
	},
	
	vmlStrokeColor: function(el) {
		el.vml.stroke.fillcolor = el.currentStyle.borderColor;
	},
	
	vmlStrokeWeight: function(el) {
		var borders = ['Top', 'Right', 'Bottom', 'Left'];
		el.bW = {};
		for (var b=0; b<4; b++) {
			el.bW[borders[b]] = parseInt(el.currentStyle['border' + borders[b] + 'Width'], 10) || 0;
		}
	},
	
	vmlOffsets: function(el) {
		var dims = ['Left', 'Top', 'Width', 'Height'];
		for (var d=0; d<4; d++) {
			el.dim[dims[d]] = el['offset'+dims[d]];
		}
		var assign = function(obj, topLeft) {
			obj.style.left = (topLeft ? 0 : el.dim.Left) + 'px';
			obj.style.top = (topLeft ? 0 : el.dim.Top) + 'px';
			obj.style.width = el.dim.Width + 'px';
			obj.style.height = el.dim.Height + 'px';
		};
		for (var v in el.vml) {
			var mult = (v == 'image') ? 1 : 2;
			el.vml[v].coordsize = (el.dim.Width*mult) + ', ' + (el.dim.Height*mult);
			assign(el.vml[v], true);
		}
		assign(el.vmlBox, false);
		
		if (DD_roundies.IE8) {
			el.vml.stroke.style.margin = '-1px';
			if (typeof el.bW == 'undefined') {
				this.vmlStrokeWeight(el);
			}
			el.vml.color.style.margin = (el.bW.Top-1) + 'px ' + (el.bW.Left-1) + 'px';
		}
	},
	
	vmlPath: function(el) {
		var coords = function(direction, w, h, r, aL, aT, mult) {
			var cmd = direction ? ['m', 'qy', 'l', 'qx', 'l', 'qy', 'l', 'qx', 'l'] : ['qx', 'l', 'qy', 'l', 'qx', 'l', 'qy', 'l', 'm']; /* whoa */
			aL *= mult;
			aT *= mult;
			w *= mult;
			h *= mult;
			var R = r.slice(); /* do not affect original array */
			for (var i=0; i<4; i++) {
				R[i] *= mult;
				R[i] = Math.min(w/2, h/2, R[i]); /* make sure you do not get funky shapes - pick the smallest: half of the width, half of the height, or current value */
			}
			var coords = [
				cmd[0] + Math.floor(0+aL) +','+ Math.floor(R[0]+aT),
				cmd[1] + Math.floor(R[0]+aL) +','+ Math.floor(0+aT),
				cmd[2] + Math.ceil(w-R[1]+aL) +','+ Math.floor(0+aT),
				cmd[3] + Math.ceil(w+aL) +','+ Math.floor(R[1]+aT),
				cmd[4] + Math.ceil(w+aL) +','+ Math.ceil(h-R[2]+aT),
				cmd[5] + Math.ceil(w-R[2]+aL) +','+ Math.ceil(h+aT),
				cmd[6] + Math.floor(R[3]+aL) +','+ Math.ceil(h+aT),
				cmd[7] + Math.floor(0+aL) +','+ Math.ceil(h-R[3]+aT),
				cmd[8] + Math.floor(0+aL) +','+ Math.floor(R[0]+aT)
			];
			if (!direction) {
				coords.reverse();
			}
			var path = coords.join('');
			return path;
		};
	
		if (typeof el.bW == 'undefined') {
			this.vmlStrokeWeight(el);
		}
		var bW = el.bW;
		var rad = el.DD_radii.slice();
		
		/* determine outer curves */
		var outer = coords(true, el.dim.Width, el.dim.Height, rad, 0, 0, 2);
		
		/* determine inner curves */
		rad[0] -= Math.max(bW.Left, bW.Top);
		rad[1] -= Math.max(bW.Top, bW.Right);
		rad[2] -= Math.max(bW.Right, bW.Bottom);
		rad[3] -= Math.max(bW.Bottom, bW.Left);
		for (var i=0; i<4; i++) {
			rad[i] = Math.max(rad[i], 0);
		}
		var inner = coords(false, el.dim.Width-bW.Left-bW.Right, el.dim.Height-bW.Top-bW.Bottom, rad, bW.Left, bW.Top, 2);
		var image = coords(true, el.dim.Width-bW.Left-bW.Right+1, el.dim.Height-bW.Top-bW.Bottom+1, rad, bW.Left, bW.Top, 1);
		
		/* apply huge path string */
		el.vml.color.path = inner;
		el.vml.image.path = image;
		el.vml.stroke.path = outer + inner;
		
		this.clipImage(el);
	},
	
	nixBorder: function(el) {
		var s = el.currentStyle;
		var sides = ['Top', 'Left', 'Right', 'Bottom'];
		for (var i=0; i<4; i++) {
			el.runtimeStyle['padding' + sides[i]] = (parseInt(s['padding' + sides[i]], 10) || 0) + (parseInt(s['border' + sides[i] + 'Width'], 10) || 0) + 'px';
		}
		el.runtimeStyle.border = 'none';
	},
	
	clipImage: function(el) {
		var lib = DD_roundies;
		if (!el.vmlBg || !lib.imgSize[el.vmlBg]) {
			return;
		}
		var thisStyle = el.currentStyle;
		var bg = {'X':0, 'Y':0};
		var figurePercentage = function(axis, position) {
			var fraction = true;
			switch(position) {
				case 'left':
				case 'top':
					bg[axis] = 0;
					break;
				case 'center':
					bg[axis] = 0.5;
					break;
				case 'right':
				case 'bottom':
					bg[axis] = 1;
					break;
				default:
					if (position.search('%') != -1) {
						bg[axis] = parseInt(position, 10) * 0.01;
					}
					else {
						fraction = false;
					}
			}
			var horz = (axis == 'X');
			bg[axis] = Math.ceil(fraction ? (( el.dim[horz ? 'Width' : 'Height'] - (el.bW[horz ? 'Left' : 'Top'] + el.bW[horz ? 'Right' : 'Bottom']) ) * bg[axis]) - (lib.imgSize[el.vmlBg][horz ? 'width' : 'height'] * bg[axis]) : parseInt(position, 10));
			bg[axis] += 1;
		};
		for (var b in bg) {
			figurePercentage(b, thisStyle['backgroundPosition'+b]);
		}
		el.vml.image.filler.position = (bg.X/(el.dim.Width-el.bW.Left-el.bW.Right+1)) + ',' + (bg.Y/(el.dim.Height-el.bW.Top-el.bW.Bottom+1));
		var bgR = thisStyle.backgroundRepeat;
		var c = {'T':1, 'R':el.dim.Width+1, 'B':el.dim.Height+1, 'L':1}; /* these are defaults for repeat of any kind */
		var altC = { 'X': {'b1': 'L', 'b2': 'R', 'd': 'Width'}, 'Y': {'b1': 'T', 'b2': 'B', 'd': 'Height'} };
		if (bgR != 'repeat') {
			c = {'T':(bg.Y), 'R':(bg.X+lib.imgSize[el.vmlBg].width), 'B':(bg.Y+lib.imgSize[el.vmlBg].height), 'L':(bg.X)}; /* these are defaults for no-repeat - clips down to the image location */
			if (bgR.search('repeat-') != -1) { /* now let's revert to dC for repeat-x or repeat-y */
				var v = bgR.split('repeat-')[1].toUpperCase();
				c[altC[v].b1] = 1;
				c[altC[v].b2] = el.dim[altC[v].d]+1;
			}
			if (c.B > el.dim.Height) {
				c.B = el.dim.Height+1;
			}
		}
		el.vml.image.style.clip = 'rect('+c.T+'px '+c.R+'px '+c.B+'px '+c.L+'px)';
	},
	
	pseudoClass: function(el) {
		var self = this;
		setTimeout(function() { /* would not work as intended without setTimeout */
			self.applyVML(el);
		}, 1);
	},
	
	reposition: function(el) {
		this.vmlOffsets(el);
		this.vmlPath(el);
	},
	
	roundify: function(rad) {
		this.style.behavior = 'none';
		if (!this.currentStyle) {
			return;
		}
		else {
			var thisStyle = this.currentStyle;
		}
		var allowed = {BODY: false, TABLE: false, TR: false, TD: false, SELECT: false, OPTION: false, TEXTAREA: false};
		if (allowed[this.nodeName] === false) { /* elements not supported yet */
			return;
		}
		var self = this; /* who knows when you might need a setTimeout */
		var lib = DD_roundies;
		this.DD_radii = rad;
		this.dim = {};

		/* attach handlers */
		var handlers = {resize: 'reposition', move: 'reposition'};
		if (this.nodeName == 'A') {
			var moreForAs = {mouseleave: 'pseudoClass', mouseenter: 'pseudoClass', focus: 'pseudoClass', blur: 'pseudoClass'};
			for (var a in moreForAs) {
				handlers[a] = moreForAs[a];
			}
		}
		for (var h in handlers) {
			this.attachEvent('on' + h, function() {
				lib[handlers[h]](self);
			});
		}
		this.attachEvent('onpropertychange', function() {
			lib.readPropertyChanges(self);
		});
		
		/* ensure that this elent and its parent is given hasLayout (needed for accurate positioning) */
		var giveLayout = function(el) {
			el.style.zoom = 1;
			if (el.currentStyle.position == 'static') {
				el.style.position = 'relative';
			}
		};
		giveLayout(this.offsetParent);
		giveLayout(this);
		
		/* create vml elements */
		this.vmlBox = document.createElement('ignore'); /* IE8 really wants to be encased in a wrapper element for the VML to work, and I don't want to disturb getElementsByTagName('div') - open to suggestion on how to do this differently */
		this.vmlBox.runtimeStyle.cssText = 'behavior:none; position:absolute; margin:0; padding:0; border:0; background:none;'; /* super important - if something accidentally matches this (you yourseld did this once, Drew), you'll get infinitely-created elements and a frozen browser! */
		this.vmlBox.style.zIndex = thisStyle.zIndex;
		this.vml = {'color':true, 'image':true, 'stroke':true};
		for (var v in this.vml) {
			this.vml[v] = document.createElement(lib.ns + ':shape');
			this.vml[v].filler = document.createElement(lib.ns + ':fill');
			this.vml[v].appendChild(this.vml[v].filler);
			this.vml[v].stroked = false;
			this.vml[v].style.position = 'absolute';
			this.vml[v].style.zIndex = thisStyle.zIndex;
			this.vml[v].coordorigin = '1,1';
			this.vmlBox.appendChild(this.vml[v]);
		}
		this.vml.image.fillcolor = 'none';
		this.vml.image.filler.type = 'tile';
		this.parentNode.insertBefore(this.vmlBox, this);
		
		this.isImg = false;
		if (this.nodeName == 'IMG') {
			this.isImg = true;
			this.style.visibility = 'hidden';
		}
		
		setTimeout(function() {
			lib.applyVML(self);
		}, 1);
	}
	
};

try {
	document.execCommand("BackgroundImageCache", false, true);
} catch(err) {}
DD_roundies.IEversion();
DD_roundies.createVmlNameSpace();
DD_roundies.createVmlStyleSheet();

if (DD_roundies.IE8 && document.attachEvent && DD_roundies.querySelector) {
	document.attachEvent('onreadystatechange', function() {
		if (document.readyState == 'complete') {
			var selectors = DD_roundies.selectorsToProcess;
			var length = selectors.length;
			var delayedCall = function(node, radii, index) {
				setTimeout(function() {
					DD_roundies.roundify.call(node, radii);
				}, index*100);
			};
			for (var i=0; i<length; i++) {
				var results = document.querySelectorAll(selectors[i].selector);
				var rLength = results.length;
				for (var r=0; r<rLength; r++) {
					if (results[r].nodeName != 'INPUT') { /* IE8 doesn't like to do this to inputs yet */
						delayedCall(results[r], selectors[i].radii, r);
					}
				}
			}
		}
	});
}