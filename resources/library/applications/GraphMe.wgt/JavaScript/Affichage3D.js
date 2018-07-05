
var fonction3D = false;
var outilPrecedent = "";

function activer3D(){
	if(fonction3D){ // Si activé alors on le désative
		fonction3D = false;
		outil.choisir(outilPrecedent);
		document.getElementById('onglet3D').innerHTML = "3D";
		document.getElementById('zoomButtons').style.display = "block";
		document.getElementById('toolButtons').style.display = "block";
		affichage.init();
		outil.init();
		affichage.initZoom2(document.getElementById('zoomDefaut').value);
	}
	else{ // Sinon on l'active
		fonction3D = true;
		outilPrecedent = outil.actuel;
		outil.choisir("deplacement");
		document.getElementById('onglet3D').innerHTML = "2D";
		document.getElementById('zoomButtons').style.display = "none";
		document.getElementById('toolButtons').style.display = "none";
		display3D.init();
		outil.liste = [];
		outil.init();
		message.supprimer();
	}
	cacherMenu();
	if(fonction3D){
		if(document.getElementById('input3D').value == ""){
			afficherMenu('menuFonctions3D');
		}
		else{
			display3D.draw();
		}
	}
	saveOptions();
}

var display3D = {
	canvas: null,
	ctx: null,
	width: 0,
	height: 0,
	centerX: 0,
	centerY: 0,
	scale: 50,
	linePrecision: 0.02,
	functionPrecision: 0.2,
	angle: Math.PI/8,
	zoomValue: 1,
	left: -6.5,
	right: 6.5,
	rouge3D: 0,
	vert3D: 1,
	bleu3D: 2,
	couleurGenerale: 0,
	background: "rgba(0, 0, 0, 0.5)",
	
	init: function(width, height){
		var displayElement = document.getElementById("affichage");
		
		// Clear displayElement content
		if(displayElement.hasChildNodes()){
			while(displayElement.childNodes.length >= 1 ){
				displayElement.removeChild(displayElement.firstChild);
			}
		}
		
		// Setup sizes
		this.width = width || displayElement.clientWidth;
		this.height = height || displayElement.clientHeight;
		this.centerX = this.width / 2;
		this.centerY = this.height / 2;
		
		// Create canvas
		this.canvas = document.createElement("canvas");
		this.canvas.width = this.width;
		this.canvas.height = this.height;
		displayElement.appendChild(this.canvas);
		
		this.ctx = this.canvas.getContext('2d');
		this.clear();
	},
	
	clear: function(){
		var ctx = this.ctx;
		ctx.clearRect(0, 0, this.width, this.height);
		ctx.fillStyle = this.background;
		ctx.fillRect(0, 0, this.width, this.height);
		this.axes();
	},
	
	draw: function(){
	// 	var ti = new Date().getTime();
	// 	this.init();
		this.clear();
		
		var txtFct = fct.remplacer(fct.verifier(document.getElementById("input3D").value));
		if(txtFct == ""){
			return;
		}
		
		var func = new CartesianFunction(txtFct);
		
		if(document.getElementById("selectAffichage3D").value == "points"){
			var coordX, coordY, coordZ;
			for(var x=this.left; x<this.right; x+=this.functionPrecision){
				for(var y=this.left; y<this.right; y+=this.functionPrecision){
					coordX = x;
					coordY = y;
					coordZ = func.f(x,y);
					if(isNaN(coordZ)){
						continue;
					}
					this.point3D(coordX, coordY, coordZ);
				}
			}
		}
		else{
			var x1, y1, z1, x2, y2, z2, x3, y3, z3;
			var x, y;
			var valAngle = Math.round(this.angle/Math.PI);
			if(valAngle%2==0){
				for(var x1=this.left; x1<this.right; x1+=this.functionPrecision){
					for(var y1=this.left; y1<this.right; y1+=this.functionPrecision){
						x = x1;
						y = y1;
						z1 = func.f(x,y);
						x2 = x1 + this.functionPrecision;
						y2 = y1;
						x = x2;
						//y = y2;
						z2 = func.f(x,y);
						x3 = x2;
						y3 = y2 + this.functionPrecision;
						//x = x3
						y = y3;
						z3 = func.f(x,y);
						x4 = x3 - this.functionPrecision;
						y4 = y3;
						x = x4;
						//y = y4;
						z4 = func.f(x,y);
						if(isNaN(z1)||isNaN(z2)||isNaN(z3)||isNaN(z4)){
							continue;
						}
						this.polygone3D(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
					}
				}
			}
			else{
				for(var x1=this.right; x1>this.left; x1-=this.functionPrecision){
					for(var y1=this.right; y1>this.left; y1-=this.functionPrecision){
						x = x1;
						y = y1;
						z1 = func.f(x,y);
						x2 = x1 - this.functionPrecision;
						y2 = y1;
						x = x2;
						//y = y2;
						z2 = func.f(x,y);
						x3 = x2;
						y3 = y2 - this.functionPrecision;
						//x = x3;
						y = y3;
						z3 = func.f(x,y);
						x4 = x3 + this.functionPrecision;
						y4 = y3;
						x = x4;
						//y = y4;
						z4 = func.f(x,y);
						if(isNaN(z1)||isNaN(z2)||isNaN(z3)||isNaN(z4)){
							continue;
						}
						this.polygone3D(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4);
					}
				}
			}
		}
	// 	var tf = new Date().getTime();
	// 	window.console.log(tf-ti);
	},
	
	// Dessine un point à la position (x, y, z)
	point3D: function(x, y, z){ 
		var posX = (Math.sin(this.angle)*x + Math.cos(this.angle)*y)*this.scale;
		var posZ = -(z - Math.cos(this.angle)*x/2.6 + Math.sin(this.angle)*y/2.6)*this.scale;

		var opacity = Math.round((1-((5+y*Math.sin(this.angle)-x*Math.cos(this.angle)) / 450)*this.scale)*1000)/1000;
		var couleur = new Array();
		couleur[0] = Math.round((5+z)*this.scale);
		couleur[1] = Math.round(510 - (5+z)*this.scale);
		couleur[2] = this.couleurGenerale;
		if(opacity > 1){ opacity = 1; }
		if(opacity < 0){ opacity = 0; }
		if(couleur[0] > 255){ couleur[0] = 255; }
		if(couleur[0] < 0){ couleur[0] = 0; }
		if(couleur[1] > 255){ couleur[1] = 255; }
		if(couleur[1] < 0){ couleur[1] = 0; }

		var ctx = this.ctx;
		ctx.save();
		ctx.translate(this.centerX, this.centerY);
		ctx.scale(this.zoomValue, this.zoomValue);
		ctx.fillStyle = "rgba("+couleur[this.rouge3D]+","+couleur[this.vert3D]+", "+couleur[this.bleu3D]+", "+opacity+")";
		ctx.fillRect(posX-1, posZ-1, 2, 2);
		ctx.restore();
	},
	
	// Dessine un polygone qui a comme sommets : (x1, y1, z1) , (x2, y2, z2), (x3, y3, z3) et (x4, y4, z4)
	polygone3D: function(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4){
		var ctx = this.ctx;
		ctx.save();
		ctx.translate(this.centerX, this.centerY);
		ctx.scale(this.zoomValue, this.zoomValue);
		ctx.beginPath();
		ctx.moveTo((Math.sin(this.angle)*x1 + Math.cos(this.angle)*y1)*this.scale , -(z1 - Math.cos(this.angle)*x1/2.6 + Math.sin(this.angle)*y1/2.6)*this.scale);
		ctx.lineTo((Math.sin(this.angle)*x2 + Math.cos(this.angle)*y2)*this.scale , -(z2 - Math.cos(this.angle)*x2/2.6 + Math.sin(this.angle)*y2/2.6)*this.scale);
		ctx.lineTo((Math.sin(this.angle)*x3 + Math.cos(this.angle)*y3)*this.scale , -(z3 - Math.cos(this.angle)*x3/2.6 + Math.sin(this.angle)*y3/2.6)*this.scale);
		ctx.lineTo((Math.sin(this.angle)*x4 + Math.cos(this.angle)*y4)*this.scale , -(z4 - Math.cos(this.angle)*x4/2.6 + Math.sin(this.angle)*y4/2.6)*this.scale);

		var opacity = Math.round((1-((5+y1*Math.sin(this.angle)-x1*Math.cos(this.angle)) / 450)*this.scale)*1000)/1000;
		var couleur = new Array();
		couleur[0] = Math.round((5+z1)*this.scale);
		couleur[1] = Math.round(510 - (5+z1)*this.scale);
		couleur[2] = this.couleurGenerale;
		if(opacity > 1){ opacity = 1; }
		if(opacity < 0){ opacity = 0; }
		
	// 	for(var i=0; i<couleur.length; i++){
	// 		couleur[i] -= parseInt((opacity*255));
	// 	}
		
		if(couleur[0] > 255){ couleur[0] = 255; }
		if(couleur[0] < 0){ couleur[0] = 0; }
		if(couleur[1] > 255){ couleur[1] = 255; }
		if(couleur[1] < 0){ couleur[1] = 0; }

		ctx.fillStyle = "rgba("+couleur[this.rouge3D]+","+couleur[this.vert3D]+", "+couleur[this.bleu3D]+", "+opacity+")";
		ctx.strokeStyle = "rgba(0,0,0,0.1)";
		ctx.closePath();
		ctx.fill();
		ctx.stroke();
		ctx.restore();
	},

	// Dessine les axes
	axes: function(){
		for(var i=-5; i<5; i+=this.linePrecision){
			this.point3D(0, 0, i);
		}
		for(var i=-5.5; i<5.5; i+=this.linePrecision){
			this.point3D(i, 0, 0);
		}
		for(var i=-5.5; i<5.5; i+=this.linePrecision){
			this.point3D(0, i, 0);
		}
	},

	cube: function(x, y, z, r){
		// Face de devant
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+i, y, z);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r, y+i, z);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r-i, y+r, z);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x, y+r-i, z);
		}
		// Face de derrière
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+i, y, z+r);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r, y+i, z+r);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r-i, y+r, z+r);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x, y+r-i, z+r);
		}
		// Arrêtes
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x, y, z+i);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x, y+r, z+i);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r, y, z+i);
		}
		for(var i=0; i<r; i+=this.linePrecision){
			this.point3D(x+r, y+r, z+i);
		}
	},
	
	// Change les couleurs de l'affichage 3D et affiche l'aperçu de celles-ci.
	checkCouleurs3D: function(){
		var rouge, vert, bleu;
		var sensRouge, sensVert, sensBleu;
		var precisionApercu = 64;
		this.couleurGenerale = parseInt(document.getElementById("couleur3Dgenerale").value);
		if(!this.couleurGenerale){
			this.couleurGenerale = 0;
		}
		if(this.couleurGenerale < 0){
			this.couleurGenerale = 0;
		}
		if(this.couleurGenerale > 255){
			this.couleurGenerale = 255;
		}
		switch(document.getElementById("selectRouge3D").value){
			case "plus":
				this.rouge3D = 0;
				rouge = 255;
				sensRouge = -1;
				break;
			case "moins":
				this.rouge3D = 1;
				rouge = 0;
				sensRouge = 1;
				break;
			case "tout":
				this.rouge3D = 2;
				rouge = this.couleurGenerale;
				sensRouge = 0;
				break;
		}
		switch(document.getElementById("selectVert3D").value){
			case "plus":
				this.vert3D = 0;
				vert = 255;
				sensVert = -1;
				break;
			case "moins":
				this.vert3D = 1;
				vert = 0;
				sensVert = 1;
				break;
			case "tout":
				this.vert3D = 2;
				vert = this.couleurGenerale;
				sensVert = 0;
				break;
		}
		switch(document.getElementById("selectBleu3D").value){
			case "plus":
				this.bleu3D = 0;
				bleu = 255;
				sensBleu = -1;
				break;
			case "moins":
				this.bleu3D = 1;
				bleu = 0;
				sensBleu = 1;
				break;
			case "tout":
				this.bleu3D = 2;
				bleu = this.couleurGenerale;
				sensBleu = 0;
				break;
		}
		document.getElementById("apercuCouleur3D").innerHTML = "<span style='background-color:rgba("+rouge+","+vert+","+bleu+",1);color:rgba(0,0,0,0);'>.</span>";
		for(var i=0; i<precisionApercu; i++){
			rouge = rouge + sensRouge*(256/precisionApercu);
			vert = vert + sensVert*(256/precisionApercu);
			bleu = bleu + sensBleu*(256/precisionApercu);
			document.getElementById("apercuCouleur3D").innerHTML = document.getElementById("apercuCouleur3D").innerHTML + "<span style='background-color:rgba("+rouge+","+vert+","+bleu+",1);color:rgba(0,0,0,0);'>.</span>";
		}
		if(this.ctx){
			this.draw();
		}
	},
	
	zoom: function(value){
		if(!fonction3D){
			return;
		}
		this.zoomValue *= value;
		this.left /= value;
		this.right /= value;
		this.functionPrecision /= value;
		this.linePrecision /= value;
		//alert(this.zoomValue+" ; "+this.left+" ; "+this.right+" ; "+this.functionPrecision+" ; "+this.linePrecision);
		this.draw();
	},
	
	move: function(value){
		this.angle += Math.PI/32 * value;
		this.draw();
	},
	
	initZoom: function(){
		this.angle = Math.PI/8;
		this.zoomValue = 1;
		this.left = -6.5;
		this.right = 6.5;
		this.draw();
	}
};

