var listeServeurs = [
	'http://yannick.vessaz.net/GraphMe/GraphMe.wgt/'
	/*'http://gyb.educanet2.ch/tm-widgets/yannick/GraphMe.wgt/'*/
];

var widget = {
	init: function(){
// 		try{
			document.getElementById('inputRapide').value = "";
			document.getElementById('selectTheme').value = "darkblue";
			affichage.setOptions();
			document.getElementById('zoomDefaut').value = "3.7";
			document.getElementById('functionType').value = "cartesian";
			document.getElementById('inputTaille').value = "3";
			document.getElementById('checkDerivee1').checked = false;
			document.getElementById('checkDerivee2').checked = false;
			document.getElementById('checkPrimitive1').checked = false;
			document.getElementById('checkAire').checked = false;
			document.getElementById('selectStyle').value = "continu";
			document.getElementById('input3D').value = "";
			document.getElementById('inputPrecision3D').value = "0.2";
			document.getElementById('selectAffichage3D').value = "surfaces";
			document.getElementById('selectRouge3D').value = "plus";
			document.getElementById('selectVert3D').value = "moins";
			document.getElementById('selectBleu3D').value = "tout";
			document.getElementById('couleur3Dgenerale').value = "0";
			
			affichage.init();
			outil.init();
			editorPreview.init();
			display3D.checkCouleurs3D();
			loadOptions();
			
			if(!fonction3D){
				if(fct.list.length == 0){
					this.displayStartInput();
				}
				else if(!window.sankore){
					this.displayFunctionsHelp();
				}
			}
// 		}
// 		catch(err){
// 			alert(err+'\n'+err.stack);
// 		}
	},
	
	displayStartInput: function(){
		document.getElementById('divInputRapide').style.display = "block";
		var textEnterFunction = "Enter a cartesian function";
		try{
			textEnterFunction = languages.getText("enterFunction");
		}
		catch(e){}
		var textNeedHelp = "Need help ?";
		try{
			textNeedHelp = languages.getText("needHelp");
		}
		catch(e){}
		
		var text = textEnterFunction + '. <a onclick="' + "message.supprimer(); afficherMenu('menuAide') " + '">'+ textNeedHelp +'</a>'
		
		message.ajouter(275,75,text);
		setTimeout(message.supprimer, 15000);
		
		document.getElementById('inputRapide').focus();
	},
	
	addStartFunction: function(){
		message.supprimer();
		document.getElementById('divInputRapide').style.display = 'none';
		document.getElementById('input').value = document.getElementById('inputRapide').value;
		fct.ajouter();
		this.displayFunctionsHelp();
	},
	
	displayFunctionsHelp: function(){
		var text = "Click here to modify the functions or add a new function.";
		try{
			text = languages.getText("clickHereToModify");
		}
		catch(e){}
		setTimeout(function(){message.ajouter(220,35,text)}, 50);
		setTimeout(message.supprimer, 5000);
	},
	
	resize: function(){
		if(fonction3D){
			display3D.init();
			display3D.draw();
		}
		else{
			affichage.init();
		}
		outil.init();
	},
	
	reset: function(){
		delOptions();
		window.location.reload();
	},
	
	getState: function(){
		var state = {
			theme: document.getElementById("selectTheme").value,
			displayMethod: document.getElementById("selectMethodeAffichage").value,
			defaultZoom: document.getElementById("zoomDefaut").value,
			displayGrid: document.getElementById("checkGrille").checked,
			displayAxes: document.getElementById("checkAxes").checked,
			displayScale: document.getElementById("checkEchelle").checked,
			accuracy: document.getElementById("inputPrecision").value,
			improvedAccuracy: document.getElementById("checkPrecision").checked,
			thickness: document.getElementById("inputTaille").value,
			lineStyle: document.getElementById("selectStyle").value,
			derivative1: document.getElementById("checkDerivee1").checked,
			derivative2: document.getElementById("checkDerivee2").checked,
			primitive: document.getElementById("checkPrimitive1").checked,
			drawArea: document.getElementById("checkAire").checked,
			lineStyle3D: document.getElementById("selectAffichage3D").value,
			accuracy3D: document.getElementById("inputPrecision3D").value,
			plotHistory: historique.get(),
			functions: fct.get(),
			display3D: fonction3D,
			input3D: document.getElementById('input3D').value,
			currentTool: outil.actuel
		};
		if(window.sankore){
			state.fullScreen = pleinEcran;
			state.displayLeft = affichage.xGauche;
			state.displayRight = affichage.xDroite;
			state.displayUp = affichage.yHaut;
			state.displayDown = affichage.yBas;
		}
		return state;
	},
	
	setState: function(state){
		document.getElementById("selectTheme").value = state.theme;
		document.getElementById("selectMethodeAffichage").value = state.displayMethod;
		document.getElementById("zoomDefaut").value = state.defaultZoom;
		document.getElementById("checkGrille").checked = state.displayGrid;
		document.getElementById("checkAxes").checked = state.displayAxes;
		document.getElementById("checkEchelle").checked = state.displayScale;
		document.getElementById("inputPrecision").value = state.accuracy;
		document.getElementById("checkPrecision").checked = state.improvedAccuracy;
		document.getElementById("inputTaille").value = state.thickness;
		document.getElementById("selectStyle").value = state.lineStyle;
		document.getElementById("checkDerivee1").checked = state.derivative1;
		document.getElementById("checkDerivee2").checked = state.derivative2;
		document.getElementById("checkPrimitive1").checked = state.primitive;
		document.getElementById("checkAire").checked = state.drawArea;
		document.getElementById("selectAffichage3D").value = state.lineStyle3D;
		document.getElementById("inputPrecision3D").value = state.accuracy3D;
		historique.set(state.plotHistory);
		fct.set(state.functions);
		document.getElementById('input3D').value = state.input3D || "";
		if(state.display3D){
			activer3D();
		}
		
		if(window.sankore){
			if(state.fullScreen){
				agrandirAffichage();
			}
			affichage.xGauche = state.displayLeft;
			affichage.xDroite = state.displayRight;
			affichage.yHaut = state.displayUp;
			affichage.yBas = state.displayDown;
			affichage.setBornes();
		}
		
		affichage.getOptions();
		if(!window.sankore){
			affichage.initZoom2(document.getElementById('zoomDefaut').value);
		}
		changerTheme(document.getElementById("selectTheme").value);
		if(state.display3D){
			display3D.draw();
		}
		else{
			affichage.dessiner();
		}
		
		outil.choisir(state.currentTool);
	}
};
