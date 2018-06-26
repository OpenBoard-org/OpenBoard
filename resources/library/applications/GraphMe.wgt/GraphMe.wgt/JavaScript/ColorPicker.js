
// -------------------- Color Picker --------------------

var idColor = "";

// Différentes fonctions nécessaire au Color Picker (menu du choix de la couleur)

var colorPicker = {
	// Configuration
	idSV : "canvasSV",	// id du canvas affichant la saturation et la valeur
	idT : "canvasT",	// id du canvas affichant la teinte
	idO : "canvasO",	// id du canvas affichant l'opacité
	width : 250,		// largeur
	height : 250,		// hauteur
	rayonRonds : 5,		// rayon des ronds
	ombreActive : "0px 0px 3px rgba(150,200,255,1), 0px 0px 8px rgba(64,190,255,1)",
	ombreInactive : "0px 0px 5px rgba(64,64,64,0.4)",

	// Variables définies lors de l'initialisation
	canvasSV : null,
	canvasT : null,
	canvasO : null,
	ctxSV : null,
	ctxT : null,
	ctxO : null,
	lingradS : null,
	lingradV : null,
	lingradT : null,
	
	// Couleurs
	backgroundColor : "rgb(193,255,0)",
	saturation : 100,
	valeur : 100,
	teinte : 75,
	rouge : 193,
	vert : 255,
	bleu : 0,
	opacity: 1,
	// Couleurs RGB sans application de la saturation et de la valeur
	r : 255,
	g : 0,
	b : 0,
	
	// Autres variables
	sourisDown : false,	// Indique si on clique sur le colorPicker ou pas
	sourisDehors : true, // Indique si la souris est en-dehors du colorPicker lors d'un clique
	idColor : null,

	// Fonction d'initialisation
	init : function(id){
		// Récupérer les éléments
		this.canvasSV = document.getElementById(this.idSV);
		this.canvasT = document.getElementById(this.idT);
		this.canvasO = document.getElementById(this.idO);
		
		// Définir la taille
		this.canvasSV.width = this.width;
		this.canvasSV.height = this.height;
		this.canvasSV.style.width = this.width + "px";
		this.canvasSV.style.height = this.height + "px";
		
		this.canvasT.width = this.width/10;
		this.canvasT.height = this.height;
		this.canvasT.style.width = this.width/10 + "px";
		this.canvasT.style.height = this.height + "px";
		
		this.canvasO.width = this.width*1.15;
		this.canvasO.height = this.height/10;
		this.canvasO.style.width = this.width*1.15 + "px";
		this.canvasO.style.height = this.height/10 + "px";
		
		// Initialisation canvas
		this.ctxSV = this.canvasSV.getContext("2d");
		this.ctxT = this.canvasT.getContext("2d");
		this.ctxO = this.canvasO.getContext("2d");
		
		// Création des dégradés
		this.lingradV = this.ctxSV.createLinearGradient(0, 0, 0, this.height);
		this.lingradV.addColorStop(0, 'rgba(255,255,255,0)');
		this.lingradV.addColorStop(1, 'rgba(255,255,255,1)');
		
		this.lingradS = this.ctxSV.createLinearGradient(0, 0, this.width, 0);
		this.lingradS.addColorStop(0, 'rgba(0,0,0,1)');
		this.lingradS.addColorStop(1, 'rgba(0,0,0,0)');
		
		this.lingradT = this.ctxT.createLinearGradient(0, 0, 0, this.height);
		this.lingradT.addColorStop(0, 'rgb(255,0,0)');
		this.lingradT.addColorStop(1/6, 'rgb(255,255,0)');
		this.lingradT.addColorStop(2/6, 'rgb(0,255,0)');
		this.lingradT.addColorStop(3/6, 'rgb(0,255,255)');
		this.lingradT.addColorStop(4/6, 'rgb(0,0,255)');
		this.lingradT.addColorStop(5/6, 'rgb(255,0,255)');
		this.lingradT.addColorStop(1, 'rgb(255,0,0)');
		
		// Événements roulette (initialisé seulement une fois)
		if(window.addEventListener && !this.idColor){
			this.canvasSV.addEventListener('DOMMouseScroll', function(event){colorPicker.eventWheel(event, colorPicker.idSV)}, false);
			this.canvasSV.onmousewheel =  function(event){colorPicker.eventWheel(event, colorPicker.idSV)};
			this.canvasT.addEventListener('DOMMouseScroll',  function(event){colorPicker.eventWheel(event, colorPicker.idT)}, false);
			this.canvasT.onmousewheel =  function(event){colorPicker.eventWheel(event, colorPicker.idT)};
			this.canvasO.addEventListener('DOMMouseScroll',  function(event){colorPicker.eventWheel(event, colorPicker.idO)}, false);
			this.canvasO.onmousewheel =  function(event){colorPicker.eventWheel(event, colorPicker.idO)};
		}
		
		// Définir la couleur
		this.idColor = id;
		var couleur = eval(document.getElementById(this.idColor).title);
// 		alert(id+" ; "+couleur)
		document.getElementById("apercuCouleur").style.backgroundColor = couleur;
		document.getElementById("apercuCouleur2").style.backgroundColor = couleur;
		this.definirCouleur(couleur);
		
		// Dessiner
		this.dessiner();
		this.dessinerApercu();
		
		// Définir les valeurs des inputs
		this.definirInputs();
		
		// Définir le style de l'ombre
		this.sourisOut();
		
	},
	
	definirCouleur : function(colorRGB){
		var table = /(.*?)rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.exec(colorRGB);
		if (table == null){
			table = /(.*?)rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.exec(colorRGB);
		}
		if(table == null){
			return "";
		}
		this.rouge = parseInt(table[2]);
		this.vert = parseInt(table[3]);
		this.bleu = parseInt(table[4]);
		this.RGB_SVT();
	},
	
	sourisClick : function(id){
		this.sourisDown = id;
		if(id != this.idO){
			document.getElementById(id).style.boxShadow = this.ombreActive;
		}
	},
	
	sourisOut : function(){
		this.canvasSV.style.boxShadow = this.ombreInactive;
		this.canvasT.style.boxShadow = this.ombreInactive;
	},
	
	sourisOver : function(id){
		if(this.sourisDown == id && id != this.idO){
			document.getElementById(id).style.boxShadow = this.ombreActive;
		}
	},
	
	sourisUp : function(){
		this.sourisDown = false;
		this.canvasSV.style.boxShadow = this.ombreInactive;
		this.canvasT.style.boxShadow = this.ombreInactive;
		this.dessinerO();
	},
	
	eventWheel : function(event, id){
		if(!event) event = window.event;
		if(event.wheelDelta){
			if(event.wheelDelta < 0){
				colorPicker.sourisWheelUp(id);
			}
			else{
				colorPicker.sourisWheelDown(id);
			}
		}
		else if(event.detail){
			if(event.detail > 0){
				colorPicker.sourisWheelUp(id);
			}
			else{
				colorPicker.sourisWheelDown(id);
			}
		}
	},
	
	sourisWheelUp : function(id){
		switch(id){
			case this.idT:
				colorPicker.ajouterT(5);
				break;
			case this.idO:
				colorPicker.ajouterO(0.1);
				break;
			case this.idSV:
				colorPicker.ajouterS(-5);
				break;
		}
	},
	
	sourisWheelDown : function(id){
		switch(id){
			case this.idT:
				colorPicker.ajouterT(-5);
				break;
			case this.idO:
				colorPicker.ajouterO(-0.1);
				break;
			case this.idSV:
				colorPicker.ajouterS(5);
				break;
		}
	},
	
	ajouterT : function(nbr){
		this.teinte += nbr;
		if(this.teinte < 0){
			this.teinte = 0;
		}
		else if(this.teinte > 360){
			this.teinte = 360;
		}
		this.SVT_RGB();
	},
	
	ajouterO : function(nbr){
		this.opacity = Math.round((this.opacity+nbr)*100)/100;
		if(this.opacity < 0){
			this.opacity = 0;
		}
		else if(this.opacity > 1){
			this.opacity = 1;
		}
		this.SVT_RGB();
	},
	
	ajouterS : function(nbr){
		this.saturation += nbr;
		if(this.saturation < 0){
			this.saturation = 0;
		}
		else if(this.saturation > 100){
			this.saturation = 100;
		}
		this.SVT_RGB();
	},
	
	// Lors du déplacement de la souris
	moveSV : function(event){
		// Vérifie si on appuie sur la souris
		if(this.sourisDown != this.idSV){
			return 0;
		}
		
		var element = this.canvasSV;
		var posDivY = 0;
		var posDivX = 0;
		
		// Récupérer la position du canvas par rapport à la page
		while(element){
			posDivY = posDivY + element.offsetTop;
			posDivX = posDivX + element.offsetLeft;
			element = element.offsetParent;
		}
		
		// Définir la saturation et la valeur à partir de la position de la souris
		this.saturation = 100-Math.round((event.clientY - posDivY -1)/(this.height+1)*100);
		this.valeur = Math.round((event.clientX - posDivX -1)/(this.width+1)*100);
		
		this.SVT_RGB();
	},
	moveT : function(event){
		if(this.sourisDown != this.idT){
			return 0;
		}
		var element = this.canvasT;
		var posDivY = 0;
		
		while(element){
			posDivY = posDivY + element.offsetTop;
			element = element.offsetParent;
		}
		
		this.teinte = Math.round( (event.clientY - posDivY -0) / (this.height+1)*360);
		
		this.SVT_RGB();
	},
	
	SVT_RGB : function(){
		this.T_rgb();
		this.rouge = Math.round((this.r + (255-this.r) * (-1) * (this.saturation-100) / 100 )* this.valeur / 100);
		this.vert = Math.round((this.g + (255-this.g) * (-1) * (this.saturation-100) / 100 )* this.valeur / 100);
		this.bleu = Math.round((this.b + (255-this.b) * (-1) * (this.saturation-100) / 100 )* this.valeur / 100);
		
		this.definirInputs();
		this.dessinerApercu();
		this.dessiner();
	},
	
	T_rgb : function(){
		var r,g,b = 0;
		var T = this.teinte;
		
		if (T<60){
			r = 255;
			g = T/60*255;
			b = 0;
		}
		else if (T<120){
			r = (255-(T%60/60*255))%256;
			g = 255;
			b = 0;
		}
		else if (T<180){
			r = 0;
			g = 255;
			b = T%60/60*255;
		}
		else if (T<240){
			r = 0;
			g = (255-(T%60/60*255))%256;
			b = 255;
		}
		else if (T<300){
			r = T%60/60*255;
			g = 0;
			b = 255;
		}
		else if (T<360){
			r = 255;
			g = 0;
			b = (255-(T%60/60*255))%256;
		}
		else{
			r = 255;
			g = 0;
			b = 0;
		}
		
		this.r = Math.round(r);
		this.g = Math.round(g);
		this.b = Math.round(b);
	},
	
	RGB_SVT : function(){
		// Voir http://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur#Conversion_de_RVB_vers_TSV
		var r = this.rouge/255;
		var g = this.vert/255;
		var b = this.bleu/255;
		if(!isFinite(r)){
			r = 0;
		}
		if(!isFinite(g)){
			g = 0;
		}
		if(!isFinite(b)){
			b = 0;
		}
		var max = Math.max(r,g,b);
		var min = Math.min(r,g,b);
		var s, v, t;
		// Teinte
		switch(max){
			case r:
				t = (60 * (g-b)/(max-min) + 360) % 360;
				break;
			case g:
				t = 60 * (b-r)/(max-min) + 120;
				break;
			case b:
				t = 60 * (r-g)/(max-min) + 240;
				break;
			default: /* case min: */
				t = 0;
				break;
		}
		
		// Saturation
		if(max == 0){
			s = 0;
		}
		else{
			s = 1-(min/max);
		}
		
		// Valeur
		v = max;
		
		// Définir les variables
		this.saturation = s*100;
		this.valeur = v*100;
		this.teinte = Math.round(t);
		this.T_rgb();
	},
	
	dessiner : function(){
		var ctxSV = this.ctxSV;
		var ctxT = this.ctxT;
		// Fond
		ctxSV.fillStyle = "rgb("+this.r+","+this.g+","+this.b+")";
		ctxSV.fillRect(0, 0, this.width, this.height);
		// Dégradés
		ctxSV.fillStyle = this.lingradV;
		ctxSV.fillRect(0, 0, this.width, this.height);
		ctxSV.fillStyle = this.lingradS;
		ctxSV.fillRect(0, 0, this.width, this.height);
		// Souris
		var x = Math.round(this.width*this.valeur/100);
		var y = Math.round(this.height-this.height*this.saturation/100);
		ctxSV.beginPath();
		ctxSV.arc(x, y, this.rayonRonds, 0, 2*Math.PI, true);
		ctxSV.strokeStyle = "rgba(255,255,255,0.8)";
		ctxSV.shadowOffsetX = 1;
		ctxSV.shadowOffsetY = 1;
		ctxSV.shadowColor = "rgba(0,0,0,1)";
		ctxSV.shadowBlur = 2;
		ctxSV.lineWidth = 1.5;
		ctxSV.stroke();
		
		// Fond Teinte
		ctxT.fillStyle = this.lingradT;
		ctxT.fillRect(0, 0, this.width/10, this.height);
		// Souris Teinte
		var pos = Math.round(0.99*this.height*this.teinte/360);
		ctxT.fillStyle = "rgba(255,255,255,0.8)";
		ctxT.shadowOffsetX = 0;
		ctxT.shadowOffsetY = 0;
		ctxT.shadowColor = "rgba(0,0,0,1)";
		ctxT.shadowBlur = 3;
		ctxT.fillRect(0, pos, this.width/10, 2);
		
		// Désactiver les ombres
		ctxSV.shadowColor = "rgba(0,0,0,0)";
		ctxT.shadowColor = "rgba(0,0,0,0)";
		
		this.dessinerO();
	},
	
	dessinerApercu : function(){
		document.getElementById("apercuCouleur").style.backgroundColor = "rgba("+this.rouge+","+this.vert+","+this.bleu+","+this.opacity+")";
	},
	
	definirInputs : function(){
		document.getElementById("inputValeur").value = this.valeur;
		document.getElementById("inputSaturation").value = this.saturation;
		document.getElementById("inputTeinte").value = this.teinte;
		document.getElementById("inputRouge").value = this.rouge;
		document.getElementById("inputVert").value = this.vert;
		document.getElementById("inputBleu").value = this.bleu;
		document.getElementById("inputOpacity").value = this.opacity;
	},
	
	recupererInputs : function(){
		this.valeur = parseInt(document.getElementById("inputValeur").value);
		this.saturation = parseInt(document.getElementById("inputSaturation").value);
		this.teinte = parseInt(document.getElementById("inputTeinte").value);
		this.opacity = parseFloat(document.getElementById("inputOpacity").value);
		this.SVT_RGB();
		
		this.definirInputs();
		this.dessinerApercu();
		this.dessiner();
	},
	recupererInputs2 : function(){
		this.rouge = parseInt(document.getElementById("inputRouge").value);
		this.vert = parseInt(document.getElementById("inputVert").value);
		this.bleu = parseInt(document.getElementById("inputBleu").value);
		this.RGB_SVT();
		
		this.definirInputs();
		this.dessinerApercu();
		this.dessiner();
	},
	
	moveO : function(event){
		if(this.sourisDown != this.idO){
			return 0;
		}
		
		var element = this.canvasO;
		var posDivX = 0;
		while(element){
			posDivX = posDivX + element.offsetLeft;
			element = element.offsetParent;
		}
		
		this.opacity = Math.round( (event.clientX - posDivX -this.width/10) / (this.width*0.9)*100)/100;
		if(this.opacity < 0){
			this.opacity = 0;
		}
		else if(this.opacity > 1){
			this.opacity = 1;
		}
		this.dessinerO();
		this.definirInputs();
		this.dessinerApercu();
	},
	
	dessinerO : function(){
		var ctx = this.ctxO;
		
		ctx.shadowBlur = 2;
		
		// Ligne
		if(this.sourisDown == this.idO){
			ctx.shadowColor = "rgba(0,50,100,1)";
		}
		else{
			ctx.shadowColor = "rgba(0,0,0,1)";
		}
		ctx.globalAlpha = 1;
		ctx.shadowOffsetX = 0;
		ctx.shadowOffsetY = 0;
		ctx.fillStyle = "rgba("+this.rouge+","+this.vert+","+this.bleu+",1)";
		ctx.clearRect(0,0,this.width*1.15,this.height);
		ctx.fillRect(this.width/10, this.height/20-1, this.width*0.9, 3);
		
		// Rond
		var x = this.width/10 + this.width*0.9*this.opacity-1;
		var y = this.height/20;
		if(this.sourisDown == this.idO){
			ctx.fillStyle = "rgba(0,40,80,1)";
			ctx.strokeStyle = "rgba(230,250,255,0.8)";
		}
		else{
			ctx.fillStyle = "rgba(0,0,0,1)";
			ctx.strokeStyle = "rgba(255,255,255,0.8)";
		}
		ctx.globalAlpha = 0.1+this.opacity*0.9;
		ctx.beginPath();
		ctx.arc(x, y, this.rayonRonds+1, 0, 2*Math.PI, true);
		ctx.shadowColor = "rgba(0,0,0,1)";
		ctx.shadowOffsetX = 1;
		ctx.shadowOffsetY = 1;
		ctx.lineWidth = 1;
		ctx.fill();
		ctx.stroke();
		
		// Petits ronds
		ctx.globalAlpha = 1;
		ctx.shadowColor = "rgba(0,0,0,1)";
		
		ctx.beginPath();
		ctx.arc(8, this.height/20+0.5, this.rayonRonds, 0, 2*Math.PI, true);
		ctx.strokeStyle = "rgba(0,0,0,0.5)";
		ctx.shadowOffsetX = 0;
		ctx.shadowOffsetY = 0;
		ctx.lineWidth = 1;
		ctx.stroke();
		
		ctx.beginPath();
		ctx.arc(this.width*1.08, this.height/20+0.5, this.rayonRonds, 0, 2*Math.PI, true);
		ctx.strokeStyle = "rgba(0,0,0,0.8)";
		ctx.fillStyle = "rgba(0,0,0,0.6)";
		ctx.shadowOffsetX = 0;
		ctx.shadowOffsetY = 0;
		ctx.lineWidth = 1;
		ctx.stroke();
		ctx.fill();
	},
	
	exporterCouleur : function(){
		eval(document.getElementById(this.idColor).title+' = "rgba(" + this.rouge +", " + this.vert + ", " + this.bleu +", "+this.opacity+")"');
		document.getElementById(this.idColor).style.backgroundColor = eval(document.getElementById(this.idColor).title);
	},
	
	fermer : function(){
		colorPicker.exporterCouleur();
		if(colorPicker.idColor == 'buttonColor'){
			cacherMenu();
		}
		else if(dernierMenu == "menuFonctions"){
			editeur.getOptions();
			afficherMenu(dernierMenu);
		}
		else{
			cacherMenu();
		}
		actualiserGraph();
	}
};


