
// Convertir une coordonnée sur le graphique en position relative à l'affichage (ou à l'écran si ecran = true)
function coordToPosX(x, ecran){
	if(ecran){
		return coordToPosX(x, false)+affichage.offsetLeft;
	}
	else{
		return (x - affichage.xGauche) * affichage.multX;
	}
}
function coordToPosY(y, ecran){
	if(ecran){
		return coordToPosY(y, false)+affichage.offsetTop;
	}
	else{
		return affichage.hauteur - (y - affichage.yBas) * affichage.multY;
	}
}

var affichage = {
	// Options
	id : "affichage",
	largeur : 640, // Set by init
	hauteur : 430, // Set by init
	couleurFond : "rgba(0,0,0,0)",
	
	// Bornes (zone d'affichage)
	xGauche : -5.5, // Set by init
	xDroite : 5.5, // Set by init
	yBas : -3.7,
	yHaut : 3.7,
	
	multX : 1,
	multY : 1,
	
	// Axes
	axes : true,
	couleurAxes : "rgba(0,0,0,0.5)",
	widthAxes : 2,
	// Grille
	grille : true,
	couleurGrille : "rgba(255,255,255,0.1)",
	widthGrille : 2,
	// Echelle
	echelle : true,
	couleurEchelle : "rgba(255,255,255,1)",
	
	// Précision
	precision : 100,
	precisionAmelioree : true, // false = fonction plus jolie lorsqu'on dé-zoom, true = affichage plus rapide lors du dé-zoom.
	
	// Méthode et style
	methode : "canvas",
	style : "continu",
	
	// Variable définies lors de l'initialisation
	canvas : null,
	ctx : null,
	object : null,
	offsetTop : null, // Position de l'affichage par rapport à la page
	offsetLeft : null,
	
	init : function(element, width, height){
		this.setBornes();
		
		// Supprimer le contenu de l'affichage
		if(!element){
			element = document.getElementById(this.id);
		}
		if(element.hasChildNodes()){
			while(element.childNodes.length >= 1 ){
				element.removeChild(element.firstChild);
			}
		}
		
		// Définir la taille
		this.largeur = width || element.clientWidth;
		this.hauteur = height || element.clientHeight;
		
		// Set left and right proportionally to width, height, top and bottom
		var centerX = this.xGauche + (this.xDroite - this.xGauche)/2;
		var centerY = this.yBas + (this.yHaut - this.yBas)/2;
		var dx = ((this.yHaut - this.yBas) / 2) * this.largeur / this.hauteur;
		this.xGauche = Math.round((centerX - dx)*100)/100;
		this.xDroite=  Math.round((centerX + dx)*100)/100;
		this.setBornes();
		
		// Récupérer la position
		var boundingClientRect = element.getBoundingClientRect();
		this.offsetTop = boundingClientRect.top;
		this.offsetLeft = boundingClientRect.left;

		// Sélectionner la méthode d'affichage
		if(this.methode == "svg"){
// 			this.object = document.createElement("embed");
// 			this.object.type = "image/svg+xml";
// 			this.object.src = "AffichageSVG.svg";
			this.object = document.getElementById("embedSVG");
			this.object.width = this.largeur;
			this.object.height = this.hauteur;
			this.object.style.top = this.offsetTop +1 +"px";
			this.object.style.left = this.offsetLeft +1 +"px";
			this.object.style.display = "block";
// 			element.appendChild(this.object);
			
// 			affichage.ctx = svg;
// 			affichage.dessiner();
// 			// Exécuter this.dessiner() maintenant ne va pas car svg n'est pas
// 			// encore défini dans cette fonction, il faut en lancer un nouvelle...
			setTimeout("affichage.ctx = svg", 50);
			setTimeout("affichage.dessiner()", 100);
		}
		else if(this.methode == "uniboard"){
			if(window.uniboard || window.sankore){
				try{
				initUniboard();
				this.ctx = uniboard;
				this.dessiner();
				}
				catch(err){
					alert(err.message);
				}
			}
			else{
				this.methode = "canvas";
				this.setOptions();
				this.init();
			}
		}
		else{
			document.getElementById("embedSVG").style.display = "none";
			
			this.canvas = document.createElement("canvas");
			this.canvas.width = this.largeur;
			this.canvas.height = this.hauteur;
			element.appendChild(this.canvas);
			
			this.ctx = this.canvas.getContext("2d");
			
			this.dessiner();
		}
		
		// Événements
		if(window.addEventListener){
			element = document.getElementById("eventAffichage");
			element.addEventListener('DOMMouseScroll', souris.wheel, false);
			element.onmousewheel = souris.wheel;
			element.oncontextmenu = ctxMenu.ouvrir;
		}
	},
	
	calculer : function(){
		this.getBornes();
		this.getOptions();
		if(fonction3D){
			display3D.draw()
		}
		else{
			this.dessiner();
		}
		saveOptions();
	},
	
	dessiner : function(){
		try{
// 			var ti = new Date().getTime();
			var precision;
			if(this.precisionAmelioree){
				precision = 10/this.precision;
			}
			else{
				precision = Math.abs(this.xDroite - this.xGauche)/this.precision;
			}
			this.multX = this.largeur/Math.abs(this.xDroite - this.xGauche);
			this.multY = this.hauteur/Math.abs(this.yHaut - this.yBas);
			
			var ctx = new Object();
			ctx = this.ctx;
			ctx.clearRect(0,0,this.largeur,this.hauteur);
			
			ctx.fillStyle = this.couleurFond;
			ctx.fillRect(0,0,this.largeur,this.hauteur);
			
			ctx.strokeOpacity = 1; // svg
			
			// Couleur pour l'aire sous la fonction
			ctx.fillStyle = "rgba(0,180,255,0.3)";
			
			// Fonctions
			for(var i=0; i<fct.list.length; i++){
				if(!fct.list[i]){
					continue;
				}
				fct.list[i].plot(ctx, precision, affichage);
			}
			
			ctx.strokeOpacity = 0.2; // svg
			
			// Grille et échelle
// 			var intervalX = Math.round(Math.abs(this.xGauche-this.xDroite)/10);
			var interval = Math.round(Math.abs(this.yBas-this.yHaut)/10);
			if(interval <= 0){
				interval = 1;
			}
// 			if(intervalY <= 0){
// 				intervalY = 1;
// 			}
			var initialX = Math.floor(-this.xGauche) % interval;
			var initialY = Math.floor(-this.yBas) % interval;
			ctx.beginPath();
			ctx.fillStyle = this.couleurEchelle;
			ctx.strokeStyle = this.couleurEchelle;
			ctx.lineWidth = this.widthGrille;
				if(this.methode == "uniboard"){
					ctx.lineWidth /= 2;
					ctx.fillStyle = "rgb(0,0,0)";
					ctx.strokeStyle = "rgb(100,100,100)";
				}
			for(var i=initialX; i<=Math.round(this.xDroite-this.xGauche); i=i+interval){
				var position = Math.round((Math.ceil(this.xGauche) - this.xGauche +i) * this.multX);
				if(this.grille){
					ctx.moveTo(position, 0);
					ctx.lineTo(position, this.hauteur);
				}

				if(this.echelle){
					ctx.fillText(Math.ceil(i+this.xGauche), position-6, (this.hauteur-(-this.yBas*this.multY))-2);
				}
			}
			for(var i=initialY;i<=Math.round(this.yHaut-this.yBas);i=i+interval){
				var position = this.hauteur - Math.round((Math.ceil(this.yBas) - this.yBas +i) * this.multY);
				if(this.grille){
					ctx.moveTo(0, position);
					ctx.lineTo(this.largeur, position);
				}

				if(this.echelle){
					ctx.fillText(Math.ceil(i+this.yBas), (-this.xGauche*this.multX)+2, position+6);
				}
			}
			ctx.strokeStyle = this.couleurGrille;
			ctx.stroke();
			
			ctx.strokeOpacity = 0.8; //svg
			
			// Axes
			if(this.axes){
				ctx.beginPath();
				ctx.strokeStyle = this.couleurAxes;
				ctx.lineWidth = this.widthAxes;
				if(this.methode == "uniboard"){
					ctx.lineWidth *= 2;
					ctx.strokeStyle = "rgb(0,0,0)";
				}
				ctx.moveTo(-this.xGauche*this.multX, 0)
				ctx.lineTo(-this.xGauche*this.multX, this.hauteur)

				ctx.moveTo(0, this.hauteur+this.yBas*this.multY)
				ctx.lineTo(this.largeur, this.hauteur+this.yBas*this.multY)
				ctx.stroke()
			}
			
			// Autres
			outil.dessinerListe();
// 			var tf = new Date().getTime();
// 			window.console.log(tf-ti);
		}
		catch(err){
			var message = err.message;
			afficherErreur(message);
		}
	},
	
	getBornes : function(){
		this.xGauche = parseFloat(document.getElementById("borneXGauche").value);
		this.xDroite = parseFloat(document.getElementById("borneXDroite").value);
		this.yBas = parseFloat(document.getElementById("borneYGauche").value);
		this.yHaut = parseFloat(document.getElementById("borneYDroite").value);
	},
	
	setBornes : function(){
		document.getElementById("borneXGauche").value = this.xGauche;
		document.getElementById("borneXDroite").value = this.xDroite;
		document.getElementById("borneYGauche").value = this.yBas;
		document.getElementById("borneYDroite").value = this.yHaut;
	},
	
	getOptions : function(){
		this.axes = document.getElementById("checkAxes").checked ? true : false;
		this.grille = document.getElementById("checkGrille").checked ? true : false;
		this.echelle = document.getElementById("checkEchelle").checked ? true : false;
		this.precision = document.getElementById("inputPrecision").value;
		this.precisionAmelioree = document.getElementById("checkPrecision").checked ? true : false;
		var methode = document.getElementById("selectMethodeAffichage").value;
		if(methode != this.methode){
			this.methode = methode;
			this.init();
		}
	},
	
	setOptions : function(){
		document.getElementById("selectMethodeAffichage").value = this.methode;
		document.getElementById("checkGrille").checked = this.grille;
		document.getElementById("checkAxes").checked = this.axes;
		document.getElementById("checkEchelle").checked = this.echelle;
		document.getElementById("inputPrecision").value = this.precision;
		document.getElementById("checkPrecision").checked = this.precisionAmelioree;
	},
	
	deplacerX : function(x){
		if(fonction3D){
			display3D.move(x);
			return;
		}
		this.xGauche += x;
		this.xDroite += x;
		this.dessiner();
		this.setBornes();
		saveOptions();
	},
	deplacerY : function(y){
		if(fonction3D){
			return;
		}
		this.yHaut += y;
		this.yBas += y;
		this.dessiner();
		this.setBornes();
		saveOptions();
	},
	centrer: function(){
		var valeurX = (this.xDroite-this.xGauche)/2;
		var valeurY = (this.yHaut-this.yBas)/2;
		this.initZoom(valeurX, valeurY);
	},
	initZoom : function(valeurX, valeurY){
		if(fonction3D){
			display3D.initZoom();
		}
		this.xGauche = -valeurX;
		this.xDroite = valeurX;
		this.yBas = -valeurY;
		this.yHaut = valeurY;
		this.setBornes();
		this.dessiner();
	},
	initZoom2 : function(valeur){
		var valeurY = parseFloat(valeur);
		var valeurX = valeurY * this.largeur / this.hauteur;
		this.initZoom(valeurX, valeurY);
	},
	zoom : function(facteur){
		if(fonction3D){
			return;
		}
		var diffBornes, ajouter;
		
		// Horizontal
		diffBornes = Math.abs(this.xDroite - this.xGauche);
		ajouter = Math.round(diffBornes * (facteur-1)*2)/4;
// 		log(diffBornes, ajouter, facteur)
		if(ajouter == 0){
			ajouter = 0.25;
// 			log("affichage.zoom -> x : ajouter = 0.5")
		}
		
		this.xGauche -= ajouter;
		this.xDroite += ajouter;
		
		// Vertical
		diffBornes = Math.abs(this.yHaut - this.yBas);
		ajouter = Math.round(diffBornes * (facteur-1)*2)/4;
		if(ajouter == 0){
			ajouter = 0.25;
// 			log("affichage.zoom -> y : ajouter = 0.5")
		}
		
		this.yBas -= ajouter;
		this.yHaut += ajouter;
		
		this.dessiner();
		this.setBornes();
		
		saveOptions();
	}
};
