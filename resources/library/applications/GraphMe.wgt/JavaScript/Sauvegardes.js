// Fonctions permettant de sauvegarder les options

var optionsLoaded = false;

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
	if(!optionsLoaded){
		return;
	}
	var state = JSON.stringify(widget.getState());
	if(window.sankore){
		sankore.setPreference("state", state);
	}
	else if(navigator.cookieEnabled){
		setCookie("state", state);
	}
}

function loadOptions(){
	var state;
	if(window.sankore){
		state = sankore.preference("state", null);
	}
	else if(navigator.cookieEnabled){
		state = getCookie("state");
	}
	if(!state){
		optionsLoaded = true;
		return;
	}
	var stateObject = JSON.parse(state);
	var goodState = widget.getState();
	if(!checkState(stateObject, goodState)){
		loadOptionsFailed();
	}
	try{
		widget.setState(stateObject);
		optionsLoaded = true;
	}
	catch(e){
		loadOptionsFailed();
	}
}

function loadOptionsFailed(){
	var text = "Unable to load the saved parameters...";
	try{
		text = languages.getText("unableLoadParameters");
	}
	catch(e){}
	alert(text);
	delOptions();
}

function checkState(state, goodState){
	for(var i in goodState){
		if(! i in state){
			return false;
		}
	}
	return true;
}

function delOptions(){
	if(window.sankore){
		sankore.setPreference("state", "");
	}
	else if(navigator.cookieEnabled){
		delCookie("state");
	}
}

function alertOptions(){
// 	alert("Options actuellement sauvegardées\n------------------------------------------------------------\n"+document.cookie);
	document.getElementById("divAlertCookies").innerHTML = document.cookie;
	afficherMenu("menuAlertCookies");
}
