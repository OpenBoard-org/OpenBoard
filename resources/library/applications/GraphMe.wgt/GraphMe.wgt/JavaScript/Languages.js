
var languages = {
	current: "en",
	
	init: function(){
		var lang = this.navigatorLanguage();
		if(lang){
			lang = lang.substr(0,2);
			this.set(lang);
		}
	},
	
	navigatorLanguage: function(){
		if(window.sankore){
			return sankore.locale();
		}
		return navigator.language || navigator.userLanguage;
	},
	
	set: function(lang){
		if(lang in langTexts){
			this.current = lang;
			this.setTexts(langTexts[lang]);
		}
	},
	
	getText: function(id){
		return langTexts[this.current][id];
	},
	
	setTexts: function(texts){
		this.id("graphMeButton").textContent = texts.widgetName;
		this.id("functionsButton").textContent = texts.functions;
		this.id("displayButton").textContent = texts.display;
		this.id("helpButton").textContent = texts.help;
		
		this.id("boutonAgrandir").title = texts.fullscreen;
		this.id("boutonSaveGraph").title = texts.save;
		this.id("inputRapideButton").value = texts.displayAction;
		
		this.id("pointTool").title = texts.pointTool;
		this.id("moveTool").title = texts.moveTool;
		this.id("tangentTool").title = texts.tangentTool;
		
		this.id("menuExempleCloseButton").value = texts.close;
		
		this.id("menuGraphMeOptionsTab").textContent = texts.options;
		this.id("menuGraphMeAboutTab").textContent = texts.about;
		this.id("widgetOptions").textContent = texts.widgetOptions;
		this.id("widgetTheme").textContent = texts.widgetTheme;
		this.id("selectThemeDarkBlue").textContent = texts.darkBlue;
		this.id("selectThemeBlack").textContent = texts.black;
		this.id("selectThemeBlue").textContent = texts.blue;
		this.id("selectThemeWhite").textContent = texts.white;
		this.id("resetWidgetButton").value = texts.resetWidget;
		this.id("checkForUpdateButton").value = texts.checkForUpdate;
		this.id("menuGraphMeCloseButton").value = texts.close;
		
		this.id("menuCreditsOptionsTab").textContent = texts.options;
		this.id("menuCreditsAboutTab").textContent = texts.about;
		this.id("widgetDevelopedBy").textContent = texts.widgetDevelopedBy;
		this.id("widgetContactInfo").textContent = texts.widgetContactInfo;
		this.id("widgetBackgroundSource").textContent = texts.widgetBackgroundSource;
		this.id("changelogButton").textContent = texts.changelogButton;
		this.id("menuCreditsCloseButton").value = texts.close;
		
		this.id("changelogTitle").textContent = texts.changelogTitle;
		this.id("menuChangelogBackButton").value = texts.back;
		this.id("menuChangelogCloseButton").value = texts.close;
		
		this.id("menuAffichageDisplayParametersTab").textContent = texts.displayParameters;
		this.id("menuAffichageFunctionsParametersTab").textContent = texts.functionsParameters;
		this.id("plotRange").textContent = texts.plotRange;
		this.id("xAxis").textContent = texts.xAxis;
		this.id("yAxis").textContent = texts.yAxis;
		this.id("xAxisTo").textContent = texts.to;
		this.id("yAxisTo").textContent = texts.to;
		this.id("defaultZoom").textContent = texts.defaultZoom;
		this.id("defaultDisplayParameters").value = texts.defaultDisplayParameters;
		this.id("displayOptions").textContent = texts.options;
		this.id("displayMethod").textContent = texts.displayMethod;
		this.id("showGrid").textContent = texts.showGrid;
		this.id("showAxis").textContent = texts.showAxis;
		this.id("showScale").textContent = texts.showScale;
		this.id("graphAccuracy").textContent = texts.graphAccuracy;
		this.id("improveAccuracy").textContent = texts.improveAccuracy;
		this.id("menuAffichageOkButton").value = texts.ok;
		
		this.id("menuFunctionParametersDisplayParametersTab").textContent = texts.displayParameters;
		this.id("menuFunctionParametersFunctionsParametersTab").textContent = texts.functionsParameters;
		this.id("defaultFunctionParameters").textContent = texts.defaultFunctionParameters;
		this.id("thickness").textContent = texts.thickness;
		this.id("drawDerivativeAndPrimitive").textContent = texts.drawDerivativeAndPrimitive;
		this.id("drawArea").textContent = texts.drawArea;
		this.id("lineStyle").textContent = texts.lineStyle;
		this.id("selectStyleLine").textContent = texts.styleLine;
		this.id("selectStyleDotted").textContent = texts.styleDotted;
		this.id("selectStyleDashed").textContent = texts.styleDashed;
		this.id("menuFunctionParametersCloseButton").value = texts.close;
		
		this.id("display3D").textContent = texts.display3D;
		this.id("displayStyle").textContent = texts.displayStyle;
		this.id("displayStyleSurfaces").textContent = texts.styleSurfaces;
		this.id("displayStyleDotted").textContent = texts.styleDotted;
		this.id("resetDisplay3D").value = texts.resetDisplay;
		this.id("graphAccuracy3D").textContent = texts.graphAccuracy;
		this.id("colorsConfig").textContent = texts.colorsConfig;
		this.id("useRedFor").textContent = texts.useRedFor;
		this.id("useGreenFor").textContent = texts.useGreenFor;
		this.id("useBlueFor").textContent = texts.useBlueFor;
		this.id("redPositive").textContent = texts.positiveValues;
		this.id("redNegative").textContent = texts.negativeValues;
		this.id("redAlways").textContent = texts.always;
		this.id("greenPositive").textContent = texts.positiveValues;
		this.id("greenNegative").textContent = texts.negativeValues;
		this.id("greenAlways").textContent = texts.always;
		this.id("bluePositive").textContent = texts.positiveValues;
		this.id("blueNegative").textContent = texts.negativeValues;
		this.id("blueAlways").textContent = texts.always;
		this.id("globalValue").textContent = texts.globalValue;
		this.id("globalValueRange").textContent = texts.globalValueRange;
		this.id("menuAffichage3dOk").value = texts.ok;
		
		this.id("menuAideUsageTab").textContent = texts.usage;
		this.id("menuAideExamplesTab").textContent = texts.examples;
		this.id("howItWorks").textContent = texts.howItWorks;
		this.id("howItWorksText").textContent = texts.howItWorksText;
		this.id("availableFunctionsText").textContent = texts.availableFunctionsText;
		this.id("basicOperations").textContent = texts.basicOperations;
		this.id("plus").textContent = texts.plus;
		this.id("minus").textContent = texts.minus;
		this.id("multiplication").textContent = texts.multiplication;
		this.id("division").textContent = texts.division;
		this.id("modulus").textContent = texts.modulus;
		this.id("trigonometricFunctions").textContent = texts.trigonometricFunctions;
		this.id("sine").textContent = texts.sine;
		this.id("cosine").textContent = texts.cosine;
		this.id("tangent").textContent = texts.tangent;
		this.id("cotangent").textContent = texts.cotangent;
		this.id("secant").textContent = texts.secant;
		this.id("cosecant").textContent = texts.cosecant;
		this.id("arcSine").textContent = texts.arcSine;
		this.id("arcCosine").textContent = texts.arcCosine;
		this.id("arcTangent").textContent = texts.arcTangent;
		this.id("arcCotangent").textContent = texts.arcCotangent;
		this.id("hyperbolicFunctions").textContent = texts.hyperbolicFunctions;
		this.id("hypSine").textContent = texts.hypSine;
		this.id("hypCosine").textContent = texts.hypCosine;
		this.id("hypTangent").textContent = texts.hypTangent;
		this.id("hypCotangent").textContent = texts.hypCotangent;
		this.id("hypSecant").textContent = texts.hypSecant;
		this.id("hypCosecant").textContent = texts.hypCosecant;
		this.id("hypArcSine").textContent = texts.hypArcSine;
		this.id("hypArcCosine").textContent = texts.hypArcCosine;
		this.id("hypArcTangent").textContent = texts.hypArcTangent;
		this.id("hypArcCotangent").textContent = texts.hypArcCotangent;
		this.id("powerAndRoot").textContent = texts.powerAndRoot;
		this.id("squareRoot").textContent = texts.squareRoot;
		this.id("power").textContent = texts.power;
		this.id("xPowY").textContent = texts.xPowY;
		this.id("root").textContent = texts.root;
		this.id("rootText").textContent = texts.rootText;
		this.id("expAndLog").textContent = texts.expAndLog;
		this.id("naturalLog").textContent = texts.naturalLog;
		this.id("decimalLog").textContent = texts.decimalLog;
		this.id("absValue").textContent = texts.absValue;
		this.id("rounding").textContent = texts.rounding;
		this.id("roundText").textContent = texts.roundText;
		this.id("ceilText").textContent = texts.ceilText;
		this.id("floorText").textContent = texts.floorText;
		this.id("constants").textContent = texts.constants;
		this.id("keyboardShortcuts").textContent = texts.keyboardShortcuts;
		this.id("moveLeft").textContent = texts.moveLeft;
		this.id("moveTop").textContent = texts.moveTop;
		this.id("moveRight").textContent = texts.moveRight;
		this.id("moveBottom").textContent = texts.moveBottom;
		this.id("menuAideCloseButton").value = texts.close;
		
		this.id("menuAideExemplesUsageTab").textContent = texts.usage;
		this.id("menuAideExemplesExamplesTab").textContent = texts.examples;
		this.id("examplesText").textContent = texts.examplesText;
		this.id("functions2d").textContent = texts.functions2d;
		this.id("functions3d").textContent = texts.functions3d;
		this.id("menuAideExemplesCloseButton").value = texts.close;
		
		this.id("functionStudyTitle").textContent = texts.functionStudy;
		this.id("domainOfDefinition").textContent = texts.domainOfDefinition;
		this.id("symmetry").textContent = texts.symmetry;
		this.id("zeros").textContent = texts.zeros;
		this.id("sign").textContent = texts.sign;
		this.id("asymptotes").textContent = texts.asymptotes;
		this.id("extremums").textContent = texts.extremums;
		this.id("inflexionPoints").textContent = texts.inflexionPoints;
		this.id("functionStudyText").textContent = texts.functionStudyText;
		this.id("menuEtudeBackButton").value = texts.back;
		this.id("menuEtudeCloseButton").value = texts.close;
		
		this.id("menuFonctionsFunctionsTab").textContent = texts.functions;
		this.id("menuFonctionsHistoryTab").textContent = texts.history;
		this.id("newFunction").textContent = texts.newFunction;
		this.id("functionTypeCartesian").textContent = texts.cartesian;
		this.id("functionTypeImplicit").textContent = texts.implicit;
		this.id("functionTypePolar").textContent = texts.polar;
		this.id("functionTypeParametric").textContent = texts.parametric;
		this.id("addFunctionButton").value = texts.add;
		this.id("functionsListText").textContent = texts.functionsListText;
		
		this.id("editorEditPageEditTab").textContent = texts.edit;
		this.id("editorEditPageToolsTab").textContent = texts.tools;
		this.id("editorRangeFrom").textContent = texts.from;
		this.id("editorRangeTo").textContent = texts.to;
		this.id("editorThickness").textContent = texts.thickness;
		this.id("editorColor").textContent = texts.color;
		this.id("editorDerivatives").textContent = texts.derivatives;
		this.id("editorPrimitive").textContent = texts.primitive;
		this.id("editorLineStyle").textContent = texts.lineStyle;
		this.id("editorStyleLine").textContent = texts.styleLine;
		this.id("editorStyleDotted").textContent = texts.styleDotted;
		this.id("editorStyleDashed").textContent = texts.styleDashed;
		this.id("editorDrawArea").textContent = texts.drawArea;
		this.id("editorDuplicate").value = texts.duplicate;
		this.id("editorRemove").value = texts.remove;
		
		this.id("editorToolsPageEditTab").textContent = texts.edit;
		this.id("editorToolsPageToolsTab").textContent = texts.tools;
		this.id("computePoint").textContent = texts.computePoint;
		this.id("computePointButton").value = texts.compute;
		this.id("computeArea").textContent = texts.computeArea;
		this.id("areaFrom").textContent = texts.from;
		this.id("areaTo").textContent = texts.to;
		this.id("computeAreaButton").value = texts.compute;
		this.id("functionStudy").textContent = texts.functionStudy;
		this.id("functionStudyButton").value = texts.startStudy;
		this.id("editorToolsDuplicate").value = texts.duplicate;
		this.id("editorToolsRemove").value = texts.remove;
		this.id("menuFonctionsCloseButton").value = texts.close;
		
		this.id("input3dDisplayButton").value = texts.displayAction;
		
		this.id("menuHistoriqueFunctionsTab").textContent = texts.functions;
		this.id("menuHistoriqueHistoryTab").textContent = texts.history;
		this.id("latestDisplayedFunctions").textContent = texts.latestDisplayedFunctions;
		this.id("menuHistoriqueCloseButton").value = texts.close;
		
		this.id("chooseColor").textContent = texts.chooseColor;
		this.id("colorPickerColor").textContent = texts.color;
		this.id("colorPickerValue").textContent = texts.value;
		this.id("colorPickerSaturation").textContent = texts.saturation;
		this.id("colorPickerRed").textContent = texts.red;
		this.id("colorPickerGreen").textContent = texts.green;
		this.id("colorPickerBlue").textContent = texts.blue;
		this.id("inputOpacity").textContent = texts.opacity;
		this.id("apercuCouleur").title = texts.newColor;
		this.id("apercuCouleur2").title = texts.oldColor;
		this.id("colorPickerCancelButton").value = texts.cancel;
		this.id("colorPickerOkButton").value = texts.ok;
		
		this.id("blackListError").textContent = texts.error;
		this.id("cannotDrawFunction").textContent = texts.cannotDrawFunction;
		this.id("invalidExpression").textContent = texts.invalidExpression;
		this.id("blackListErrorOkButton").value = texts.ok;
		this.id("fctError").textContent = texts.error;
		this.id("checkTheFunction").textContent = texts.checkTheFunction;
		this.id("errorMessage").textContent = texts.errorMessage;
		this.id("youCanFindExamples").textContent = texts.youCanFindExamples;
		this.id("fctErrorHelpButton").value = texts.help;
		this.id("fctErrorOkButton").value = texts.ok;
		
		this.id("menuSaveGraphTitle").textContent = texts.saveGraph;
		this.id("saveWidthText").textContent = texts.width;
		this.id("saveHeightText").textContent = texts.height;
		this.id("saveBackgroundColor").textContent = texts.backgroundColor;
		this.id("imageFormat").textContent = texts.imageFormat;
		this.id("menuSaveGraphCancelButton").value = texts.cancel;
		this.id("menuSaveGraphOkButton").value = texts.ok;
		
		this.id("saveImageTitle").textContent = texts.saveImageTitle;
		this.id("saveImageText").textContent = texts.saveImageText;
		this.id("menuSaveImageCloseButton").value = texts.close;
		
		this.id("updateTitle").textContent = texts.update;
		this.id("currentVersion").textContent = texts.currentVersion;
		this.id("latestVersionAvailable").textContent = texts.latestVersionAvailable;
// 		this.id("useLatestVersionText").textContent = texts.useLatestVersionText;
// 		this.id("useLatestVersion").value = texts.useLatestVersion;
		this.id("updateBackButton").value = texts.back;
		this.id("updateOkButton").value = texts.ok;
		
		this.id("updateErrorTitle").textContent = texts.error;
		this.id("updateErrorText").textContent = texts.updateErrorText;
		this.id("updateErrorOkButton").value = texts.ok;
		
		this.id("ctxMenuDisplay").textContent = texts.display;
		this.id("ctxMenuAxes").textContent = texts.axes;
		this.id("ctxMenuScale").textContent = texts.scale;
		this.id("ctxMenuGrid").textContent = texts.grid;
		this.id("ctxMenuReset").value = texts.reset;
		this.id("ctxMenuSave").value = texts.save;
	},
	
	class: function(className){
		return document.getElementsByClassName(className);
	},
	
	id: function(id){
		return document.getElementById(id);
	}
};


var langTexts = {
	"fr": {
		"widgetName": "GraphMe",
		"functions": "Fonctions",
		"display": "Affichage",
		"help": "Aide",
		"close": "Fermer",
		"back": "Retour",
		"ok": "Ok",
		"fullscreen": "Plein écran",
		"save": "Sauvegarder",
		"displayAction": "Afficher",
		"pointTool": "Outil point",
		"moveTool": "Outil déplacement",
		"tangentTool": "Outil tangente",
		"options": "Options",
		"about": "À propos",
		"darkBlue": "Bleu foncé",
		"black": "Noir",
		"blue": "Bleu",
		"white": "Blanc",
		"widgetOptions": "Options du widget",
		"widgetTheme": "Thème du widget",
		"resetWidget": "Réinitialiser le widget",
		"checkForUpdate": "Vérifier les mises à jour",
		"widgetDevelopedBy": "Widget réalisé par",
		"widgetContactInfo": "Si vous voulez rapporter un bug, avez une suggestion par rapport au widget ou voulez simplement poser une question, merci de me contacter par e-mail à l'adresse suivante",
		"widgetBackgroundSource": "Les images d'arrière-plan viennent de l'environnement de bureau KDE",
		"changelogButton": "qu'est-ce qui a changé?",
		"changelogTitle": "Notes de version",
		"displayParameters": "Paramètres d'affichage",
		"functionsParameters": "Paramètres des fonctions",
		"plotRange": "Zone d'affichage",
		"xAxis": "Axe des X",
		"yAxis": "Axe des Y",
		"to": "à",
		"defaultZoom": "Zoom par défaut",
		"defaultDisplayParameters": "Réinitialiser l'affichage",
		"displayMethod": "Méthode d'affichage du graphique",
		"showGrid": "Afficher la grille",
		"showAxis": "Afficher les axes",
		"showScale": "Afficher l'échelle",
		"graphAccuracy": "Précision du graphique, points calculés",
		"improveAccuracy": "Améliorer la précision lors d'un zoom arrière (plus lent)",
		"defaultFunctionParameters": "Paramètres par défaut des fonctions",
		"thickness": "Épaisseur",
		"drawDerivativeAndPrimitive": "Dessiner les dérivées et primitives",
		"drawArea": "Dessiner l'aire sous la fonction",
		"lineStyle": "Style de ligne",
		"styleLine": "continu",
		"styleDotted": "points",
		"styleDashed": "traits",
		"display3D": "Affichage 3D",
		"displayStyle": "Style d'affichage",
		"styleSurfaces": "surfaces",
		"resetDisplay": "Réinitialiser l'affichage",
		"colorsConfig": "Configuration des couleurs",
		"useRedFor": "utiliser le rouge pour",
		"useGreenFor": "utiliser le vert pour",
		"useBlueFor": "utiliser le bleu pour",
		"positiveValues": "valeur positive",
		"negativeValues": "valeur négative",
		"always": "toujours",
		"globalValue": "Valeur générale",
		"globalValueRange": "(entre 0 et 255)",
		"usage": "Utilisation",
		"examples": "Exemples",
		"howItWorks": "Fonctionnement",
		"howItWorksText": "Ce widget vous permet de dessiner des fonctions mathématiques. Entrez une première fonction dans le champ en haut du widget et cliquez sur le bouton \"Afficher\". Ouvrez le menu \"Fonctions\" pour modifier votre fonction ou en ajouter une nouvelle.",
		"availableFunctionsText": "Vous pouvez saisir les fonctions mathématiques suivantes",
		"basicOperations": "Les opérations de base",
		"plus": "Addition",
		"minus": "Soustraction",
		"multiplication": "Multiplication",
		"division": "Division",
		"modulus": "Modulo",
		"trigonometricFunctions": "Les fonctions trigonométriques",
		"sine": "Sinus",
		"cosine": "Cosinus",
		"tangent": "Tangente",
		"cotangent": "Cotangente",
		"secant": "Secante",
		"cosecant": "Cosecante",
		"arcSine": "Arcsinus",
		"arcCosine": "Arccosinus",
		"arcTangent": "Arctangente",
		"arcCotangent": "Arccotangente",
		"hyperbolicFunctions": "Les fonctions hyperboliques",
		"hypSine": "Sinus hyp",
		"hypCosine": "Cosinus hyp",
		"hypTangent": "Tangente hyp",
		"hypCotangent": "Cotangente hyp",
		"hypSecant": "Secante hyp",
		"hypCosecant": "Cosecante hyp",
		"hypArcSine": "Arcsinus hyp",
		"hypArcCosine": "Arccosinus hyp",
		"hypArcTangent": "Arctangente hyp",
		"hypArcCotangent": "Arccotangente hyp",
		"powerAndRoot": "Les racines et les puissances",
		"squareRoot": "Racine carrée",
		"power": "Puissances",
		"xPowY": "Élève x à une puissance y",
		"root": "Racines",
		"rootText": "Racine y d'un nombre x",
		"expAndLog": "Les exponentielles et logarithmes",
		"naturalLog": "logarithme naturel",
		"decimalLog": "logarithme de base",
		"absValue": "La valeur absolue d'un nombre",
		"rounding": "Les arrondis",
		"roundText": "arrondit à l'entier le plus proche",
		"ceilText": "arrondit à l'entier supérieur",
		"floorText": "arrondit à l'entier inférieur",
		"constants": "Constantes",
		"keyboardShortcuts": "Raccourcis clavier",
		"moveLeft": "déplacer le graphique à gauche",
		"moveTop": "déplacer le graphique en haut",
		"moveRight": "déplacer le graphique à droite",
		"moveBottom": "déplacer le graphique en bas",
		"examplesText": "(Vous pouvez cliquer sur un exemple pour l'afficher. Pensez à ajuster la précision du graphique dans les options si nécessaire.)",
		"functions2d": "Fonctions 2D",
		"functions3d": "Fonctions 3D",
		"saveGraph": "Enregistrer le graphique",
		"functionStudy": "Étude de fonction",
		"domainOfDefinition": "Ensemble de définition",
		"symmetry": "Parité",
		"zeros": "Zéros de la fonction",
		"sign": "Signe",
		"asymptotes": "Asymptotes",
		"extremums": "Extremums",
		"inflexionPoints": "Points d'inflexion",
		"functionStudyText": "L'outil permettant d'étudier les fonctions n'est pas fiable à 100%. Pensez à vérifier les résultats de l'étude avant d'en faire une quelconque utilisation.",
		"history": "Historique",
		"newFunction": "Nouvelle fonction",
		"cartesian": "Cartésienne",
		"implicit": "Implicite",
		"polar": "Polaire",
		"parametric": "Paramétrique",
		"add": "Ajouter",
		"functionsListText": "Fonction dessinées",
		"edit": "Modifier",
		"tools": "Outils",
		"from": "de",
		"to": "à",
		"color": "Couleur",
		"derivatives": "Dérivées",
		"primitive": "Primitive",
		"duplicate": "Dupliquer",
		"remove": "Supprimer",
		"computePoint": "Calculer un point de la fonction",
		"compute": "Calculer",
		"computeArea": "Calculer l'aire sous la fonction",
		"startStudy": "Démarrer l'étude",
		"latestDisplayedFunctions": "Dernières fonctions dessinées",
		"chooseColor": "Choix de la couleur",
		"value": "Valeur",
		"saturation": "Saturation",
		"red": "Rouge",
		"green": "Vert",
		"blue": "Bleu",
		"opacity": "Opacité",
		"oldColor": "Ancienne Couleur",
		"newColor": "Nouvelle Couleur",
		"cancel": "Annuler",
		"error": "Erreur",
		"cannotDrawFunction": "Impossible de dessiner la fonction",
		"invalidExpression": "Expression ou caractère invalide",
		"checkTheFunction": "Vérifiez la fonction que vous vouliez dessiner",
		"errorMessage": "Message d'erreur",
		"youCanFindExamples": "Une description ainsi que des exemples des différentes fonctions que vous pouvez dessiner se trouvent dans l'aide",
		"width": "Largeur",
		"height": "Hauteur",
		"backgroundColor": "Couleur de fond",
		"imageFormat": "Format de l'image",
		"update": "Mise à jour",
		"currentVersion": "Version en cours d'utilisation",
		"latestVersionAvailable": "Dernière version disponible en ligne",
		"useLatestVersionText": "Vous pouvez remplacer la version en cours d'utilisation par une version du widget en ligne. Notez que ceci ne met pas à jour le widget. Il faudra réitérer cette procédure la prochaine fois que vous l'utiliserez.",
		"useLatestVersion": "Utiliser la dernière version",
		"updateErrorText": "Il est impossible de mettre à jour le widget vers la version en ligne car vous utilisez déjà la version en ligne",
		"axes": "Axes",
		"scale": "Échelle",
		"grid": "Grille",
		"reset": "Réinitialiser",
		"saveImageTitle": "Sauvegarder l'image",
		"saveImageText": "Cliquez avec le bouton de droite sur l'image et sélectionnez \"Enregistrer l'image\". Il est aussi possible de choisir \"Copier l'image\" et ensuite la coller dans une autre application.",
		
		/* Inside code */
		"even": "paire",
		"odd": "impaire",
		"none_f": "aucune",
		"none_m": "aucun",
		"clickHereToModify": "Cliquez ici pour modifier les fonctions ou en ajouter une nouvelle.",
		"enterFunction": "Entrez une fonction à dessiner",
		"needHelp": "Consulter l'aide",
		"unableLoadParameters": "Impossible de charger les paramètres enregistrés..."
	}
};

