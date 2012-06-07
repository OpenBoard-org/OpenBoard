var listeOptions = ["theme","precision","decalageX","decalageY","epaisseur","grille","axes","echelle","zoom","methode","methode3D","precision3D","maj"];

function setCookie(nom,valeur){
	var jours = 545; // Durée de validité des cookies
	var date = new Date();
	date.setDate(date.getDate()+jours);
	document.cookie = nom + "=" + escape(valeur) + "; expires="+date.toGMTString();
}

function getCookie(nom){
	var start = 0;
	var end = 0;
	if(document.cookie.length>0){
		start = document.cookie.indexOf(nom + "=");
		if(start!=-1){
			start = start + nom.length+1;
			end = document.cookie.indexOf(";",start);
			if(end==-1){
				end = document.cookie.length;
			}
			return unescape(document.cookie.substring(start,end));
		}
	}
	return "";
}

function delCookie(nom){
	var date = new Date();
	date.setDate(date.getDate()-1);
	document.cookie = nom + "=; expires="+date.toGMTString();
}

function saveOptions(){
	setCookie("theme",document.getElementById("selectTheme").value);
	setCookie("precision",document.getElementById("inputPrecision").value);
	setCookie("decalageX",document.getElementById("inputDecalageX").value);
	setCookie("decalageY",document.getElementById("inputDecalageY").value);
	setCookie("epaisseur",document.getElementById("inputTaille").value);
	setCookie("grille",document.getElementById("checkGrille").checked);
	setCookie("axes",document.getElementById("checkAxes").checked);
	setCookie("echelle",document.getElementById("checkEchelle").checked);
	setCookie("zoom",document.getElementById("zoomDefaut").value);
	setCookie("methode",document.getElementById("selectMethodeAffichage").value);
	setCookie("methode3D",document.getElementById("selectAffichage3D").value);
	setCookie("precision3D",document.getElementById("inputPrecision3D").value);
	setCookie("maj",document.getElementById("checkMaJ").checked);
	document.getElementById("infoSauvegarde").style.display = "block"
}

function loadOptions(){
	if(navigator.cookieEnabled){
		if(checkOptions()){
			document.getElementById("selectTheme").value = getCookie("theme");
			changerTheme(document.getElementById("selectTheme").value);
			document.getElementById("inputPrecision").value = getCookie("precision");
			document.getElementById("inputDecalageX").value = getCookie("decalageX");
			document.getElementById("inputDecalageY").value = getCookie("decalageY");
			document.getElementById("inputTaille").value = getCookie("epaisseur");
			if(getCookie("grille")=="true"){
				document.getElementById("checkGrille").checked = true;
			}
			else{
				document.getElementById("checkGrille").checked = false;
			}
			if(getCookie("axes")=="true"){
				document.getElementById("checkAxes").checked = true;
			}
			else{
				document.getElementById("checkAxes").checked = false;
			}
			if(getCookie("echelle")=="true"){
				document.getElementById("checkEchelle").checked = true;
			}
			else{
				document.getElementById("checkEchelle").checked = false;
			}
			document.getElementById("zoomDefaut").value = getCookie("zoom");
			document.getElementById("selectMethodeAffichage").value = getCookie("methode");
			document.getElementById("selectAffichage3D").value = getCookie("methode3D");
			document.getElementById("inputPrecision3D").value = getCookie("precision3D");
			if(getCookie("maj")=="true"){
				document.getElementById("checkMaJ").checked = true;
			}
			else{
				document.getElementById("checkMaJ").checked = false;
			}
			actualiserGraph();
		}
		else{
			if(document.cookie!=""){
				alert("It's can't be downloaded ...");
			}
		}
	}
	else{
		document.getElementById("cacheCookies").style.display = "block";
	}
}

function delOptions(){
	var i;
	for(i=0; i<listeOptions.length; i++){
		delCookie(listeOptions[i]);
	}
}

function checkOptions(){
	var test = true;
	for(i=0; i<listeOptions.length; i++){
		if(getCookie(listeOptions[i])==""){
			test = false;
		}
	}
	return test;
}

function alertOptions(){
	alert(sankoreLang[lang].show_saved + "\n------------------------------------------------------------\n"+document.cookie);
}