// Déclaration des zones de dessin
var 	aperçu = document.getElementById('aperçu'), // Fenêtre Aperçu dans l'onglet
	feuille = document.getElementById('feuille'), //Feuille qui sera envoyée comme fond de page à Sankoré
	largeur_16_9='1820px', // Dimension de la page en 16/9
	hauteur_16_9='1024px',
	largeur_4_3='1280px', // Dimension de la page en 4/3
	hauteur_4_3='960px';
// Déclaration des couleurs par défaut	
var 	couleur_fond_uni='#ccc', // Couleur du fond uni
	couleur_marge_seyes='red',//Couleur de la marge Seyes
	couleur_marge_maternelle='red',//Couleur de la marge Maternelle
	couleur_marge_guide_ane='red',//Couleur de la marge guide_ane
	couleur_seyes_ligne_horizontale='#8E7CC3', //Violet
	couleur_seyes_ligne_verticale='#8E7CC3', //Violet
	couleur_seyes_interligne='#6FA8DC', //bleu pâle
	couleur_fond_seyes='white', // Couleur du fond Seyes
	couleur_maternelle_ligne_horizontale='#8E7CC3', //Ligne principale Violet
	couleur_maternelle_interligne='#6FA8DC', //Interligne bleu pâle
	couleur_fond_maternelle='white', //Couleur de fond Maternelle
	couleur_ligne_guide_ane='rgb(207,226,243)', //Ligne principale Bleu clair
	couleur_fond_guide_ane='white',//Couleur fond guide ane
	couleur_dys_1='blue', // 1ere ligne Dys
	couleur_dys_2='green', // 2e ligne Dys
	couleur_dys_3='brown', //3e ligne Dys
	couleur_dys_4='red', //4e ligne Dys
	couleur_fond_dys='white', // Couleur de Fond Dys
	couleur_quadrillage='#4a86e8',// Couleur des Lignes du quadrillage Bleu clair
	couleur_fond_quadrillage='white',//Couleur du fond du quadrillage
	couleur_mm_ligne='black', // Lignes principales papier mm Noir
	couleur_mm_2='#888888', //  Lignes intermédiaires papier mm Gris
	couleur_fond_mm='white',// Couleur du fond papier mm
	couleur_pointé='#888888',// Couleur des points Gris
	couleur_fond_pointé='white';//Couleur de fond du papier pointé
// Épaisseurs de traits par défaut
var	epaisseur_seyes=2,
	epaisseur_maternelle=2,
	epaisseur_guide_ane=2,
	epaisseur_dys=2,
	epaisseur_quadrillage=4,
	epaisseur_mm=3,
	epaisseur_pointé=5;
var	position_marge_seyes=5, // Marge de 5 carreaux sur feuille Seyes finale
	position_marge_maternelle=5, // Marge de 5 carreaux sur feuille Maternelle finale
	position_marge_guide_ane=5; // Marge de 5 carreaux sur feuille Guide Âne finale

var	interligne;// Espacement entre les lignes

// Palette de couleurs pour les fonds (avec couleur transparente)
var palette1=[
		['rgba(0,0,0,0)',"#000","#444","#666","#888","#ccc","#eee","#fff"],
		["#f00","#f90","#ff0","#0f0","#0ff","#00f","#90f","#f0f"],
		["#f4cccc","#fce5cd","#fff2cc","#d9ead3","#d0e0e3","#cfe2f3","#d9d2e9","#ead1dc"],
		["#ea9999","#f9cb9c","#ffe599","#b6d7a8","#a2c4c9","#9fc5e8","#b4a7d6","#d5a6bd"],
		["#e06666","#f6b26b","#ffd966","#93c47d","#76a5af","#6fa8dc","#8e7cc3","#c27ba0"],
		["#c00","#e69138","#f1c232","#6aa84f","#45818e","#3d85c6","#674ea7","#a64d79"],
		["#900","#b45f06","#bf9000","#38761d","#134f5c","#0b5394","#351c75","#741b47"],
		["#600","#783f04","#7f6000","#274e13","#0c343d","#073763","#20124d","#4c1130"]
	],
// Palette de couleurs pour les lignes (sans couleur transparente)
	palette2=[
		["#000","#444","#666","#888","#ccc","#eee","#f3f3f3","#fff"],
		["#f00","#f90","#ff0","#0f0","#0ff","#00f","#90f","#f0f"],
		["#f4cccc","#fce5cd","#fff2cc","#d9ead3","#d0e0e3","#cfe2f3","#d9d2e9","#ead1dc"],
		["#ea9999","#f9cb9c","#ffe599","#b6d7a8","#a2c4c9","#9fc5e8","#b4a7d6","#d5a6bd"],
		["#e06666","#f6b26b","#ffd966","#93c47d","#76a5af","#6fa8dc","#8e7cc3","#c27ba0"],
		["#c00","#e69138","#f1c232","#6aa84f","#45818e","#3d85c6","#674ea7","#a64d79"],
		["#900","#b45f06","#bf9000","#38761d","#134f5c","#0b5394","#351c75","#741b47"],
		["#600","#783f04","#7f6000","#274e13","#0c343d","#073763","#20124d","#4c1130"]
	];

function initialisation(){
	$("#feuille").hide();	// On masque la feuille de papier, elle sera transférée dans Open-Sankoré
	$("#aperçu").show(); // On affiche la fenêtre qui sert d'aperçu

	$("#marge_seyes").prop("checked", false); // Marge et réglage de la marge désactivé par défaut sur papier Seyes
	$("#curseur_position_marge_seyes").hide(); 
	$("#marge_maternelle").prop("checked", false); // Marge et réglage de la marge désactivé par défaut sur papier Maternelle
	$("#curseur_position_marge_maternelle").hide(); 
	$("#marge_guide_ane").prop("checked", false); // Marge et réglage de la marge désactivées par défaut sur papier Guide Âne
	$("#curseur_position_marge_guide_ane").hide();
	
	//##### Sélection du 1er onglet #####
	document.getElementById('uni').style.display = "block";//On affiche l'onglet sélectionné
	document.getElementsByClassName("lien_onglet")[0].className += " active"; // On active le lien
	dessiner('uni','aperçu'); // Actualisation de l'aperçu

	//##############################
	//#####		Définition des palettes	#####
	//#####							#####
	//##### 	Comportement			#####
	//##### 	Paramétrage 			#####
	//##############################
	//##### UNI #####
	$("#palette_fond_uni").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_uni,
		palette: palette2,
		change: function(c) {
			couleur_fond_uni=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('uni','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### SEYES #####
	$("#palette_seyes_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_ligne_horizontale,
		palette: palette2,
		change: function(c) {
			couleur_seyes_ligne_horizontale=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('seyes','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_seyes_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_ligne_verticale,
		palette: palette2,
		change: function(c) {
			couleur_seyes_ligne_verticale=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('seyes','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_seyes_3").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_interligne,
		palette: palette2,
		change: function(c) {
			couleur_seyes_interligne=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('seyes','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_seyes").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_seyes,
		palette: palette1,
		change: function(c) {
			couleur_fond_seyes=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('seyes','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### MATERNELLE #####
	$("#palette_maternelle_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_maternelle_ligne_horizontale,
		palette: palette2,
		change: function(c) {
			couleur_maternelle_ligne_horizontale=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_maternelle_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_maternelle_interligne,
		palette: palette2,
		change: function(c) {
			couleur_maternelle_interligne=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_maternelle").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_maternelle,
		palette: palette1,
		change: function(c) {
			couleur_fond_maternelle=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### GUIDE ANE #####
	$("#palette_ligne_guide_ane").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_ligne_guide_ane,
		palette: palette2,
		change: function(c) {
			couleur_ligne_guide_ane=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_guide_ane").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_guide_ane,
		palette: palette1,
		change: function(c) {
			couleur_fond_guide_ane=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### DYS #####
	$("#palette_dys_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_1,
		palette: palette2,
		change: function(c) {
			couleur_dys_1=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('dys','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_dys_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_2,
		palette: palette2,
		change: function(c) {
			couleur_dys_2=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('dys','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_dys_3").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_3,
		palette: palette2,
		change: function(c) {
			couleur_dys_3=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('dys','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_dys_4").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_4,
		palette: palette2,
		change: function(c) {
			couleur_dys_4=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('dys','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_dys").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_dys,
		palette: palette1,
		change: function(c) {
			couleur_fond_dys=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('dys','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### QUADRILLAGE #####
	$("#palette_quadrillage").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_quadrillage,
		palette: palette2,
		change: function(c) {
			couleur_quadrillage=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('quadrillage','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_quadrillage").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_quadrillage,
		palette: palette1,
		change: function(c) {
			couleur_fond_quadrillage=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('quadrillage','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### MILLIMÉTRÉ #####
	$("#palette_mm_ligne").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_mm_ligne,
		palette: palette2,
		change: function(c) {
			couleur_mm_ligne=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('mm','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_mm_interligne").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_mm_2,
		palette: palette2,
		change: function(c) {
			couleur_mm_2=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('mm','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_mm").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_mm,
		palette: palette1,
		change: function(c) {
			couleur_fond_mm=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('mm','aperçu'); // Actualisation de l'aperçu
		}
	});
	//##### POINTÉ #####
	$("#palette_pointé").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_pointé,
		palette: palette2,
		change: function(c) {
			couleur_pointé=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('pointé','aperçu'); // Actualisation de l'aperçu
		}
	});
	$("#palette_fond_pointé").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_pointé,
		palette: palette1,
		change: function(c) {
			couleur_fond_pointé=c.toRgbString();// Renvoie le code Rgba de la couleur sélectionnée
			dessiner('pointé','aperçu'); // Actualisation de l'aperçu
		}
	});
	// ##########################
	//##### GESTION DES MARGES  #####
	// ##########################
	//##### SEYES #####
	// Modification de la case à cocher Affichage de la marge
	$('#marge_seyes').on('change', function () {
		$("#curseur_position_marge_seyes").toggle(); // Bascule de l'affichage du curseur de réglage de la marge
		dessiner('seyes','aperçu'); // Actualisation de l'aperçu
	});
	// Modification de la position de la marge
	$( function() {$( "#curseur_position_marge_seyes" ).slider({
		value:5,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_seyes" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('seyes','aperçu'); // Actualisation de l'aperçu
			}
		});
	$( "#taille_marge_seyes" ).val( $( "#curseur_position_marge_seyes" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	//##### MATERNELLE #####
	// Modification de la case à cocher Affichage de la marge en Maternelle
	$('#marge_maternelle').on('change', function () {
		$("#curseur_position_marge_maternelle").toggle(); // Bascule de l'affichage du curseur de réglage de la marge
		dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
	});
	// Modification de la position de la marge en Maternelle
	$( function() {$( "#curseur_position_marge_maternelle" ).slider({
		value:1,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_maternelle" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
			}
		});
	$( "#taille_marge_maternelle" ).val( $( "#curseur_position_marge_maternelle" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	//##### GUIDE_ANE #####
	// Modification de la case à cocher Affichage de la marge en Guide Âne
	$('#marge_guide_ane').on('change', function () {
		$("#curseur_position_marge_guide_ane").toggle(); // Bascule de l'affichage du curseur de réglage de la marge
		dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
	});
	// Modification de la position de la marge Guide Âne
	$( function() {$( "#curseur_position_marge_guide_ane" ).slider({
		value:4,
		min: 1,
		max: 20,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_guide_ane" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
			}
		});
	$( "#taille_marge_guide_ane" ).val( $( "#curseur_position_marge_guide_ane" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	

}
//#############################################################
//##### Définitions des curseurs de réglage des dimensions des HAUTEURS DE LIGNES #####
//############################################################
$( function() {
	//##### CREATION DES CURSEURS #####
	//##### SEYES #####
	$( "#curseur_seyes" ).slider({
		value:15,
		min: 10,
		max: 60,
		step: 5,
		slide: function( event, ui ) {
				$( "#taille_carreau_seyes" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change: function( event, ui ) {
				dessiner('seyes','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### MATERNELLE #####
	$( "#curseur_maternelle" ).slider({
		value:20,
		min: 10,
		max: 50,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_carreau_maternelle" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### GUIDE ÂNE #####
	$( "#curseur_guide_ane" ).slider({
		value:50,
		min: 10,
		max: 200,
		step: 10,
		slide:function( event, ui ) {
				$( "#interligne_guide_ane" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### DYS #####
	$( "#curseur_dys" ).slider({
		value:30,
		min: 10,
		max: 60,
		step: 5,
		slide:function( event, ui ) {
				$( "#taille_carreau_dys" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('dys','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### QUADRILLAGE #####
	$( "#curseur_quadrillage" ).slider({
		value:30,
		min: 10,
		max: 200,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_carreau_quadrillage" ).val( ui.value +"x"+ui.value); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('quadrillage','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### MILLIMÉTRÉ #####
	$( "#curseur_mm" ).slider({
		value:10,
		min: 5,
		max: 30,
		step: 5,
		slide:function( event, ui ) {
				$( "#taille_carreau_mm" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('mm','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### POINTÉ #####
	$( "#curseur_pointé" ).slider({
		value:30,
		min: 10,
		max: 150,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_pointé" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('pointé','aperçu'); // Actualisation de l'aperçu
			}
	});
		
	//##### INITIALISATION DES CURSEURS #####
	$( "#taille_carreau_maternelle" ).val( $( "#curseur_maternelle" ).slider( "value" ));	 // Affichage de la valeur initiale
	$( "#taille_carreau_seyes" ).val( $( "#curseur_seyes" ).slider( "value" ));	 // Affichage de la valeur initiale
	$( "#interligne_guide_ane" ).val( $( "#curseur_guide_ane" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_carreau_dys" ).val( $( "#curseur_dys" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_carreau_quadrillage" ).val( $( "#curseur_quadrillage" ).slider( "value" )+"x"+$( "#curseur_quadrillage" ).slider( "value" ) ); // Affichage de la valeur initiale
	$( "#taille_carreau_mm" ).val( $( "#curseur_mm" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_pointé" ).val( $( "#curseur_pointé" ).slider( "value" )); // Affichage de la valeur initiale
});

// ##############################
//##### GESTION DU PAPIER POINTÉ  #####
// ##############################
$( function() {
	// Changement du type de point pour le papier pointé maillage carré
	$('input[type=radio][name=type_point_page_pointé]').change(function() {
		dessiner('pointé','aperçu'); // Actualisation de l'aperçu
	});
	// Changement du type de papier pointé maillage (carré/iso)
	$('input[type=radio][name=type_papier_pointé]').change(function() {
		dessiner('pointé','aperçu'); // Actualisation de l'aperçu
	});
});

//###########################
//##### ÉPAISSEURS DE TRAITS #####
//##########################
$( function() {
	//##### CREATION DES CURSEURS #####
	//##### SEYES #####
	$( "#curseur_épaisseur_seyes" ).slider({
		value:epaisseur_seyes,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#épaisseur_seyes" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('seyes','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### MATERNELLE #####
	$( "#curseur_épaisseur_maternelle" ).slider({
		value:epaisseur_maternelle,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#épaisseur_maternelle" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('maternelle','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### GUIDE ÂNE #####
	$( "#curseur_épaisseur_guide_ane" ).slider({
		value:epaisseur_guide_ane,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#épaisseur_guide_ane" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### DYS #####
	$( "#curseur_épaisseur_dys" ).slider({
		value:epaisseur_dys,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#épaisseur_dys" ).val( ui.value );
				dessiner('dys','aperçu'); // Actualisation de l'aperçu

			}
	});
	//##### QUADRILLAGE #####
	$( "#curseur_épaisseur_quadrillage" ).slider({
		value:epaisseur_quadrillage,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#épaisseur_quadrillage" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('quadrillage','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### MILLIMÉTRÉ #####
	$( "#curseur_épaisseur_mm" ).slider({
		value:epaisseur_mm,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#épaisseur_mm" ).val( ui.value ); // Actualisation de la valeur affichée
			},
			
		change:function( event, ui ) {
				dessiner('mm','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### POINTÉ #####
	$( "#curseur_épaisseur_pointé" ).slider({
		value:epaisseur_pointé,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#épaisseur_pointé" ).val( ui.value ); // Actualisation de la valeur affichée
			},
		change:function( event, ui ) {
				dessiner('pointé','aperçu'); // Actualisation de l'aperçu
			}
	});
	//##### INITIALISATION DES CURSEURS #####
	$( "#épaisseur_seyes" ).val( $( "#curseur_épaisseur_seyes" ).slider( "value" ));
	$( "#épaisseur_maternelle" ).val( $( "#curseur_épaisseur_maternelle" ).slider( "value" ));
	$( "#épaisseur_guide_ane" ).val( $( "#curseur_épaisseur_guide_ane" ).slider( "value" ));
	$( "#épaisseur_dys" ).val( $( "#curseur_épaisseur_dys" ).slider( "value" ));
	$( "#épaisseur_quadrillage" ).val( $( "#curseur_épaisseur_quadrillage" ).slider( "value" ));
	$( "#épaisseur_mm" ).val( $( "#curseur_épaisseur_mm" ).slider( "value" ));
	$( "#épaisseur_pointé" ).val( $( "#curseur_épaisseur_pointé" ).slider( "value" ));
	$( "#épaisseur_pointé" ).val( $( "#curseur_épaisseur_pointé" ).slider( "value" ));
});

 
  
//############################
//##### CRÉATION DES ONGLETS #####
//###########################
function onglet(evt, nom_onglet) {
	var i, contenu_onglet, lien_onglet;
	
	contenu_onglet = document.getElementsByClassName("contenu_onglet");//On masque le contenu de tous les onglets
	for (i = 0; i < contenu_onglet.length; i++) {
		contenu_onglet[i].style.display = "none";
	}
	lien_onglet = document.getElementsByClassName("lien_onglet");// On désactive les lien
	for (i = 0; i < lien_onglet.length; i++) {
		lien_onglet[i].className = lien_onglet[i].className.replace(" active", "");
	}
	document.getElementById(nom_onglet).style.display = "block";//On affiche l'onglet sélectionné
	evt.currentTarget.className += " active"; // On active le lien
	if (nom_onglet=='aide') {
		$("#aperçu").hide(); // On masque la fenêtre qui sert d'aperçu
	}else{
		$("#aperçu").show(); // On affiche la fenêtre qui sert d'aperçu
		dessiner(nom_onglet,'aperçu'); // Actualisation de l'aperçu
	}
}

//#######################
//##### TRACÉ DU PAPIER #####
//#######################

//##### Comportement du bouton de commande Aperçu/Générer #####
$(document).on('click', '.bouton_commande', function (e) {
	e.preventDefault();
	// On récupèration le type de papier sélectionné
	var type_papier=$(this).parents('.contenu_onglet').attr('id');
	
	dessiner(type_papier,'feuille');
			var canvas = document.getElementById('feuille');
			var adresse=canvas.toDataURL('image/png',0.1); // Compression forte (0.1) car sans compression (1), image générée trop lourde à charger sous windows
			// Décallage pour voir toutes les Ã©tiquettes
			sankore.addObject(adresse,'100%','100%',0,0,true);
			//~ Description Ajoute l'objet à la scène
			//~ Paramètres
			//~ [QString] pUrl: the object URL.
			//~ [int] width: la largeur.
			//~ [int] height: la hauteur.
			//~ [int] x: the x coordinate.
			//~ [int] y: the y coordinate.
			//~ [bool] background: if this flag is true, the object is set as background.


	
})

//##### Tracer du papier sur une zone (aperçu ou feuille)
function dessiner(type_papier,zone){
	// Utilisation du booléen 'tabelau' afin d'adapter l'interligne en fonction de l'aperçu ou du tableau
	var tableau=true; // Dessin du papier sur le tableau
	switch (zone){
		case 'aperçu': 
			var 	ctx =document.getElementById("aperçu").getContext('2d'),
				hauteur=$('#aperçu').height();
				largeur=$('#aperçu').width();
				tableau=false; // Dessin dans la fenêtre d'aperçu
			break;
		case 'feuille': 
			//~ Détection du format de page 16/9 ou 4/3
			var origine="taille_page_"+type_papier;
					
			if ($('input[type=radio][name=taille_page_'+type_papier+']:checked').val()=='16/9') {
				
				$('#feuille').attr({width: largeur_16_9,height:hauteur_16_9});
			}else{
				
				$('#feuille').attr({width: largeur_4_3,height:hauteur_4_3});
			}
			var 	ctx =document.getElementById("feuille").getContext('2d'),
				hauteur=$('#feuille').height();
				largeur=$('#feuille').width();
			break;
			
	}
	var epaisseur; // Épaisseur du tracé
	var i=0,j=0,i2=0,j2=0,n=0;
	ctx.clearRect(0, 0, largeur, hauteur); // On efface le fond
		switch (type_papier){ //Détection du papier choisi
			case "uni": 
				ctx.fillStyle =couleur_fond_uni;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			break;
			
			case "seyes": // Seyes;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_seyes;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_seyes" ).slider("value");// Épaisseur définie par le curseur
			// Interlignes
				interligne=$( "#curseur_seyes" ).slider("value"), taille_carreau=interligne*4;//N° de ligne
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_seyes_interligne;
				var n=0;
				for (i = interligne; i < hauteur+1; i += interligne) {// i=0 est une ligne donc on commence à i=interligne (on en saute 1)
				    if (n % 4!== 0) { ctx.moveTo(0, i); ctx.lineTo(largeur, i); }// Tous les 4 interlignes, on saute une ligne (3 interlignes par carreaux
				    n+=1;//Ligne suivante
				}
				ctx.stroke();
			// Lignes Verticales
				var position_marge=0; // Aucune marge par défaut
				if ($('#marge_seyes').is(':checked')){// Si la case Marge est cochée
					position_marge_seyes=$( "#curseur_position_marge_seyes" ).slider("value");// Position de la marge définie par le curseur
					if (zone=='aperçu') {position_marge=taille_carreau} else {position_marge=position_marge_seyes*taille_carreau} // marge à 1 carreau sur l'aperçu et à 5 carreaux sur la feuille
				}
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.strokeStyle=couleur_seyes_ligne_verticale;
				for (i = position_marge+taille_carreau; i < largeur+1; i += taille_carreau) {// On commence à tracer les lignes verticales 1 carreau après la marge
				     ctx.moveTo(i, 0);
				     ctx.lineTo(i, hauteur);
				}
				ctx.stroke();
			// Lignes Horizontales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_seyes_ligne_horizontale;		
				for (i = interligne; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Affichage de la marge si nécessaire
				if ($('#marge_seyes').is(':checked')){
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // épaisseur de la marge
					ctx.strokeStyle=couleur_marge_seyes;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}
			break;
			case "maternelle": // maternelle;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_maternelle;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On remplit le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_maternelle" ).slider("value");// Épaisseur définie par le curseur
			// Interlignes
				if (tableau) {
					interligne=$( "#curseur_maternelle" ).slider("value")*2}//Hauteur de ligne sur la page
				else{
				interligne=$( "#curseur_maternelle" ).slider("value")*0.5}//Hauteur de ligne dans l'aperçu

				taille_carreau=interligne*3;//Hauteur de ligne
			// Lignes Horizontales principales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_maternelle_ligne_horizontale;
				
				for (i = interligne; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Lignes Horizontales secondaires
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_maternelle_interligne;
				for (i = 0; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Marge ?
				var position_marge=0; // Aucune marge par défaut
				if ($('#marge_maternelle').is(':checked')){// Si la case Marge est cochée
					position_marge_maternelle=$( "#curseur_position_marge_maternelle" ).slider("value");// Position de la marge définie par le curseur
					if (zone=='aperçu') {position_marge=position_marge_maternelle*taille_carreau/2} else {position_marge=position_marge_maternelle*taille_carreau} // marge à une distance différente sur l'aperçu et sur le tableau (mise à l'échelle)
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // épaisseur de la marge
					ctx.strokeStyle=couleur_marge_maternelle;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}				
			break;
			case 'guide_ane':
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_guide_ane;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_guide_ane" ).slider("value");// Épaisseur définie par le curseur
			// Interlignes
				interligne=$( "#curseur_guide_ane" ).slider("value");//N° de ligne
			// Lignes Horizontales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_ligne_guide_ane;
				for (i = interligne; i < largeur+1; i += interligne) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Affichage de la marge si nécessaire
				var position_marge=0; // Aucune marge par défaut
				if ($('#marge_guide_ane').is(':checked')){// Si la case Marge est cochée
					position_marge_guide_ane=$( "#curseur_position_marge_guide_ane" ).slider("value");// Position de la marge définie par le curseur
					if (zone=='aperçu') {position_marge=interligne*4} else {position_marge=interligne*position_marge_guide_ane} // marge plus petite sur l'aperçu ou plus grande sur la feuille
				}
				if ($('#marge_guide_ane').is(':checked')){
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // épaisseur de la marge
					ctx.strokeStyle=couleur_marge_guide_ane;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}
			break;
			
			case "dys": // Dyspraxie;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_dys;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_dys" ).slider("value");// Épaisseur définie par le curseur
			// Interlignes
				interligne=$( "#curseur_dys" ).slider("value");
				taille_carreau_dys=interligne*5;//Espace entre 2 lignages
			// Lignes Horizontales
				for (j= epaisseur; j< hauteur+1; j+=taille_carreau_dys) {// La première ligne est entière, on commence à epaisseur
					
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
					ctx.strokeStyle = couleur_dys_1;
					ctx.moveTo(i, j);
					ctx.lineTo(largeur, j);
					ctx.stroke();
					
					ctx.beginPath();
					j+=interligne*2;
					ctx.strokeStyle = couleur_dys_2;
					ctx.moveTo(i, j);
					ctx.lineTo(largeur,j);
					ctx.stroke();
					
					ctx.beginPath();
					j+=interligne;
					ctx.strokeStyle = couleur_dys_3;
					ctx.moveTo(0, j);
					ctx.lineTo(largeur, j);
					ctx.stroke();
					
					ctx.beginPath();
					j+=interligne*2;
					ctx.strokeStyle = couleur_dys_4;
					ctx.moveTo(0,j);
					ctx.lineTo(largeur, j);
					
					ctx.stroke();
				}
			break;
				
			case "quadrillage" : // Petits Carreaux type 10x10
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_quadrillage;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_quadrillage" ).slider("value");// Épaisseur définie par le curseur
			// Lignes
				interligne=$( "#curseur_quadrillage" ).slider("value"); // Taille du quadrillage, 30 par défaut
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.beginPath();
				ctx.strokeStyle = couleur_quadrillage;
			// Démarrage des boucle à "epaisseur" pour que les lignes périphériques apparaissent en entier.				
				for(i=epaisseur;i<largeur+1;i += interligne) { // Lignes verticales
					ctx.moveTo(i, epaisseur/2);ctx.lineTo(i, hauteur);// epaisseur/2 pour que l'angle en haut à gauche soit complet
				}
			// Démarrage des boucle à "epaisseur" pour que les lignes périphériques apparaissent en entier.				
				for (j=epaisseur;j<hauteur+1;j+=interligne) { // Lignes horizontales
					ctx.moveTo(epaisseur/2, j);ctx.lineTo(largeur, j);// epaisseur/2 pour que l'angle en haut à gauche soit complet
				}
				ctx.stroke(); // Affichage du tracé
			break;
								
			case "mm" :  // Papier Millimétré
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_mm;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Épaisseur
				epaisseur=$( "#curseur_épaisseur_mm" ).slider("value");// Épaisseur définie par le curseur
			// Interlignes
				interligne=$( "#curseur_mm" ).slider("value"); // Taille du quadrillage, 30 par défaut
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // épaisseur des lignes tracées
				ctx.strokeStyle = couleur_mm_2;
			// Horizontales
				for (i = epaisseur; i < hauteur+1; i += interligne) {
					if (n%10!== 0) { ctx.moveTo(epaisseur, i); ctx.lineTo(largeur, i); }
					n+=1;
				}			
			// Verticales
				n=0;
				for (j = epaisseur; j < largeur+1; j += interligne) {
				    if (n%10!== 0) { ctx.moveTo(j,epaisseur); ctx.lineTo(j,hauteur); }
				    n+=1;
				}
				ctx.stroke(); // Tracer des lignes
			
			// Lignes principales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // épaisseur des lignes tracées
				ctx.strokeStyle=couleur_mm_ligne;
			// Verticales
			// Démarrage des boucle à "epaisseur" pour que les lignes périphériques apparaissent en entier.
				for (i = epaisseur; i < largeur+1; i += 10*interligne) {
				     ctx.moveTo(i, epaisseur/2);// epaisseur/2 pour que l'angle en haut à gauche soit complet
				     ctx.lineTo(i, hauteur);
				}
			// Horizontales
			// Démarrage des boucle à "epaisseur" pour que les lignes périphériques apparaissent en entier.
				for (i = epaisseur; i < largeur+1; i += 10*interligne) {
				     ctx.moveTo(epaisseur/2, i); // epaisseur/2 pour que l'angle en haut à gauche soit complet
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			break;
				
			case "pointé":
				switch ($('input[type=radio][name=type_papier_pointé]:checked').val()){//détection du type de papier pointé carré ou iso
				case "pointé_carré" :  // Papier Pointé (maillage carré)
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_pointé;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Points
				epaisseur_pointé=$( "#curseur_épaisseur_pointé").slider("value"); // Épaisseur du trait, 5 par défaut
				maillage=$( "#curseur_pointé" ).slider("value"); // Maillage du réseau carré, 30 par défaut
				ctx.beginPath(); // Début du tracé
				switch ($('input[type=radio][name=type_point_page_pointé]:checked').val()) {
					case 'plus':
						ctx.strokeStyle = couleur_pointé;
						ctx.lineWidth = epaisseur_pointé/2; // épaisseur de la ligne du +
						// Démarrage des boucle à "epaisseur_pointé" pour que les + en bordure apparaissent en entier.
						for (j=epaisseur_pointé;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_pointé;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_pointé,j); // Dessin du +
								ctx.lineTo(i+epaisseur_pointé,j);
								ctx.moveTo(i,j-epaisseur_pointé);
								ctx.lineTo(i,j+epaisseur_pointé);
							}
						}
						ctx.stroke(); // Affichage du tracé
						break;
					case 'croix':
						ctx.strokeStyle = couleur_pointé;
						ctx.lineWidth = epaisseur_pointé/2;// épaisseur de la ligne du x
						// Démarrage des boucle à "epaisseur_pointé" pour que les croix en bordure apparaissent en entier.
						for (j=epaisseur_pointé;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_pointé;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_pointé,j-epaisseur_pointé); // Dessin de la croix
								ctx.lineTo(i+epaisseur_pointé,j+epaisseur_pointé);
								ctx.moveTo(i-epaisseur_pointé,j+epaisseur_pointé);
								ctx.lineTo(i+epaisseur_pointé,j-epaisseur_pointé);
							}
						}
						ctx.stroke(); // Affichage du tracé
						break;
					default : // Par défaut, un point
						ctx.fillStyle = couleur_pointé;
						// Démarrage des boucle à "epaisseur_pointé" pour que les points en bordure apparaissent en entier.
						for (j=epaisseur_pointé;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_pointé;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i,j);
								ctx.arc(i, j, epaisseur_pointé, 0, 2 * Math.PI);
							}
						}
						ctx.fill(); // Affichage du tracé
						break;
				}
			break;
				
			case "pointé_iso" :  // Papier Pointé Isométrique (maillage triangulaire)
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_pointé;//Couleur du fond sélectionné dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Points
				epaisseur_pointé=$( "#curseur_épaisseur_pointé").slider("value"); // Épaisseur du trait, 5 par défaut
				maillage=$( "#curseur_pointé" ).slider("value"); // Maillage du réseau carré, 30 par défaut
				var delta=maillage*Math.sqrt(3)/2; // Delta correspond à l'espace entre 2 lignes du fait de l'isométrie des points
				ctx.beginPath(); // Début du tracé
				switch ($('input[type=radio][name=type_point_page_pointé]:checked').val()) {
					case 'plus':
						ctx.strokeStyle = couleur_pointé;
						ctx.lineWidth = epaisseur_pointé/2; // épaisseur de la ligne du +
						i2=0;j2=0;// Départ en haut à gauche
						// Démarrage des boucle à "epaisseur_pointé" pour que les + en bordure apparaissent en entier.
						for(i=epaisseur_pointé;i<largeur;i+=maillage){ // Boucle sur la hauteur
							for (j=epaisseur_pointé;j<hauteur;j+=delta*2) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_pointé,j); // Dessin du +
								ctx.lineTo(i+epaisseur_pointé,j);
								ctx.moveTo(i,j-epaisseur_pointé);
								ctx.lineTo(i,j+epaisseur_pointé);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous décallé pour former un triangle équilatéral
								ctx.moveTo(i2,j2); // + du dessous décallé pour former un triangle équilatéral
								ctx.moveTo(i2-epaisseur_pointé,j2); // Dessin du +
								ctx.lineTo(i2+epaisseur_pointé,j2);
								ctx.moveTo(i2,j2-epaisseur_pointé);
								ctx.lineTo(i2,j2+epaisseur_pointé);
							}
						}
						ctx.stroke(); // Affichage du tracé
						break;
					case 'croix':
						ctx.strokeStyle = couleur_pointé;
						ctx.lineWidth = epaisseur_pointé/2; // épaisseur de la ligne du +
						i2=0;j2=0;// Départ en haut à gauche
						// Démarrage des boucle à "epaisseur_pointé" pour que les croix en bordure apparaissent en entier.
						for(i=epaisseur_pointé;i<largeur;i+=maillage){ // Boucle sur la hauteur
							for (j=epaisseur_pointé;j<hauteur;j+=delta*2) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_pointé,j-epaisseur_pointé); // Dessin du +
								ctx.lineTo(i+epaisseur_pointé,j+epaisseur_pointé);
								ctx.moveTo(i-epaisseur_pointé,j+epaisseur_pointé);
								ctx.lineTo(i+epaisseur_pointé,j-epaisseur_pointé);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous décallé pour former un triangle équilatéral
								ctx.moveTo(i2,j2); 
								ctx.moveTo(i2-epaisseur_pointé,j2-epaisseur_pointé); // Dessin du +
								ctx.lineTo(i2+epaisseur_pointé,j2+epaisseur_pointé);
								ctx.moveTo(i2-epaisseur_pointé,j2+epaisseur_pointé);
								ctx.lineTo(i2+epaisseur_pointé,j2-epaisseur_pointé);
							}
						}
						ctx.stroke(); // Affichage du tracé
						break;
					default : // Par défaut, un point
						ctx.fillStyle =  couleur_pointé;
						i2=0;j2=0;// Départ en haut à gauche
						// Démarrage des boucle à "epaisseur_pointé" pour que les points en bordure apparaissent en entier.
						for(i=epaisseur_pointé;i<largeur;i+=maillage) {// Boucle sur la largeur
							for (j=epaisseur_pointé;j<hauteur;j+=delta*2) { // Boucle sur la hauteur
								ctx.moveTo(i,j); // point
								ctx.arc(i, j, epaisseur_pointé, 0, 2 * Math.PI);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous décallé pour former un triangle équilatéral
								ctx.moveTo(i2,j2); // point du dessous décallé pour former un triangle équilatéral
								ctx.arc(i2, j2, epaisseur_pointé, 0, 2 * Math.PI);
							}
						}
						ctx.fill();// Affichage du tracé
						break;
				}
			break;
			}
			break;
				
		}
}