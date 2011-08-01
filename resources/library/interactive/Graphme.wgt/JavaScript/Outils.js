
// -------------------- Fonctions des outils --------------------

var listeFonctions = new Array()
var listeCouleurs = new Array()
var historique = new Array()
var ctxT

// Cette fonction permet d'effectuer un zoom. Elle change la valeur des inputs à gauche
// qui définnissent la zone à afficher et actualise le graphique.
function zoom(valeur){
	var diffBornes = Math.abs(parseFloat(document.getElementById("borneXGauche").value) - parseFloat(document.getElementById("borneXDroite").value))
	var ajouter = (diffBornes * valeur - diffBornes)/2
	var nouvelleValeur = parseFloat(document.getElementById("borneXGauche").value) - ajouter
	if (nouvelleValeur < 0){ nouvelleValeur = Math.ceil(nouvelleValeur) }
	if (nouvelleValeur > 0){ nouvelleValeur = Math.floor(nouvelleValeur) }
	if (nouvelleValeur == parseFloat(document.getElementById("borneXGauche").value) && valeur>1){ nouvelleValeur=nouvelleValeur-1 }
	if (nouvelleValeur == parseFloat(document.getElementById("borneXGauche").value) && valeur<1){ nouvelleValeur=nouvelleValeur+1 }
	document.getElementById("borneXGauche").value = nouvelleValeur
	var nouvelleValeur = parseFloat(document.getElementById("borneXDroite").value) + ajouter
	if (nouvelleValeur < 0){ nouvelleValeur = Math.ceil(nouvelleValeur) }
	if (nouvelleValeur > 0){ nouvelleValeur = Math.floor(nouvelleValeur) }
	if (nouvelleValeur == parseFloat(document.getElementById("borneXDroite").value) && valeur>1){ nouvelleValeur=nouvelleValeur+1 }
	if (nouvelleValeur == parseFloat(document.getElementById("borneXDroite").value) && valeur<1){ nouvelleValeur=nouvelleValeur-1 }
	document.getElementById("borneXDroite").value = nouvelleValeur

	var diffBornes = Math.abs(parseFloat(document.getElementById("borneYGauche").value) - parseFloat(document.getElementById("borneYDroite").value))
	var ajouter = (diffBornes * valeur - diffBornes)/2
	var nouvelleValeur = parseFloat(document.getElementById("borneYGauche").value) - ajouter
	if (nouvelleValeur < 0){ nouvelleValeur = Math.ceil(nouvelleValeur) }
	if (nouvelleValeur > 0){ nouvelleValeur = Math.floor(nouvelleValeur) }
	if (nouvelleValeur == parseFloat(document.getElementById("borneYGauche").value) && valeur>1){ nouvelleValeur=nouvelleValeur-1 }
	if (nouvelleValeur == parseFloat(document.getElementById("borneYGauche").value) && valeur<1){ nouvelleValeur=nouvelleValeur+1 }
	document.getElementById("borneYGauche").value = nouvelleValeur
	var nouvelleValeur = parseFloat(document.getElementById("borneYDroite").value) + ajouter
	if (nouvelleValeur < 0){ nouvelleValeur = Math.ceil(nouvelleValeur) }
	if (nouvelleValeur > 0){ nouvelleValeur = Math.floor(nouvelleValeur) }
	if (nouvelleValeur == parseFloat(document.getElementById("borneYDroite").value) && valeur>1){ nouvelleValeur=nouvelleValeur+1 }
	if (nouvelleValeur == parseFloat(document.getElementById("borneYDroite").value) && valeur<1){ nouvelleValeur=nouvelleValeur-1 }
	document.getElementById("borneYDroite").value = nouvelleValeur

	if(Math.abs(parseFloat(document.getElementById("borneXGauche").value) - parseFloat(document.getElementById("borneXDroite").value)) == 0){reinitialiserZoom(1)}
	if(Math.abs(parseFloat(document.getElementById("borneYGauche").value) - parseFloat(document.getElementById("borneYDroite").value)) == 0){reinitialiserZoom(1)}
	actualiserGraph()
}
	
// Permet de réinitialiser le zoom à la valeur donnée.
function reinitialiserZoom(valeur){
	document.getElementById("borneXGauche").value = -valeur
	document.getElementById("borneXDroite").value = valeur
	document.getElementById("borneYGauche").value = -valeur
	document.getElementById("borneYDroite").value = valeur
	angle = Math.PI/8
	valeurZoom3D = 1
	gauche3D = -6.5
	droite3D = 6.5
	precisionDroite3D = 0.02
	precisionFonction3D = 0.2
	document.getElementById("inputPrecision3D").value = 0.2
	actualiserGraph()
}
	
// Ces fonctions permettent de déplacer le graphique sur l'axe "x" et "y"
// Pour cela, elles redéfinissent la zone à afficher (à gauche dans les inputs)
	function deplacerY(valeur){
	   document.getElementById("borneYGauche").value = parseFloat(document.getElementById("borneYGauche").value) + valeur
	   document.getElementById("borneYDroite").value = parseFloat(document.getElementById("borneYDroite").value) + valeur
	   actualiserGraph()
	}
	function deplacerX(valeur){
	   document.getElementById("borneXGauche").value = parseFloat(document.getElementById("borneXGauche").value) + valeur
	   document.getElementById("borneXDroite").value = parseFloat(document.getElementById("borneXDroite").value) + valeur
	   if(fonction3D){
	      angle = angle + valeur * Math.PI/8
	   }
	   actualiserGraph()
	}

// Permet d'afficher la valeur en "y" pour un point donné en "x"
	function execute(fonction) {
	   if(check(fonction)){
	      x = document.getElementById("inputX").value
	      document.getElementById("outputX").innerHTML = " f(x) = "+eval(fonction)+""
	      //alert("Si x = "+x+" \nf(x) = "+eval(fonction)+"")
	   }
	}

// Ajoute la fonction mathématique se trouvant dans l'input en haut à une liste.
// Ceci est utile à l'affichage de plusieurs fonctions simultanées.
function menuFonctions(){
	if(fonction3D){
		afficherMenu('menuHistorique')
	}
	else{
		afficherMenu('menuFonctions')
	}
}

function ajouterFonction(fct){
	listeFonctions.push(fct)
	listeCouleurs.push("rgba(0,171,255,0.9)")
	actualiserListeFonctions()
}
function actualiserListeFonctions(){
	var texteFctSupp = ""
	for(var i=0; i<listeFonctions.length; i++){
		texteFctSupp += 'f(x)='+listeFonctions[i]+' <input type="button" value="-" onclick="listeFonctions.splice('+i+', 1); actualiserListeFonctions()"/>'
		texteFctSupp += ' <span id="FctSupp'+i+'" title="listeCouleurs['+i+']" class="boutonCouleur" onclick="colorPicker(this.id); afficherMenu(\'menuCouleur\')">....</span><br/>'
	}
	document.getElementById("fonctionsSupp").innerHTML = texteFctSupp
	actualiserGraph()
}

function actualiserHistorique(){
	var texteHistorique = ""
	for(var i=0; i<historique.length; i++){
		texteHistorique += '<span class="survol" onclick="document.getElementById(\'inputEq\').value = '
		texteHistorique += "'"+historique[i]+"'"+'; actualiserGraph()">'+historique[i]+'</span> <br/>'
	}
	document.getElementById("spanHistorique").innerHTML = texteHistorique
	document.getElementById("divHistorique").scrollTop = 0
}

// Permet de changer d'outil et de faire différentes actions lors du choix de l'outil
function choixOutil(nom){
	outil = nom
	if(outil == 'deplacement'){
		document.getElementById("affichage").style.cursor = "move"
		document.getElementById("info").style.display = "none"
	}
	else{
		document.getElementById("affichage").style.cursor = "auto"
		document.getElementById("info").style.display = "block"
	}

	if(outil == 'point'){
		document.getElementById("point").style.display = "block"
	}
	else{
		document.getElementById("point").style.display = "none"
	}

	if(outil == 'tangente'){
		document.getElementById("tangente").innerHTML = '<canvas id="canvasT" width="'+largeur+'" height="'+hauteur+'"></canvas>'
		ctxT = document.getElementById('canvasT').getContext('2d')
	}
	else{
		document.getElementById("tangente").innerHTML = ""
		ctxT = null
	}
}

// Fonctions servant à gérer les événements de la souris
function sourisDown(){
	mouseDown = true
	posSourisXinit = posSourisX
	posSourisYinit = posSourisY
}
function sourisUp(){
	mouseDown = false
}
function sourisMove(event){
	posSourisX = event.clientX
	posSourisY = event.clientY
	if(mouseDown){
		if(outil == "deplacement"){
			var valeurX = (posSourisX-posSourisXinit)/multiplicateurX
			var valeurY = (posSourisYinit-posSourisY)/multiplicateurY
			if(Math.round(Math.abs(valeurX)) > 0){
				deplacerX(-Math.round(2*valeurX)/2)
				posSourisXinit = posSourisX
			}
			if(Math.round(Math.abs(valeurY)) > 0){
				deplacerY(-Math.round(2*valeurY)/2)
				posSourisYinit = posSourisY
			}
			//decalageX = posSourisX-posSourisXinit
			//decalageY = posSourisY-posSourisYinit
			//actualiserGraph()
		}
	}
	if(outil == "point"){
		var position = Math.round((posSourisX-132)*(borneXDroite-borneXGauche)/(precision*500))
		var positionX = pointX[position]
		var positionY = pointY[position]
		if(!isNaN(positionX) && !isNaN(positionY)){
			document.getElementById("info").innerHTML = "("+Math.round((positionX/multiplicateurX+borneXGauche)*100)/100+";"+Math.round(-(positionY/multiplicateurY-borneYDroite)*100)/100+")"
			document.getElementById("point").style.left = (positionX+130-4)+"px"
			document.getElementById("point").style.top = (positionY+53-10)+"px"
		}
	}
	if(outil == "tangente"){
		var position = Math.round((posSourisX-129)/multiplicateurX/precision)
		var positionX = pointX[position]
		var positionY = pointY[position]
		var valeurPente = ((hauteur-pente[position])/multiplicateurY+borneYGauche)
		//hauteur - (((y-y1)/precision - borneYGauche)* multiplicateurY)
		document.getElementById("info").innerHTML = "("+Math.round(valeurPente*100)/100+")"
		ctxT.clearRect(0,0,largeur*2,hauteur*2)
		if(!isNaN(positionX) && !isNaN(positionY)){
			ctxT.fillStyle = "white"
			ctxT.fillRect (positionX-1, positionY-2, 6, 6)
			ctxT.strokeStyle = "white"
			ctxT.lineWidth = 2
			ctxT.beginPath()
			valeurPente = valeurPente * (hauteur/largeur) * (borneXDroite-borneXGauche)/(borneYDroite-borneYGauche)
			ctxT.moveTo(0+4, positionY+positionX*valeurPente+1)
			ctxT.lineTo(largeur+4, positionY+positionX*valeurPente-largeur*valeurPente+1)
			ctxT.stroke()
		}
	}
	//document.getElementById("info").innerHTML = " "+ (posSourisX-120) + ";" + (posSourisY-43)
}
function doubleClick(ctrlKey){
	if(ctrlKey){
		zoom(1.25)
		zoom3D(1.25)
	}
	else{
		zoom(0.8)
		zoom3D(0.8)
	}
}


// Evènements du clavier
function keyPress(event){
	switch(event.keyCode){
		case 27:
			reset()
			break
		case 37:
			if(event.ctrlKey){
				deplacerX(-1)
			}
			break
		case 38:
			if(event.ctrlKey){
				deplacerY(1)
			}
			break
		case 39:
			if(event.ctrlKey){
				deplacerX(1)
			}
			break
		case 40:
			if(event.ctrlKey){
				deplacerY(-1)
			}
			break
		default:
			//alert(event.keyCode+" ; "+event.ctrlKey)
	}
}


// ---- Fonctions de test ----	
function testSVG(){
	document.getElementById("affichage").innerHTML = '<svg:svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg"> <svg:line x1="0" y1="0" x2="300" y2="300" style="stroke:rgb(99,99,99);stroke-width:2"/> </svg:svg> '
}
function testCanvas(){
	document.getElementById("affichage").innerHTML = '<canvas id="canvas" width="'+largeur+'" height="'+hauteur+'"></canvas>'
	ctx = document.getElementById('canvas').getContext('2d')
	var lingrad = ctx.createLinearGradient(100,100,largeur-100,hauteur-100)
	lingrad.addColorStop(0, 'rgba(0,50,255,1)')
	lingrad.addColorStop(1, 'rgba(0,255,255,1)')
	ctx.fillStyle = lingrad
	ctx.textAlign = "center"
	ctx.font = "72px bold"
	ctx.fillText("Canvas", largeur/2, hauteur/2-1)
}
function testXPM(){
	document.getElementById("affichage").innerHTML = '<img src='+"'"+'data:image/xpm;ASCII,/* XPM */static char * text_xpm[] = {"100 50 2 1"," 	c None",".	c #000000","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","     .....                .....       ...............             .......                 .......   ","      .....              .....        ..................          .......                 .......   ","       ....             .....         ...................         ........               ........   ","       .....            .....         ....................        ........               ........   ","        .....          .....          ....          .......       ........               ........   ","         ....         .....           ....            .....       .........             .........   ","         .....        .....           ....             ....       .... ....             .... ....   ","          .....      .....            ....             .....      .... .....           ..... ....   ","           ....     .....             ....              ....      ....  ....           ....  ....   ","           .....    .....             ....              ....      ....  ....           ....  ....   ","            .....  .....              ....              ....      ....  .....         .....  ....   ","            ..... .....               ....              ....      ....   ....         ....   ....   ","             .........                ....             .....      ....   ....         ....   ....   ","              ........                ....             ....       ....    ....       ....    ....   ","              .......                 ....            .....       ....    ....       ....    ....   ","               .....                  ....          .......       ....    .....     .....    ....   ","               ......                 ....................        ....     ....     ....     ....   ","              .......                 ...................         ....     ....     ....     ....   ","              ........                ..................          ....     .....   .....     ....   ","             ..........               ...............             ....      ....   ....      ....   ","            ..... .....               ....                        ....      ..... .....      ....   ","            ....   .....              ....                        ....       .... ....       ....   ","           .....    ....              ....                        ....       .........       ....   ","          .....     .....             ....                        ....       .........       ....   ","         .....       .....            ....                        ....        .......        ....   ","         .....        ....            ....                        ....        .......        ....   ","        .....         .....           ....                        ....         .....         ....   ","       .....           .....          ....                        ....         .....         ....   ","       .....            ....          ....                        ....                       ....   ","      .....             .....         ....                        ....                       ....   ","     .....               .....        ....                        ....                       ....   ","     .....                ....        ....                        ....                       ....   ","    .....                 .....       ....                        ....                       ....   ","   .....                   .....      ....                        ....                       ....   ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    ","                                                                                                    "};'+"'"+'width="500" height="330"/>'
}


// ---- Aire sous la fonction (intégrale) ----
function AireSousFct(fct, a, b, n){
	var aire, largeurRect, gaucheRect, droiteRect, millieuRect, hauteurRect, aireRect;
	var f = function(x){
		return eval(fct);
	};
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
	var fonction = document.getElementById("inputEq").value
	if(check(fonction)){
		var a = parseInt(document.getElementById("aireG").value);
		var b = parseInt(document.getElementById("aireD").value);
		var n = 50;
		var arrondi = Math.round(AireSousFct(fonction, a, b, n)*1000)/1000;
		document.getElementById("outputAire").innerHTML = "A = " + arrondi;
	}
	actualiserGraph();
}