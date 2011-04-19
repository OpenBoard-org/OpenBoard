
// -------------------- Fonctions de l'interface --------------------

var pleinEcran = false
var maximise = true
var tailleFenetreX = window.innerWidth 
var tailleFenetreY = window.innerHeight 
var AncienneMethodeAffichage = 0

// Cette fonction permet d'afficher le menu désiré.
// Il faut lui donner l'id du menu à afficher.
function afficherMenu(id){
	if (menuActuel == id){
		cacherMenu()
	}
	else {
		if (menuActuel !== ""){
			cacherMenu()
		}
		menuActuel = id
		document.getElementById(id).style.display = "block"
	}
}

// Permet de cacher le menu actuellement affiché
function cacherMenu(){
	document.getElementById(menuActuel).style.display = "none"
	menuActuel = ""
}

// ---- Minimiser ou Maximiser le widget (pour Uniboard) ----
function miniMax(){
	if (maximise){
		maximise = false
		document.getElementById('affichage').style.display = "none"
		document.getElementById('gauche').style.display = "none"
		document.getElementById('miniMax').innerHTML = "+"
		window.resizeTo(400,50)
		AncienneMethodeAffichage = document.getElementById("selectMethodeAffichage").selectedIndex
		document.getElementById("selectMethodeAffichage").selectedIndex = "3"
	}
	else{
		maximise = true
		document.getElementById('affichage').style.display = "block"
		document.getElementById('gauche').style.display = "block"
		document.getElementById('miniMax').innerHTML = "-"
		window.resizeTo(tailleFenetreX,tailleFenetreY)
		document.getElementById("selectMethodeAffichage").selectedIndex = AncienneMethodeAffichage
	}
}

// Action des petits bouton + et - dans les options
function boutonPlus(id, nombre){
	var element = document.getElementById(id)
	var valeurActuelle = Number(element.value)
	element.value = valeurActuelle + nombre
}
function boutonMoins(id, nombre){
	var element = document.getElementById(id)
	var valeurActuelle = Number(element.value)
	if(valeurActuelle>=nombre*2){
		element.value = valeurActuelle - nombre
	}
}

// Changer de thème
function changerTheme(){
	var theme = document.getElementById("selectTheme").value
	switch(theme){
		case "noir":
			document.body.style.backgroundImage = "url(Images/fond2.png)"
			document.getElementById("gauche").style.backgroundImage = "url(Images/gauche2.png)"
			document.getElementById("onglet3D").style.backgroundImage = "url(Images/onglet1.png)"
			document.getElementById("texteFonction").style.color = "white"
			var couleurEchelle = "rgba(255,255,255,0.8)"
			var couleurGrille = "rgba(255,255,255,0.1)"
			var couleurAxes = "rgba(0,0,0,0.5)"
			break
		case "bleu":
			document.body.style.backgroundImage = "url(Images/fond4.png)"
			document.getElementById("gauche").style.backgroundImage = "url(Images/gauche3.png)"
			document.getElementById("onglet3D").style.backgroundImage = "url(Images/onglet2.png)"
			document.getElementById("texteFonction").style.color = "white"
			var couleurEchelle = "rgba(255,255,255,0.8)"
			var couleurGrille = "rgba(255,255,255,0.1)"
			var couleurAxes = "rgba(0,0,0,0.5)"
			break
		case "blanc":
			document.body.style.backgroundImage = "url(Images/fond5.png)"
			document.getElementById("gauche").style.backgroundImage = "url(Images/gauche3.png)"
			document.getElementById("onglet3D").style.backgroundImage = "url(Images/onglet2.png)"
			document.getElementById("texteFonction").style.color = "black"
			var couleurEchelle = "rgba(0,0,0,0.8)"
			var couleurGrille = "rgba(255,255,255,0.2)"
			var couleurAxes = "rgba(0,0,0,0.5)"
			break
	}
}

// Affiche un message d'erreur
function error(err){
	alert(" Erreur sur la page...\n\n Description: " + err.description + "\n\n Cliquez sur OK pour continuer.\n\n")
}


function agrandirAffichage(){
	if (pleinEcran){
		pleinEcran = false
		if(fonction3D){
			document.getElementById('gauche3D').style.display = "block"
			largeur = 570
			document.getElementById("affichage").style.width = largeur+"px"
			document.getElementById("affichage").style.left = "59px"
			document.getElementById("flecheGauche").style.left = "67px"
			document.getElementById("flecheHaut").style.left = "290px"
			document.getElementById("flecheBas").style.left = "290px"
		}
		else{
			choixOutil(outilPrecedent)
			document.getElementById('gauche').style.display = "block"
			largeur = 500
			document.getElementById("affichage").style.width = largeur+"px"
			document.getElementById("affichage").style.left = "129px"
			document.getElementById("flecheGauche").style.left = "137px"
			document.getElementById("flecheHaut").style.left = "345px"
			document.getElementById("flecheBas").style.left = "345px"
		}
		document.getElementById('haut').style.display = "block"
		document.getElementById('onglet3D').style.display = "block"
		hauteur = 400
		document.getElementById("affichage").style.height = hauteur+"px"
		document.getElementById("affichage").style.top = "52px"
	}
	else{
		pleinEcran = true
		if(fonction3D){
			document.getElementById('gauche3D').style.display = "none"
		}
		else{
			document.getElementById('gauche').style.display = "none"
			outilPrecedent = outil
			choixOutil("deplacement")
		}
		document.getElementById('haut').style.display = "none"
		document.getElementById('onglet3D').style.display = "none"
		largeur = 625
		hauteur = 445
		document.getElementById("affichage").style.width = largeur+"px"
		document.getElementById("affichage").style.left = "15px"
		document.getElementById("affichage").style.height = hauteur+"px"
		document.getElementById("affichage").style.top = "15px"
		document.getElementById("flecheGauche").style.left = "67px"
		document.getElementById("flecheHaut").style.left = "290px"
		document.getElementById("flecheBas").style.left = "290px"
	}
	actualiserGraph()
}

// Redémarre le widget
function reset(){
	window.location.reload()
}

// Ferme le widget
function close(){
	window.close()
}

// Actions de mise à jour du widget
function miseAjour(){
	afficherMenu("mAj")
	choixOutil("deplacement")
	document.getElementById("thisVersion").innerHTML = '<object type="text/html" data="version.html"></object>'
	document.getElementById("newVersion").innerHTML = '<object type="text/html" data="http://gyb.educanet2.ch/tm-widgets/yannick/GraphMe.wgt/version.html"></object>'
}

function checkboxMaJ(){
	if(document.location.href=='http://gyb.educanet2.ch/tm-widgets/yannick/GraphMe.wgt/Grapheur.xhtml'){
		afficherMenu('erreurMaJ')
		document.getElementById("checkMaJ").checked = false
	}
	else{
		if(document.getElementById("checkMaJ").checked){
			loadOptions()
			document.getElementById("checkMaJ").checked = true
			saveOptions()
		}
		else{
			loadOptions()
			document.getElementById("checkMaJ").checked = false
			saveOptions()
		}
	}
}

function majAuto(){
	if(document.location.href=='http://gyb.educanet2.ch/tm-widgets/yannick/GraphMe.wgt/Grapheur.xhtml'){
		document.getElementById("cacheMaJ").style.display = "block"
		document.getElementById("checkMaJ").checked = true
	}
	else{
		if(document.getElementById("checkMaJ").checked){
			afficherMenu('demandeMaJ')
		}
	}
}

// Afficher une page web à la place dans la zone d'affichage
function navigateur(lien){
	cacherMenu()
	agrandirAffichage()
	document.getElementById("affichage").innerHTML = '<object type="text/html" data="'+lien+'" style="width:100%;height:100%;"></object>'
}