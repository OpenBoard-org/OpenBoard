 
var editeur = {
	idFct : 0,
	fct : {
		fct : "",
		fctX : "",
		fctY : "",
		from : 0,
		to : 2*Math.PI,
		couleur : "",
		couleurD1 : "",
		couleurD2 : "",
		couleurP1 : "",
		width : 2,
		style : "continu",
		aire : false,
		derivee1 : false,
		derivee2 : false,
		primitive1 : false,
		conditionInitiale : 0
	},
	editer : function(id){
		this.idFct = id;
		if(fct.list[this.idFct]){
			document.getElementById("editeurFonction").style.visibility = "visible";
			this.setOptions();
		}
		else{
			document.getElementById("editeurFonction").style.visibility = "hidden";
		}
		fct.updateList();
	},
	couleur : function(id){
		colorPicker.init(id);
		afficherMenu('menuCouleur');
	},
	getOptions : function(){
		this.fct.fct = document.getElementById("editeurInput").value;
		this.fct.fctX = document.getElementById("editeurFctX").value;
		this.fct.fctY = document.getElementById("editeurFctY").value;
		this.fct.from = document.getElementById("editeurFrom").value;
		this.fct.to = document.getElementById("editeurTo").value;
		this.fct.width = document.getElementById("editeurWidth").value;
		this.fct.style = document.getElementById("editeurStyle").value;
		this.fct.aire = document.getElementById("editeurAire").checked ? true : false;
		this.fct.derivee1 = document.getElementById("editeurD1").checked ? true : false;
		this.fct.derivee2 = document.getElementById("editeurD2").checked ? true : false;
		this.fct.primitive1 = document.getElementById("editeurP1").checked ? true : false;
		if(this.fct.primitive1){
			document.getElementById("editeurConditionInitiale").disabled = "";
			document.getElementById("editeurTexteConditionInitiale").style.color = "rgb(0,0,0)"
		}
		else{
			document.getElementById("editeurConditionInitiale").disabled = "disabled";
			document.getElementById("editeurTexteConditionInitiale").style.color = "rgb(128,128,128)"
		}
		this.fct.conditionInitiale = parseFloat(document.getElementById("editeurConditionInitiale").value);
		
		fct.list[this.idFct].set(this.fct);
		
		fct.updateList();
		historique.ajouter(fct.list[this.idFct]);
		this.apercu();
	},
	setOptions : function(){
		var f = fct.list[this.idFct].get();
		for(var i in f){
			this.fct[i] = f[i];
		}
		
		this.setDisplayStyle();
		
		document.getElementById("editeurInput").value = this.fct.fct;
		document.getElementById("editeurFctX").value = this.fct.fctX;
		document.getElementById("editeurFctY").value = this.fct.fctY;
		document.getElementById("editeurFrom").value = this.fct.from;
		document.getElementById("editeurTo").value = this.fct.to;
		document.getElementById("editeurCouleur").style.backgroundColor = this.fct.couleur;
		document.getElementById("editeurCouleurD1").style.backgroundColor = this.fct.couleurD1;
		document.getElementById("editeurCouleurD2").style.backgroundColor = this.fct.couleurD2;
		document.getElementById("editeurCouleurP1").style.backgroundColor = this.fct.couleurP1;
		document.getElementById("editeurWidth").value = this.fct.width;
		document.getElementById("editeurStyle").value = this.fct.style;
		document.getElementById("editeurAire").checked = this.fct.aire ? "checked" : "";
		document.getElementById("editeurD1").checked = this.fct.derivee1 ? "checked" : "";
		document.getElementById("editeurD2").checked = this.fct.derivee2 ? "checked" : "";
		document.getElementById("editeurP1").checked = this.fct.primitive1 ? "checked" : "";
		if(this.fct.primitive1){
			document.getElementById("editeurConditionInitiale").disabled = "";
			document.getElementById("editeurTexteConditionInitiale").style.color = "rgb(0,0,0)"
		}
		else{
			document.getElementById("editeurConditionInitiale").disabled = "disabled";
			document.getElementById("editeurTexteConditionInitiale").style.color = "rgb(128,128,128)"
		}
		document.getElementById("editeurConditionInitiale").value = this.fct.conditionInitiale;
		
		document.getElementById("outputX").innerHTML = " ";
		document.getElementById("outputAire").innerHTML = " ";
		this.apercu();
	},
	setDisplayStyle : function(){
		if(fct.list[this.idFct] instanceof CartesianFunction){
			document.getElementById("editeurFctPrefix").textContent = "f(x) = ";
			document.getElementById("editeurFctDiv").style.display = "block";
			document.getElementById("editeurFctXYDiv").style.display = "none";
			document.getElementById("editeurRangeDiv").style.display = "none";
			document.getElementById("editeurDeriveesDiv").style.display = "block";
			document.getElementById("editeurStyleDiv").style.display = "block";
			document.getElementById("editeurAireDiv").style.display = "block";
		}
		else if(fct.list[this.idFct] instanceof ImplicitFunction){
			document.getElementById("editeurFctPrefix").textContent = "";
			document.getElementById("editeurFctDiv").style.display = "block";
			document.getElementById("editeurFctXYDiv").style.display = "none";
			document.getElementById("editeurRangeDiv").style.display = "none";
			document.getElementById("editeurDeriveesDiv").style.display = "none";
			document.getElementById("editeurStyleDiv").style.display = "none";
			document.getElementById("editeurAireDiv").style.display = "none";
		}
		else if(fct.list[this.idFct] instanceof PolarFunction){
			document.getElementById("editeurFctPrefix").textContent = "r(t) = ";
			document.getElementById("editeurFctDiv").style.display = "block";
			document.getElementById("editeurFctXYDiv").style.display = "none";
			document.getElementById("editeurRangeDiv").style.display = "block";
			document.getElementById("editeurDeriveesDiv").style.display = "none";
			document.getElementById("editeurStyleDiv").style.display = "block";
			document.getElementById("editeurAireDiv").style.display = "none";
		}
		else if(fct.list[this.idFct] instanceof ParametricFunction){
			document.getElementById("editeurFctDiv").style.display = "none";
			document.getElementById("editeurFctXYDiv").style.display = "block";
			document.getElementById("editeurRangeDiv").style.display = "block";
			document.getElementById("editeurDeriveesDiv").style.display = "none";
			document.getElementById("editeurStyleDiv").style.display = "block";
			document.getElementById("editeurAireDiv").style.display = "none";
		}
	},
	etudier : function(){
		fct.etudier(this.idFct);
	},
	supprimer : function(){
		fct.enlever(this.idFct);
		if(!fct.list[this.idFct]){
			this.editer(this.idFct - 1);
		}
		else{
			this.editer(this.idFct);
		}
	},
	dupliquer : function(){
		fct.dupliquer(this.idFct);
		this.editer(this.idFct + 1);
	},
	apercu : function(){
		editorPreview.drawFunction(fct.list[this.idFct]);
	}
};


var editorPreview = {
	id : "editeurApercu",
	
	xGauche : -5,
	xDroite : 5,
	yBas : -4,
	yHaut : 4,
	
	largeur : 0,
	hauteur : 0,
	multX : 1,
	multY : 1,
	
	ctx: null,
	
	init : function(){
		var canvas = document.getElementById("editeurApercu");
		this.ctx = canvas.getContext("2d");
		this.largeur = canvas.width;
		this.hauteur = canvas.height;
		this.multX = this.largeur/Math.abs(this.xDroite - this.xGauche);
		this.multY = this.hauteur/Math.abs(this.yHaut - this.yBas);
		this.precision = Math.abs(this.xDroite - this.xGauche)/50;
	},
	
	drawFunction : function(func){
		var x, y, pointX, pointY;
		var ctx = this.ctx;
		ctx.clearRect(0,0,100,80);
		ctx.globalAlpha = 0.7;
		ctx.fillStyle = "rgba(0,0,0,0.5)"
		ctx.fillRect(0,39.5, 100, 1);
		ctx.fillRect(49.5,0, 1, 80);
		ctx.beginPath();
// 		ctx.lineWidth = func.width/1.8;
// 		ctx.strokeStyle = func.couleur;
// 		ctx.moveTo(-100,-100);
// 		for(x = -5; x <= 5; x+=0.2){
// 			// Evaluer la valeur y du point x
// 			try{
// 				y = func.f(x);
// 			}
// 			catch(e){}
// 			if(!isNaN(y)){
// 				// Transformation des coordonnées
// 				pointX = x * 10 + 50;
// 				pointY = 40 - y * 10;
// 				
// 				// Dessiner la fonction
// 				ctx.lineTo(pointX, pointY);
// 			}
// 		}
		func.plot(ctx, this.precision, this);
// 		ctx.stroke();
	}
};
