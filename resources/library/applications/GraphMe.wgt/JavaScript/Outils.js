
// -------------------- Fonctions des outils --------------------


// Permet d'afficher la valeur en "y" pour un point donné en "x"
function calculerPoint() {
	x = document.getElementById("inputX").value;
	document.getElementById("outputX").innerHTML = " f(x) = "+fct.list[editeur.idFct].f(x)+"";
	//alert("Si x = "+x+" \nf(x) = "+eval(fonction)+"")
}


// Historique

var historique = {
	liste : [],
	
	actualiser : function(){
		var texteHistorique = ""
		for(var i=0; i<this.liste.length; i++){
			texteHistorique += '<span class="survol" onclick="historique.use('+i+'); actualiserGraph()">';
			texteHistorique += this.liste[i].readableText();
			texteHistorique += '</span> <br/>';
		}
		document.getElementById("spanHistorique").innerHTML = texteHistorique
		document.getElementById("divHistorique").scrollTop = 0
	},
	
	use : function(index){
		fct.add(this.liste[index]);
	},
	
	ajouter : function(func){
		if(func.fct == ""){
			return;
		}
		for(var i=0; i<this.liste.length; i++){
			if(this.liste[i].fct == func.fct){
				this.liste.splice(i, 1);
			}
		}
		var newFunction = fct.functionFromObject(func.get());
		this.liste.unshift(newFunction);
		this.actualiser();
		saveOptions();
	},
	
	get: function(){
		var list = [];
		for(var i=0; i<this.liste.length; i++){
			list.push(this.liste[i].get());
		}
		return list;
	},
	
	set: function(obj){
		for(var i=0; i<obj.length; i++){
			this.liste.push(fct.functionFromObject(obj[i]));
		}
		this.actualiser();
	}
}


// Outils de la souris

var outil = {
	//
	idAffichage : "affichageOutils",
	
	// Outil sélectionné
	actuel : "point",
	
	// Canvas
	canvas : null,
	ctx : null,
	
	width: 0,
	height: 0,
	
	// Liste des éléments à dessiner
	liste : [],
	
	// Fonction d'initialisation
	init : function(width, height){
		var element = document.getElementById(this.idAffichage);
		if(element.hasChildNodes()){
			while(element.childNodes.length >= 1 ){
				element.removeChild(element.firstChild);
			} 
		}
		
		this.width = width || element.clientWidth;
		this.height = height || element.clientHeight;
		
// 		element.style.width = affichage.largeur +"px";
// 		element.style.height = affichage.hauteur +"px";
// 		element.style.left = document.getElementById(affichage.id).offsetLeft+1 +"px";
// 		element.style.top = document.getElementById(affichage.id).offsetTop+1 +"px";
		
		this.canvas = document.createElement("canvas");
		this.canvas.width = this.width;
		this.canvas.height = this.height;
		element.appendChild(this.canvas);
		this.ctx = this.canvas.getContext('2d');
	},
	
	// Permet de changer d'outil et de faire différentes actions lors du choix de l'outil
	choisir : function(nom){
		document.getElementById("pointTool").className = "toolButton";
		document.getElementById("moveTool").className = "toolButton";
		document.getElementById("tangentTool").className = "toolButton";
		switch(nom){
			case 'point':
				document.getElementById("pointTool").className += " selectedTool";
				break;
			case 'deplacement':
				document.getElementById("moveTool").className += " selectedTool";
				break;
			case 'tangente':
				document.getElementById("tangentTool").className += " selectedTool";
				break;
		}
		if(nom == 'deplacement'){
			document.getElementById("eventAffichage").style.cursor = "move";
		}
		else{
			document.getElementById("eventAffichage").style.cursor = "auto";
		}
		this.dessinerListe();
		this.actuel = nom;
		saveOptions();
	},
	
	// Gestion des événements reçus de l'objet souris
	move : function(x, y, xInit, yInit, active){
		switch(this.actuel){
			case "deplacement":
				if(active){
					this.deplacement(x, y, xInit, yInit);
				}
				break;
			case "point":
				this.point(x, y);
				break;
			case "tangente":
				this.tangente(x, y);
				break;
		}
	},
	
	down : function(x, y){
		switch(this.actuel){
			case "point":
				this.point(x, y, true)
				break;
			case "tangente":
				this.tangente(x, y, true);
				break;
		}
	},
	
	// Fonctions des outils
	deplacement : function(x, y, xInit, yInit){
		var valeurX = (x-xInit)/affichage.multX;
		var valeurY = (yInit-y)/affichage.multY;
		if(Math.round(Math.abs(valeurX)) > 0){
			if(fonction3D){
				affichage.deplacerX(Math.round(2*valeurX)/2);
			}
			else{
				affichage.deplacerX(-Math.round(2*valeurX)/2);
			}
			souris.xInit = x;
		}
		if(Math.round(Math.abs(valeurY)) > 0){
			affichage.deplacerY(-Math.round(2*valeurY)/2);
			souris.yInit = y;
		}
	},
	
	point : function(sourisX, sourisY, ajouterDansListe){
		var ctx = new Object();
		ctx = this.ctx;
		
		// Dessiner les points et tangentes supplémentaires
		this.dessinerListe();
		
		for(var i=0; i<fct.list.length; i++){
			var func = fct.list[i];
			if(!func || func == ""){
				continue;
			}
			
			// Position de la souris par rapport au coin supérieur gauche de l'affichage
			var posX = sourisX - affichage.offsetLeft - 2;
			
			// Valeur de la coordonnée x et y sous la souris
			var t = posX / affichage.multX + affichage.xGauche;
			if("startAngle" in func){
				if(t < func.startAngle || t > func.endAngle){
					continue;
				}
			}
			var x = func.getX(t);
			var y = func.getY(t);
			
			// Afficher les coordonnées et le point sous la souris
			if(!isNaN(x) && !isNaN(y)){
				if(ajouterDansListe){
					this.ajouterPoint(x, y, func.couleur);
				}
				this.dessinerPoint(x, y, func.couleur);
			}
		}
	},
	dessinerPoint : function(x, y, couleur){
		var ctx = new Object();
		ctx = this.ctx;
		
		// Conversion des coordonnées
		var posX = coordToPosX(x) + 1;
		var posY = coordToPosY(y) + 1;
			
		ctx.shadowColor = couleur;
		ctx.shadowBlur = 4;
		
		// Texte
		var txtPos = "("+Math.round(x*100)/100+";"+Math.round(y*100)/100+")";
		ctx.fillStyle = "rgba(255,255,255,0.5)";
		try{
			ctx.bulle(posX+7, posY-7, txtPos.length*5.5, 20);
		}
		catch(err){
			ctx.fillRect(posX+7, posY-27, txtPos.length*5.5, 20);
		}
		
		ctx.fillStyle = "black";
		ctx.shadowColor = "black";
		ctx.shadowOffsetX = 1;
		ctx.shadowOffsetY = 1;
		ctx.shadowBlur = 2;
		ctx.fillText(txtPos, posX+8+txtPos.length/4, posY-13);
		
		// Rond
		ctx.beginPath();
		ctx.arc(posX, posY, 4, 0, 2*Math.PI, true);
		
		ctx.shadowColor = couleur;
		ctx.shadowOffsetX = 0;
		ctx.shadowOffsetY = 0;
		ctx.shadowBlur = 4;

		ctx.fillStyle = "rgba(0,0,0,1)";
		ctx.strokeStyle = "rgba(255,255,255,0.8)";
		ctx.lineWidth = 1;
		ctx.fill()
		ctx.stroke();
	},
	
	tangente : function(sourisX, sourisY, ajouterDansListe){
		var delta = 0.001;
		
		var ctx = new Object();
		ctx = this.ctx;
		
		// Dessiner les points et tangentes supplémentaires
		this.dessinerListe();
		
		// Dessiner les tangentes des fonctions
		for(var i=0; i<fct.list.length; i++){
			var func = fct.list[i];
			// Vérifier la fonction sur laquelle doit s'appliquer l'outil
			if(!func || func == ""){
				continue;
			}
			
			// Position de la souris par rapport au coin supérieur gauche de l'affichage
			var posX = sourisX - affichage.offsetLeft - 2;
			
			// Valeur de la coordonnée x sous la souris
			var t = posX / affichage.multX + affichage.xGauche;
			if("startAngle" in func){
				if(t < func.startAngle || t > func.endAngle){
					continue;
				}
			}
			var t1 = t + delta;
			var x = func.getX(t);
			var x1 = func.getX(t1);
			// Calcul de deux valeurs y et de la pente
			var y = func.getY(t);
			var y1 = func.getY(t1);
			var pente;
			if(x1 != x){
				pente = (y1-y)/(x1-x);
			}
			else{
				pente = Number.POSITIVE_INFINITY;
			}
			
			// Dessiner la pente;
			if(!isNaN(x) && !isNaN(y) && !isNaN(y1)){
				if(ajouterDansListe){
					this.ajouterTangente(x, y, pente, func.couleur);
				}
				this.dessinerTangente(x, y, pente, func.couleur);
			}
		}
	},
	dessinerTangente : function(x, y, pente, couleur){
		var ctx = new Object();
		ctx = this.ctx;
		
		// Conversion des coordonnées
		var posX = coordToPosX(x) + 1;
		var posY = coordToPosY(y) + 1;
		
		// Carré
		ctx.shadowColor = couleur;
		ctx.shadowBlur = 4;
		
		ctx.fillStyle = "white";
		ctx.fillRect(posX-3, posY-3, 6, 6);
		
		// Pente
		ctx.strokeStyle = "white";
		ctx.lineWidth = 2;
		ctx.beginPath();
		var pente2 = pente * (affichage.hauteur/affichage.largeur) * (affichage.xDroite-affichage.xGauche)/(affichage.yHaut-affichage.yBas);
		ctx.moveTo(0, posY+posX*pente2);
		ctx.lineTo(affichage.largeur, posY+posX*pente2-affichage.largeur*pente2);
		ctx.stroke();
		
		// Texte
		var txtPente;
		if(isFinite(pente)){
			txtPente = pente.toFixed(2);
		}
		else{
			txtPente = "∞"
		}
		ctx.fillStyle = "rgba(255,255,255,0.5)";
		try{
			ctx.bulle(posX+7, posY-7, txtPente.length*5.5+1, 20);
		}
		catch(err){
			ctx.fillRect(posX+7, posY-27, txtPente.length*5.5, 20);
		}
		
		ctx.fillStyle = "black";
		ctx.shadowColor = "black";
		ctx.shadowOffsetX = 1;
		ctx.shadowOffsetY = 1;
		ctx.shadowBlur = 2;
		ctx.fillText(txtPente, posX+8+txtPente.length/4, posY-13);
	},
	
	ajouterPoint : function(x, y, couleur){
		var newPoint = {
			x : x,
			y : y,
			couleur : couleur,
			dessiner : function(){
				outil.dessinerPoint(this.x, this.y, this.couleur);
			}
		};
		this.liste.push(newPoint);
	},
	ajouterTangente : function(x, y, pente, couleur){
		var newTangente = {
			x : x,
			y : y,
			pente : pente,
			couleur : couleur,
			dessiner : function(){
				outil.dessinerTangente(this.x, this.y, this.pente, this.couleur);
			}
		};
		this.liste.push(newTangente);
	},
	dessinerListe : function(){
		if(!this.ctx){
			return;
		}
		this.ctx.clearRect(0,0,affichage.largeur*2,affichage.hauteur*2);
		for(var i=0; i<this.liste.length; i++){
			this.liste[i].dessiner();
		}
		this.actualiserListe();
	},
	actualiserListe : function(){
		var element = document.getElementById("divSuppOutil");
		element.innerHTML = "";
		
		for(var i=0; i<this.liste.length; i++){
			var posX =  Math.floor(coordToPosX(this.liste[i].x, true)+9);
			var posY = Math.floor(coordToPosY(this.liste[i].y, true)-28);
			if(posX > affichage.largeur+affichage.offsetLeft || posX < affichage.offsetLeft || posY > affichage.hauteur+affichage.offsetTop || posX < affichage.offsetTop){
				continue;
			}
			var txt  = '<div style="left:'
			txt += posX +'px;';
			txt += 'top:'
			txt += posY +'px;" ';
			txt += 'onclick="'
			txt += 'outil.supprimer('+i+')';
			txt += '">x</div>';
// 			alert(txt);
			element.innerHTML += txt;
		}
	},
	supprimer : function(id){
		outil.liste.splice(id, 1);
		outil.dessinerListe();
// 		alert(id)
	}
};

// Evènements du clavier
function keyPress(event){
	switch(event.keyCode){
		case 27:
// 			widget.resetState()
			break
		case 37:
			if(event.ctrlKey){
				affichage.deplacerX(-1)
			}
			break
		case 38:
			if(event.ctrlKey){
				affichage.deplacerY(1)
			}
			break
		case 39:
			if(event.ctrlKey){
				affichage.deplacerX(1)
			}
			break
		case 40:
			if(event.ctrlKey){
				affichage.deplacerY(-1)
			}
			break
		default:
			//alert(event.keyCode+" ; "+event.ctrlKey)
	}
}


// ---- Aire sous la fonction (intégrale) ----
function AireSousFct(fct, a, b, n){
	var aire, largeurRect, gaucheRect, droiteRect, millieuRect, hauteurRect, aireRect;
	var f = new Function("x", "return "+fct);
	aire = 0;
	largeurRect = (b-a)/n;
	for(var i=0; i<n; i++){
		gaucheRect = a + i*largeurRect;
		droiteRect = a + (i+1)*largeurRect;
		millieuRect = (gaucheRect+droiteRect) / 2;
		hauteurRect = f(millieuRect);
		aireRect = largeurRect * hauteurRect;
		aire = aire + aireRect;
	}
	//alert("a="+a+";b="+b+";n="+n+";A="+aire)
	return aire;
}

function calculerAire(){
	var fonction = fct.remplacer(fct.verifier(fct.list[editeur.idFct].fct));
	var a = parseInt(document.getElementById("aireG").value);
	var b = parseInt(document.getElementById("aireD").value);
	var n = 50;
	var arrondi = Math.round(AireSousFct(fonction, a, b, n)*1000)/1000;
	document.getElementById("outputAire").innerHTML = "A = " + arrondi;
	actualiserGraph();
}

// ---- Sauvegarde du graphique ----
var backgroundSauvegarde = "rgba(0,0,0,0.5)";

function saveGraph(){
	var userWidth = parseInt(document.getElementById("saveWidth").value);
	var userHeight = parseInt(document.getElementById("saveHeight").value);
	if(userWidth <= 0 || userHeight <= 0){
		userWidth = affichage.width;
		userHeight = affichage.height;
	}
	if(fonction3D){
		var background = display3D.background;
		display3D.background = backgroundSauvegarde;
		
		display3D.init(userWidth, userHeight);
		display3D.draw();
		
		var dataURL;
		if(document.getElementById("selectSaveType").value == "jpg"){
			dataURL = display3D.canvas.toDataURL("image/jpeg");
		}
		else{
			dataURL = display3D.canvas.toDataURL();
		}
		document.getElementById("saveImageContent").innerHTML = '<img src="' + dataURL + '"/>';
		afficherMenu('menuSaveImage');
		
		display3D.background = background;
		setTimeout(function(){
			display3D.init();
			display3D.draw();
		}, 1);
	}
	else{
		if(affichage.methode == "canvas"){
			var l = affichage.largeur;
			var h = affichage.hauteur;
			var c = affichage.couleurFond;
			
			affichage.largeur = parseInt(document.getElementById("saveWidth").value);
			affichage.hauteur = parseInt(document.getElementById("saveHeight").value);
			affichage.couleurFond = backgroundSauvegarde;
			
			var newDiv = document.createElement("div");
			affichage.init(newDiv, userWidth, userHeight);
			
			outil.init();
			outil.dessinerListe();
			affichage.ctx.drawImage(outil.canvas,0,0);
			
			var dataURL;
			if(document.getElementById("selectSaveType").value == "jpg"){
				dataURL = affichage.canvas.toDataURL("image/jpeg");
			}
			else{
				dataURL = affichage.canvas.toDataURL();
			}
			document.getElementById("saveImageContent").innerHTML = '<img src="' + dataURL + '"/>';
			afficherMenu('menuSaveImage');
			
			affichage.largeur = l;
			affichage.hauteur = h;
			affichage.couleurFond = c;
			affichage.init();
			outil.init();
		}
		else if(affichage.methode == "svg"){
			window.open("JavaScript/AffichageSVG.svg");
		}
	}
}
