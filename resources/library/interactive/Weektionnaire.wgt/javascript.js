// Les lignes qui suivent changent l'apparence du bouton Weektionnaire au survol de la souris.
function BoutonWeektionnaireIn() {
		document.getElementById('Fond').style.background="url(images/widget/widgetlogoover.png)";
	}

// Les lignes qui suivent rétablissent l'apparence du bouton Weektionnaire quand la souris n'est plus dessus.	
function BoutonWeektionnaireOut() {
		document.getElementById('Fond').style.background="url(images/widget/widget.png)";
	}

// Les lignes qui suivent changent l'apparence du bouton Minimiser au survol de la souris.	
function BoutonMinimiserIn() {
		document.getElementById('Fond').style.background="url(images/widget/widgetminimiserover.png)";
	}

// Les lignes qui suivent rétablissent l'apparence du bouton Minimiser quand la souris n'est plus dessus.	
function BoutonMinimiserOut() {
		document.getElementById('Fond').style.background="url(images/widget/widget.png)";
	}

// Les lignes qui suivent changent l'apparence du bouton Rechercher au survol de la souris.	
function BoutonRechercherIn() {
		document.getElementById('Fond').style.background="url(images/widget/widgetrechercherover.png)";
	}

// Les lignes qui suivent rétablissent l'apparence du bouton Rechercher quand la souris n'est plus dessus.	
function BoutonRechercherOut() {
		document.getElementById('Fond').style.background="url(images/widget/widget.png)";
	}

// Les lignes qui suivent changent l'apparence du bouton Historique au survol de la souris.	
function BoutonHistoriqueIn() {
		document.getElementById('BoutonHistorique').style.background="url(images/boutons/historiqueover.png)";
	}

// Les lignes qui suivent rétablissent l'apparence du bouton Historique quand la souris n'est plus dessus.
function BoutonHistoriqueOut() {
		document.getElementById('BoutonHistorique').style.background="url(images/boutons/historique.png)";
	}

// Les lignes qui suivent changent la taille du texte de la définition à 125%.
function TextePlusPlus(){
		document.getElementById('TexteDefinition').style.fontSize="125%";
	}

// Les lignes qui suivent changent la taille du texte de la définition à 100%.	
function TextePlus(){
		document.getElementById('TexteDefinition').style.fontSize="100%";
	}

// Les lignes qui suivent changent la taille du texte de la définition à l'originale, càd 75%.
function TexteOriginal(){
		document.getElementById('TexteDefinition').style.fontSize="75%";
	}

// Les lignes qui suivent changent la couleur du texte de la définition en noir.	
function TexteNoir(){
		document.getElementById('TexteDefinition').style.color="black";
	}

// Les lignes qui suivent changent la couleur du texte de la définition en rouge.	
function TexteRouge(){
		document.getElementById('TexteDefinition').style.color="red";
	}

// Les lignes qui suivent changent la couleur du texte de la définition en bleu.	
function TexteBleu(){
		document.getElementById('TexteDefinition').style.color="blue";
	}

// Les lignes qui suivent changent la couleur du texte de la définition en orange.	
function TexteOrange(){
		document.getElementById('TexteDefinition').style.color="orange";
	}

// Les lignes qui suivent changent la couleur du texte de la définition en vert.	
function TexteVert(){
		document.getElementById('TexteDefinition').style.color="green";
	}

// Les lignes qui suivent rétablissent la couleur du lien dont l'ID est spécifié en noir.	
function LienNoir(thingId){
		document.getElementById(thingId).style.color="black";
	}

// Les lignes qui suivent changent la couleur du lien qui change la couleur du texte en noir au survol de la souris.	
function LienGris(){
		document.getElementById('Black').style.color="Gray";
	}

// Les lignes qui suivent changent la couleur du lien qui change la couleur du texte en rouge au survol de la souris.	
function LienRouge(){
		document.getElementById('Red').style.color="red";
	}

// Les lignes qui suivent changent la couleur du lien qui change la couleur du texte en bleu au survol de la souris.
function LienBleu(){
		document.getElementById('Blue').style.color="blue";
	}

// Les lignes qui suivent changent la couleur du lien qui change la couleur du texte en orange au survol de la souris.	
function LienOrange(){
		document.getElementById('Orange').style.color="orange";
	}

// Les lignes qui suivent changent la couleur du lien qui change la couleur du texte en vert au survol de la souris.	
function LienVert(){
		document.getElementById('Green').style.color="green";
	}

// Les lignes qui suivent changent l'apparence du bouton fermer du menu Options au survol de la souris.
function FermerIn(){
		document.getElementById("Fermer").src="images/boutons/fermerover.png";
	}

// Les lignes qui suivent rétablissent l'apparence du bouton fermer du menu Options quand la souris n'est plus sur le bouton.	
function FermerOut(){
		document.getElementById("Fermer").src="images/boutons/fermer.png";
	}

// Les lignes qui suivent changent l'apparence du bouton fermer du menu Historique au survol de la souris.	
function FermerHIn(){
		document.getElementById("FermerH").src="images/boutons/fermerover.png";
	}

// Les lignes qui suivent rétabliseent l'apparence du bouton fermer du menu Historique quand la souris n'est plus sur le bouton.
function FermerHOut(){
		document.getElementById("FermerH").src="images/boutons/fermer.png";
	}

// Les lignes qui suivent ferment le menu Options au clic sur la croix en haut à droite du menu.
function FermerMenu(){
		document.getElementById('MenuOptions').style.display = "none";
	}

// Les lignes qui suivent ferment le menu Historique au clic sur la croix en haut à gauche du menu.	
function FermerHistorique(){
		document.getElementById('MenuHistorique').style.display = "none";
	}

// Les lignes qui suivent ferment les menus et l'affichage de la définition	au clic sur la zone de recherche.
function FermerDefinitionInput(){
		document.getElementById('FondDefinition').style.display = "none";
		document.getElementById('MenuOptions').style.display = "none";
		document.getElementById('MenuHistorique').style.display = "none";
	}

// Les lignes qui suivent agissent comme un "minimiser" sur n'importe quel object dont l'ID est spécifié entre parenthèses. Utilisé pour le FondDefinition.	
function Minimiser(thingId){
		var visibilite;
		visibilite = document.getElementById(thingId);
		if (visibilite.style.display == "none")
		{
		visibilite.style.display = "" ;
		} else {
		visibilite.style.display = "none" ;
		}
	}

// Les lignes qui suivent agissent comme un "minimiser" du menu Historique au clic sur le bouton Historique.	
function ToggleMenuHistorique(){
		var visibilite;
		visibilite = document.getElementById("MenuHistorique");
		if (visibilite.style.display == "none")
		{
		visibilite.style.display = "" ;
		document.getElementById("MenuOptions").style.display = "none";
		} else {
		visibilite.style.display = "none" ;
		}
	}
// Les lignes qui suivent agissent comme un "minimiser" du menu Options au clic sur le bouton Weektionnaire.	
function ToggleMenuOptions(){
		var visibilite;
		visibilite = document.getElementById("MenuOptions");
		if (visibilite.style.display == "none")
		{
		visibilite.style.display = "" ;
		document.getElementById("MenuHistorique").style.display = "none";
		} else {
		visibilite.style.display = "none" ;
		}
	}	

// Version Firefox de la recherche. Elle est rapide et efficace.	
function Rechercher(){
		// La ligne ci-dessous permet d'utiliser ce script "soit-disant" dangereux.
		netscape.security.PrivilegeManager.enablePrivilege('UniversalBrowserRead');
		// La ligne ci-dessous stocke le mot entré dans la zone de recherche dans une variable pour la recherche, ainsi que pour l'historique.
		var mot = document.getElementById('Weektionnaire').value;
		// La ligne ci-dessous place en première position la dernière entrée dans la zone de recherche.
		historique.unshift(mot);
		// La ligne ci-dessous créé la variable du lien qui permet d'aller chercher une définition selon le mot entré. Cette technique nécessite cependant de valider la recherche deux fois.
		var lien = "http://www.cnrtl.fr/definition/" +mot;
		// Definition CNRTL est le nom de l'iFrame qui va servir de conteneur temporaire. La ligne ci-dessous va donc simplement changer le lien de l'iFrame et mettre celui du mot recherché.
		document.getElementById("DefinitionCNRTL").src = lien;
		// La ligne ci-dessous remplace le contenu de la div TexteDefinition par la définition. Elle sera donc affichée.
		document.getElementById("TexteDefinition").innerHTML = frame.document.getElementById('lexicontent').innerHTML;
		// La ligne ci-dessous va afficher l'image du widget qui est, au départ, cachée, ainsi que la définition.
		document.getElementById('FondDefinition').style.display = "";
	}

// Version PHP de la recherche, le script du dessus ne fonctionnant pas sous Webkit.
function RechercherNew(){
		var mot = document.getElementById('Weektionnaire').value;
		historique.unshift(mot);
		// La ligne ci-dessous spécifie où est le document PHP.
		var lien = "http://weektionnaire.lescigales.org/WeektionnaireSearchEngine.php?recherche=" + document.getElementById('Weektionnaire').value;
		// La ligne ci-dessous va afficher le contenu du fichier PHP hebergé via un object.
		document.getElementById('TexteDefinition').innerHTML = '<object data="'+lien+'" width="242px" height="179px" type="text/html"></object>';
		document.getElementById('FondDefinition').style.display = "";
	}

// Les lignes qui suivent traitent de l'historique.

// La ligne ci-dessous créé un nouveau tableau qui stocke les mots recherchés.
var historique = new Array()
function ajouterHistorique(mot){
	historique.unshift(mot)
}

// Les lignes qui suivent actualisent l'historique à l'entrée d'un mot.
function actualiserHistorique(){
	var texteHistorique = ""
	for(var i=0; i<historique.length; i++){
		// La ligne ci-dessous créer un span avec le mot entré, et quand on clique dessus, il appelle une fonction qui modifie la valeur de la zone de recherche (plus de détails en dessous).
		texteHistorique += '<span onClick="'+"ChangerMot('"+historique[i]+"')"+'">'+historique[i]+'</span> <br/>'
	}
	document.getElementById("Mots").innerHTML = texteHistorique
}

// Les lignes ci-dessous changent la valeur de la zone de recherche nommée Weektionnaire et la remplacent par le mot cliqué qui est dans l'historique. Cette fonction est appelée au dessus.
function ChangerMot(mot){
	document.getElementById("Weektionnaire").value = mot;
	document.getElementById("MenuHistorique").style.display = "none";
}
