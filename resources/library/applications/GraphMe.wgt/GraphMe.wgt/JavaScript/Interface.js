
// -------------------- Fonctions de l'interface --------------------


var menuActuel = "divInputRapide"
var dernierMenu = ""

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
		dernierMenu = false;
	}
	else {
		if (menuActuel != ""){
			dernierMenu = menuActuel;
			cacherMenu()
		}
		menuActuel = id
		document.getElementById(id).style.display = "block"
		message.supprimer();
	}
}

// Permet de cacher le menu actuellement affiché
function cacherMenu(){
	if(menuActuel != ""){
		document.getElementById(menuActuel).style.display = "none"
		menuActuel = ""
	}
}

// Affiche un message d'erreur
function afficherErreur(message){
	document.getElementById("spanErreurFct").innerHTML = message;
	afficherMenu("erreurFct");
}

// Messages d'info

var message = {
	liste : [],
	ajouter : function(x, y, contenu){
		var newDiv = document.createElement("div");
		newDiv.style.left = x + "px";
		newDiv.style.top = y + "px";
		
		var textDiv = document.createElement("span");
		textDiv.innerHTML = contenu;
		newDiv.appendChild(textDiv);
		
		document.getElementById("divMessages").appendChild(newDiv);
	},
	supprimer : function(){
		var div = document.getElementById("divMessages");
		div.innerHTML = "";
	}
};


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

// Affiche le menu +
function menuFonctions(){
	if(fonction3D){
		afficherMenu('menuHistorique');
	}
	else{
		afficherMenu('menuFonctions');
		editeur.editer(0);
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
function changerTheme(theme){
	var positions = [
		"top-left",
		"top",
		"top-right",
		"right",
		"bottom-right",
		"bottom",
		"bottom-left",
		"left",
		"center"
	];
	for(var i=0; i<positions.length; i++){
		var pos = positions[i];
		var id = "background-" + pos;
		var path = "Images/" + theme + "/" + pos + ".png";
		document.getElementById(id).style.backgroundImage = "url("+path+")";
	}
// 	switch(theme){
// 		case "white":
// 			document.body.style.backgroundColor = "black";
// 			var couleurEchelle = "rgba(0,0,0,0.8)"
// 			var couleurGrille = "rgba(255,255,255,0.2)"
// 			var couleurAxes = "rgba(0,0,0,0.5)"
// 			break;
// 		default:
// 			document.body.style.backgroundColor = "transparent";
// 			var couleurEchelle = "rgba(255,255,255,0.8)"
// 			var couleurGrille = "rgba(255,255,255,0.1)"
// 			var couleurAxes = "rgba(0,0,0,0.5)"
// 			break;
// 	}
	saveOptions();
}

// Affiche un message d'erreur
function error(err){
	alert(" Erreur sur la page...\n\n Description: " + err.description + "\n\n Cliquez sur OK pour continuer.\n\n")
}


function agrandirAffichage(){
	if(pleinEcran){
		pleinEcran = false;
		document.getElementById('background-center').style.paddingTop = "30px";
		document.getElementById('ongletsHaut').style.display = "block";
		document.getElementById('onglet3D').style.display = "block";
		document.getElementById('boutonSaveGraph').style.display = "inline-block";
		if(window.sankore || window.uniboard){
			document.getElementById('zoneJoystick').style.display = "block";
			document.getElementById('zoomButtons').style.display = "block";
			document.getElementById('toolButtons').style.display = "block";
		}
	}
	else{
		pleinEcran = true;
		document.getElementById('background-center').style.paddingTop = "0px";
		document.getElementById('ongletsHaut').style.display = "none";
		document.getElementById('onglet3D').style.display = "none";
		document.getElementById('boutonSaveGraph').style.display = "none";
		if(window.sankore || window.uniboard){
			document.getElementById('zoneJoystick').style.display = "none";
			document.getElementById('zoomButtons').style.display = "none";
			document.getElementById('toolButtons').style.display = "none";
		}
	}
	widget.resize();
}

// Ferme le widget
function close(){
	window.close()
}

// Actions de mise à jour du widget
function miseAjour(){
	var txtHTML = "";
	document.getElementById("thisVersion").innerHTML = '<object type="text/html" data="version.html"></object>';
	for(var i=0; i<listeServeurs.length; i++){
		if(i != 0){
			txtHTML += '<br/>';
		}
		txtHTML += '<object type="text/html" data="'+listeServeurs[i]+'version.html"></object>';
// 		txtHTML += '<input type="button" value="utiliser" onclick="document.location.href = \''+listeServeurs[i]+'Grapheur.html\'"/>';
// 		txtHTML += '<input type="button" value="télécharger" onclick="window.open(\''+listeServeurs[i]+'../GraphMe.zip\', \'_blank\')"/>';
	}
	document.getElementById("newVersion").innerHTML = txtHTML;
	
	afficherMenu("mAj");
}

function checkboxMaJ(){
	if(versionOnline()){
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
// 	var audio = new Audio();
// 	audio.src = "version.ogg";
// 	audio.load();
// 	setTimeout(function(){
// 	window.console.log(" "+audio.duration);
// 	}, 0)
	if(versionOnline()){
		document.getElementById("cacheMaJ").style.display = "block";
		document.getElementById("checkMaJ").disabled = true;
		document.getElementById("checkMaJ").checked = true;
	}
	else{
		if(document.getElementById("checkMaJ").checked){
			afficherMenu('demandeMaJ');
		}
	}
}
function majAccept(){
// 	document.location.href='http://gyb.educanet2.ch/tm-widgets/yannick/GraphMe.wgt/Grapheur.html';
	for(var i=0; i<listeServeurs.length; i++){
		setTimeout(majServeur, i*500, listeServeurs[i]);
	}
}

function majServeur(serveur){
	var img = new Image();
	img.onload = function(){
		document.location.href = serveur + "Grapheur.html";
	};
	img.src = serveur + "icon.png";
}

// Retourne true si le widget est utilisé depuis un des sites en ligne
function versionOnline(){
	for(var i=0; i<listeServeurs.length; i++){
		if(document.location.href == listeServeurs[i]+"Grapheur.html"){
			return true;
		}
	}
	return false;
}


// Afficher une page web à la place dans la zone d'affichage
function navigateur(lien){
// 	cacherMenu()
// 	document.getElementById("affichage").innerHTML = '<object type="text/html" data="'+lien+'" style="width:100%;height:100%;"></object>'
	window.open(lien, "_blank")
}
