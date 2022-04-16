// D�claration des zones de dessin
var 	aper�u = document.getElementById('aper�u'), // Fen�tre Aper�u dans l'onglet
	feuille = document.getElementById('feuille'), //Feuille qui sera envoy�e comme fond de page � Sankor�
	largeur_16_9='1820px', // Dimension de la page en 16/9
	hauteur_16_9='1024px',
	largeur_4_3='1280px', // Dimension de la page en 4/3
	hauteur_4_3='960px';
// D�claration des couleurs par d�faut	
var 	couleur_fond_uni='#ccc', // Couleur du fond uni
	couleur_marge_seyes='red',//Couleur de la marge Seyes
	couleur_marge_maternelle='red',//Couleur de la marge Maternelle
	couleur_marge_guide_ane='red',//Couleur de la marge guide_ane
	couleur_seyes_ligne_horizontale='#8E7CC3', //Violet
	couleur_seyes_ligne_verticale='#8E7CC3', //Violet
	couleur_seyes_interligne='#6FA8DC', //bleu p�le
	couleur_fond_seyes='white', // Couleur du fond Seyes
	couleur_maternelle_ligne_horizontale='#8E7CC3', //Ligne principale Violet
	couleur_maternelle_interligne='#6FA8DC', //Interligne bleu p�le
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
	couleur_mm_2='#888888', //  Lignes interm�diaires papier mm Gris
	couleur_fond_mm='white',// Couleur du fond papier mm
	couleur_point�='#888888',// Couleur des points Gris
	couleur_fond_point�='white';//Couleur de fond du papier point�
// �paisseurs de traits par d�faut
var	epaisseur_seyes=2,
	epaisseur_maternelle=2,
	epaisseur_guide_ane=2,
	epaisseur_dys=2,
	epaisseur_quadrillage=4,
	epaisseur_mm=3,
	epaisseur_point�=5;
var	position_marge_seyes=5, // Marge de 5 carreaux sur feuille Seyes finale
	position_marge_maternelle=5, // Marge de 5 carreaux sur feuille Maternelle finale
	position_marge_guide_ane=5; // Marge de 5 carreaux sur feuille Guide �ne finale

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
	$("#feuille").hide();	// On masque la feuille de papier, elle sera transf�r�e dans Open-Sankor�
	$("#aper�u").show(); // On affiche la fen�tre qui sert d'aper�u

	$("#marge_seyes").prop("checked", false); // Marge et r�glage de la marge d�sactiv� par d�faut sur papier Seyes
	$("#curseur_position_marge_seyes").hide(); 
	$("#marge_maternelle").prop("checked", false); // Marge et r�glage de la marge d�sactiv� par d�faut sur papier Maternelle
	$("#curseur_position_marge_maternelle").hide(); 
	$("#marge_guide_ane").prop("checked", false); // Marge et r�glage de la marge d�sactiv�es par d�faut sur papier Guide �ne
	$("#curseur_position_marge_guide_ane").hide();
	
	//##### S�lection du 1er onglet #####
	document.getElementById('uni').style.display = "block";//On affiche l'onglet s�lectionn�
	document.getElementsByClassName("lien_onglet")[0].className += " active"; // On active le lien
	dessiner('uni','aper�u'); // Actualisation de l'aper�u

	//##############################
	//#####		D�finition des palettes	#####
	//#####							#####
	//##### 	Comportement			#####
	//##### 	Param�trage 			#####
	//##############################
	//##### UNI #####
	$("#palette_fond_uni").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_uni,
		palette: palette2,
		change: function(c) {
			couleur_fond_uni=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('uni','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### SEYES #####
	$("#palette_seyes_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_ligne_horizontale,
		palette: palette2,
		change: function(c) {
			couleur_seyes_ligne_horizontale=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('seyes','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_seyes_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_ligne_verticale,
		palette: palette2,
		change: function(c) {
			couleur_seyes_ligne_verticale=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('seyes','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_seyes_3").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_seyes_interligne,
		palette: palette2,
		change: function(c) {
			couleur_seyes_interligne=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('seyes','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_seyes").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_seyes,
		palette: palette1,
		change: function(c) {
			couleur_fond_seyes=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('seyes','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### MATERNELLE #####
	$("#palette_maternelle_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_maternelle_ligne_horizontale,
		palette: palette2,
		change: function(c) {
			couleur_maternelle_ligne_horizontale=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_maternelle_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_maternelle_interligne,
		palette: palette2,
		change: function(c) {
			couleur_maternelle_interligne=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_maternelle").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_maternelle,
		palette: palette1,
		change: function(c) {
			couleur_fond_maternelle=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### GUIDE ANE #####
	$("#palette_ligne_guide_ane").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_ligne_guide_ane,
		palette: palette2,
		change: function(c) {
			couleur_ligne_guide_ane=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_guide_ane").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_guide_ane,
		palette: palette1,
		change: function(c) {
			couleur_fond_guide_ane=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### DYS #####
	$("#palette_dys_1").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_1,
		palette: palette2,
		change: function(c) {
			couleur_dys_1=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('dys','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_dys_2").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_2,
		palette: palette2,
		change: function(c) {
			couleur_dys_2=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('dys','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_dys_3").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_3,
		palette: palette2,
		change: function(c) {
			couleur_dys_3=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('dys','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_dys_4").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_dys_4,
		palette: palette2,
		change: function(c) {
			couleur_dys_4=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('dys','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_dys").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_dys,
		palette: palette1,
		change: function(c) {
			couleur_fond_dys=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('dys','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### QUADRILLAGE #####
	$("#palette_quadrillage").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_quadrillage,
		palette: palette2,
		change: function(c) {
			couleur_quadrillage=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('quadrillage','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_quadrillage").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_quadrillage,
		palette: palette1,
		change: function(c) {
			couleur_fond_quadrillage=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('quadrillage','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### MILLIM�TR� #####
	$("#palette_mm_ligne").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_mm_ligne,
		palette: palette2,
		change: function(c) {
			couleur_mm_ligne=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('mm','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_mm_interligne").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_mm_2,
		palette: palette2,
		change: function(c) {
			couleur_mm_2=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('mm','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_mm").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,

		color: couleur_fond_mm,
		palette: palette1,
		change: function(c) {
			couleur_fond_mm=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('mm','aper�u'); // Actualisation de l'aper�u
		}
	});
	//##### POINT� #####
	$("#palette_point�").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_point�,
		palette: palette2,
		change: function(c) {
			couleur_point�=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('point�','aper�u'); // Actualisation de l'aper�u
		}
	});
	$("#palette_fond_point�").spectrum({
		showPaletteOnly: true,
		hideAfterPaletteSelect:true,
		color: couleur_fond_point�,
		palette: palette1,
		change: function(c) {
			couleur_fond_point�=c.toRgbString();// Renvoie le code Rgba de la couleur s�lectionn�e
			dessiner('point�','aper�u'); // Actualisation de l'aper�u
		}
	});
	// ##########################
	//##### GESTION DES MARGES  #####
	// ##########################
	//##### SEYES #####
	// Modification de la case � cocher Affichage de la marge
	$('#marge_seyes').on('change', function () {
		$("#curseur_position_marge_seyes").toggle(); // Bascule de l'affichage du curseur de r�glage de la marge
		dessiner('seyes','aper�u'); // Actualisation de l'aper�u
	});
	// Modification de la position de la marge
	$( function() {$( "#curseur_position_marge_seyes" ).slider({
		value:5,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_seyes" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('seyes','aper�u'); // Actualisation de l'aper�u
			}
		});
	$( "#taille_marge_seyes" ).val( $( "#curseur_position_marge_seyes" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	//##### MATERNELLE #####
	// Modification de la case � cocher Affichage de la marge en Maternelle
	$('#marge_maternelle').on('change', function () {
		$("#curseur_position_marge_maternelle").toggle(); // Bascule de l'affichage du curseur de r�glage de la marge
		dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
	});
	// Modification de la position de la marge en Maternelle
	$( function() {$( "#curseur_position_marge_maternelle" ).slider({
		value:1,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_maternelle" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
			}
		});
	$( "#taille_marge_maternelle" ).val( $( "#curseur_position_marge_maternelle" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	//##### GUIDE_ANE #####
	// Modification de la case � cocher Affichage de la marge en Guide �ne
	$('#marge_guide_ane').on('change', function () {
		$("#curseur_position_marge_guide_ane").toggle(); // Bascule de l'affichage du curseur de r�glage de la marge
		dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
	});
	// Modification de la position de la marge Guide �ne
	$( function() {$( "#curseur_position_marge_guide_ane" ).slider({
		value:4,
		min: 1,
		max: 20,
		step: 1,
		slide:function( event, ui ) {
				$( "#taille_marge_guide_ane" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
			}
		});
	$( "#taille_marge_guide_ane" ).val( $( "#curseur_position_marge_guide_ane" ).slider( "value" ));	 // Affichage de la valeur initiale
	});
	

}
//#############################################################
//##### D�finitions des curseurs de r�glage des dimensions des HAUTEURS DE LIGNES #####
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
				$( "#taille_carreau_seyes" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change: function( event, ui ) {
				dessiner('seyes','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### MATERNELLE #####
	$( "#curseur_maternelle" ).slider({
		value:20,
		min: 10,
		max: 50,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_carreau_maternelle" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### GUIDE �NE #####
	$( "#curseur_guide_ane" ).slider({
		value:50,
		min: 10,
		max: 200,
		step: 10,
		slide:function( event, ui ) {
				$( "#interligne_guide_ane" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### DYS #####
	$( "#curseur_dys" ).slider({
		value:30,
		min: 10,
		max: 60,
		step: 5,
		slide:function( event, ui ) {
				$( "#taille_carreau_dys" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('dys','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### QUADRILLAGE #####
	$( "#curseur_quadrillage" ).slider({
		value:30,
		min: 10,
		max: 200,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_carreau_quadrillage" ).val( ui.value +"x"+ui.value); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('quadrillage','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### MILLIM�TR� #####
	$( "#curseur_mm" ).slider({
		value:10,
		min: 5,
		max: 30,
		step: 5,
		slide:function( event, ui ) {
				$( "#taille_carreau_mm" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('mm','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### POINT� #####
	$( "#curseur_point�" ).slider({
		value:30,
		min: 10,
		max: 150,
		step: 10,
		slide:function( event, ui ) {
				$( "#taille_point�" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('point�','aper�u'); // Actualisation de l'aper�u
			}
	});
		
	//##### INITIALISATION DES CURSEURS #####
	$( "#taille_carreau_maternelle" ).val( $( "#curseur_maternelle" ).slider( "value" ));	 // Affichage de la valeur initiale
	$( "#taille_carreau_seyes" ).val( $( "#curseur_seyes" ).slider( "value" ));	 // Affichage de la valeur initiale
	$( "#interligne_guide_ane" ).val( $( "#curseur_guide_ane" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_carreau_dys" ).val( $( "#curseur_dys" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_carreau_quadrillage" ).val( $( "#curseur_quadrillage" ).slider( "value" )+"x"+$( "#curseur_quadrillage" ).slider( "value" ) ); // Affichage de la valeur initiale
	$( "#taille_carreau_mm" ).val( $( "#curseur_mm" ).slider( "value" )); // Affichage de la valeur initiale
	$( "#taille_point�" ).val( $( "#curseur_point�" ).slider( "value" )); // Affichage de la valeur initiale
});

// ##############################
//##### GESTION DU PAPIER POINT�  #####
// ##############################
$( function() {
	// Changement du type de point pour le papier point� maillage carr�
	$('input[type=radio][name=type_point_page_point�]').change(function() {
		dessiner('point�','aper�u'); // Actualisation de l'aper�u
	});
	// Changement du type de papier point� maillage (carr�/iso)
	$('input[type=radio][name=type_papier_point�]').change(function() {
		dessiner('point�','aper�u'); // Actualisation de l'aper�u
	});
});

//###########################
//##### �PAISSEURS DE TRAITS #####
//##########################
$( function() {
	//##### CREATION DES CURSEURS #####
	//##### SEYES #####
	$( "#curseur_�paisseur_seyes" ).slider({
		value:epaisseur_seyes,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#�paisseur_seyes" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('seyes','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### MATERNELLE #####
	$( "#curseur_�paisseur_maternelle" ).slider({
		value:epaisseur_maternelle,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#�paisseur_maternelle" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('maternelle','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### GUIDE �NE #####
	$( "#curseur_�paisseur_guide_ane" ).slider({
		value:epaisseur_guide_ane,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#�paisseur_guide_ane" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('guide_ane','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### DYS #####
	$( "#curseur_�paisseur_dys" ).slider({
		value:epaisseur_dys,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#�paisseur_dys" ).val( ui.value );
				dessiner('dys','aper�u'); // Actualisation de l'aper�u

			}
	});
	//##### QUADRILLAGE #####
	$( "#curseur_�paisseur_quadrillage" ).slider({
		value:epaisseur_quadrillage,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#�paisseur_quadrillage" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('quadrillage','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### MILLIM�TR� #####
	$( "#curseur_�paisseur_mm" ).slider({
		value:epaisseur_mm,
		min: 1,
		max: 10,
		step: 1,
		change:function( event, ui ) {
				$( "#�paisseur_mm" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
			
		change:function( event, ui ) {
				dessiner('mm','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### POINT� #####
	$( "#curseur_�paisseur_point�" ).slider({
		value:epaisseur_point�,
		min: 1,
		max: 10,
		step: 1,
		slide:function( event, ui ) {
				$( "#�paisseur_point�" ).val( ui.value ); // Actualisation de la valeur affich�e
			},
		change:function( event, ui ) {
				dessiner('point�','aper�u'); // Actualisation de l'aper�u
			}
	});
	//##### INITIALISATION DES CURSEURS #####
	$( "#�paisseur_seyes" ).val( $( "#curseur_�paisseur_seyes" ).slider( "value" ));
	$( "#�paisseur_maternelle" ).val( $( "#curseur_�paisseur_maternelle" ).slider( "value" ));
	$( "#�paisseur_guide_ane" ).val( $( "#curseur_�paisseur_guide_ane" ).slider( "value" ));
	$( "#�paisseur_dys" ).val( $( "#curseur_�paisseur_dys" ).slider( "value" ));
	$( "#�paisseur_quadrillage" ).val( $( "#curseur_�paisseur_quadrillage" ).slider( "value" ));
	$( "#�paisseur_mm" ).val( $( "#curseur_�paisseur_mm" ).slider( "value" ));
	$( "#�paisseur_point�" ).val( $( "#curseur_�paisseur_point�" ).slider( "value" ));
	$( "#�paisseur_point�" ).val( $( "#curseur_�paisseur_point�" ).slider( "value" ));
});

 
  
//############################
//##### CR�ATION DES ONGLETS #####
//###########################
function onglet(evt, nom_onglet) {
	var i, contenu_onglet, lien_onglet;
	
	contenu_onglet = document.getElementsByClassName("contenu_onglet");//On masque le contenu de tous les onglets
	for (i = 0; i < contenu_onglet.length; i++) {
		contenu_onglet[i].style.display = "none";
	}
	lien_onglet = document.getElementsByClassName("lien_onglet");// On d�sactive les lien
	for (i = 0; i < lien_onglet.length; i++) {
		lien_onglet[i].className = lien_onglet[i].className.replace(" active", "");
	}
	document.getElementById(nom_onglet).style.display = "block";//On affiche l'onglet s�lectionn�
	evt.currentTarget.className += " active"; // On active le lien
	if (nom_onglet=='aide') {
		$("#aper�u").hide(); // On masque la fen�tre qui sert d'aper�u
	}else{
		$("#aper�u").show(); // On affiche la fen�tre qui sert d'aper�u
		dessiner(nom_onglet,'aper�u'); // Actualisation de l'aper�u
	}
}

//#######################
//##### TRAC� DU PAPIER #####
//#######################

//##### Comportement du bouton de commande Aper�u/G�n�rer #####
$(document).on('click', '.bouton_commande', function (e) {
	e.preventDefault();
	// On r�cup�ration le type de papier s�lectionn�
	var type_papier=$(this).parents('.contenu_onglet').attr('id');
	
	dessiner(type_papier,'feuille');
			var canvas = document.getElementById('feuille');
			var adresse=canvas.toDataURL('image/png',0.1); // Compression forte (0.1) car sans compression (1), image g�n�r�e trop lourde � charger sous windows
			// D�callage pour voir toutes les étiquettes
			sankore.addObject(adresse,'100%','100%',0,0,true);
			//~ Description Ajoute l'objet � la sc�ne
			//~ Param�tres
			//~ [QString] pUrl: the object URL.
			//~ [int] width: la largeur.
			//~ [int] height: la hauteur.
			//~ [int] x: the x coordinate.
			//~ [int] y: the y coordinate.
			//~ [bool] background: if this flag is true, the object is set as background.


	
})

//##### Tracer du papier sur une zone (aper�u ou feuille)
function dessiner(type_papier,zone){
	// Utilisation du bool�en 'tabelau' afin d'adapter l'interligne en fonction de l'aper�u ou du tableau
	var tableau=true; // Dessin du papier sur le tableau
	switch (zone){
		case 'aper�u': 
			var 	ctx =document.getElementById("aper�u").getContext('2d'),
				hauteur=$('#aper�u').height();
				largeur=$('#aper�u').width();
				tableau=false; // Dessin dans la fen�tre d'aper�u
			break;
		case 'feuille': 
			//~ D�tection du format de page 16/9 ou 4/3
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
	var epaisseur; // �paisseur du trac�
	var i=0,j=0,i2=0,j2=0,n=0;
	ctx.clearRect(0, 0, largeur, hauteur); // On efface le fond
		switch (type_papier){ //D�tection du papier choisi
			case "uni": 
				ctx.fillStyle =couleur_fond_uni;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			break;
			
			case "seyes": // Seyes;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_seyes;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_seyes" ).slider("value");// �paisseur d�finie par le curseur
			// Interlignes
				interligne=$( "#curseur_seyes" ).slider("value"), taille_carreau=interligne*4;//N� de ligne
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // �paisseur des lignes trac�es
				ctx.strokeStyle = couleur_seyes_interligne;
				var n=0;
				for (i = interligne; i < hauteur+1; i += interligne) {// i=0 est une ligne donc on commence � i=interligne (on en saute 1)
				    if (n % 4!== 0) { ctx.moveTo(0, i); ctx.lineTo(largeur, i); }// Tous les 4 interlignes, on saute une ligne (3 interlignes par carreaux
				    n+=1;//Ligne suivante
				}
				ctx.stroke();
			// Lignes Verticales
				var position_marge=0; // Aucune marge par d�faut
				if ($('#marge_seyes').is(':checked')){// Si la case Marge est coch�e
					position_marge_seyes=$( "#curseur_position_marge_seyes" ).slider("value");// Position de la marge d�finie par le curseur
					if (zone=='aper�u') {position_marge=taille_carreau} else {position_marge=position_marge_seyes*taille_carreau} // marge � 1 carreau sur l'aper�u et � 5 carreaux sur la feuille
				}
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.strokeStyle=couleur_seyes_ligne_verticale;
				for (i = position_marge+taille_carreau; i < largeur+1; i += taille_carreau) {// On commence � tracer les lignes verticales 1 carreau apr�s la marge
				     ctx.moveTo(i, 0);
				     ctx.lineTo(i, hauteur);
				}
				ctx.stroke();
			// Lignes Horizontales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.strokeStyle = couleur_seyes_ligne_horizontale;		
				for (i = interligne; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Affichage de la marge si n�cessaire
				if ($('#marge_seyes').is(':checked')){
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // �paisseur de la marge
					ctx.strokeStyle=couleur_marge_seyes;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}
			break;
			case "maternelle": // maternelle;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_maternelle;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On remplit le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_maternelle" ).slider("value");// �paisseur d�finie par le curseur
			// Interlignes
				if (tableau) {
					interligne=$( "#curseur_maternelle" ).slider("value")*2}//Hauteur de ligne sur la page
				else{
				interligne=$( "#curseur_maternelle" ).slider("value")*0.5}//Hauteur de ligne dans l'aper�u

				taille_carreau=interligne*3;//Hauteur de ligne
			// Lignes Horizontales principales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.strokeStyle = couleur_maternelle_ligne_horizontale;
				
				for (i = interligne; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Lignes Horizontales secondaires
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // �paisseur des lignes trac�es
				ctx.strokeStyle = couleur_maternelle_interligne;
				for (i = 0; i < largeur+1; i += taille_carreau) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Marge ?
				var position_marge=0; // Aucune marge par d�faut
				if ($('#marge_maternelle').is(':checked')){// Si la case Marge est coch�e
					position_marge_maternelle=$( "#curseur_position_marge_maternelle" ).slider("value");// Position de la marge d�finie par le curseur
					if (zone=='aper�u') {position_marge=position_marge_maternelle*taille_carreau/2} else {position_marge=position_marge_maternelle*taille_carreau} // marge � une distance diff�rente sur l'aper�u et sur le tableau (mise � l'�chelle)
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // �paisseur de la marge
					ctx.strokeStyle=couleur_marge_maternelle;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}				
			break;
			case 'guide_ane':
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_guide_ane;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_guide_ane" ).slider("value");// �paisseur d�finie par le curseur
			// Interlignes
				interligne=$( "#curseur_guide_ane" ).slider("value");//N� de ligne
			// Lignes Horizontales
				ctx.beginPath();
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.strokeStyle = couleur_ligne_guide_ane;
				for (i = interligne; i < largeur+1; i += interligne) {
				     ctx.moveTo(0, i);
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			// Affichage de la marge si n�cessaire
				var position_marge=0; // Aucune marge par d�faut
				if ($('#marge_guide_ane').is(':checked')){// Si la case Marge est coch�e
					position_marge_guide_ane=$( "#curseur_position_marge_guide_ane" ).slider("value");// Position de la marge d�finie par le curseur
					if (zone=='aper�u') {position_marge=interligne*4} else {position_marge=interligne*position_marge_guide_ane} // marge plus petite sur l'aper�u ou plus grande sur la feuille
				}
				if ($('#marge_guide_ane').is(':checked')){
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // �paisseur de la marge
					ctx.strokeStyle=couleur_marge_guide_ane;
					ctx.moveTo(position_marge, 0);
					ctx.lineTo(position_marge, hauteur);
					ctx.stroke();
				}
			break;
			
			case "dys": // Dyspraxie;
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_dys;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_dys" ).slider("value");// �paisseur d�finie par le curseur
			// Interlignes
				interligne=$( "#curseur_dys" ).slider("value");
				taille_carreau_dys=interligne*5;//Espace entre 2 lignages
			// Lignes Horizontales
				for (j= epaisseur; j< hauteur+1; j+=taille_carreau_dys) {// La premi�re ligne est enti�re, on commence � epaisseur
					
					ctx.beginPath();
					ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
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
				ctx.fillStyle =couleur_fond_quadrillage;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_quadrillage" ).slider("value");// �paisseur d�finie par le curseur
			// Lignes
				interligne=$( "#curseur_quadrillage" ).slider("value"); // Taille du quadrillage, 30 par d�faut
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.beginPath();
				ctx.strokeStyle = couleur_quadrillage;
			// D�marrage des boucle � "epaisseur" pour que les lignes p�riph�riques apparaissent en entier.				
				for(i=epaisseur;i<largeur+1;i += interligne) { // Lignes verticales
					ctx.moveTo(i, epaisseur/2);ctx.lineTo(i, hauteur);// epaisseur/2 pour que l'angle en haut � gauche soit complet
				}
			// D�marrage des boucle � "epaisseur" pour que les lignes p�riph�riques apparaissent en entier.				
				for (j=epaisseur;j<hauteur+1;j+=interligne) { // Lignes horizontales
					ctx.moveTo(epaisseur/2, j);ctx.lineTo(largeur, j);// epaisseur/2 pour que l'angle en haut � gauche soit complet
				}
				ctx.stroke(); // Affichage du trac�
			break;
								
			case "mm" :  // Papier Millim�tr�
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_mm;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// �paisseur
				epaisseur=$( "#curseur_�paisseur_mm" ).slider("value");// �paisseur d�finie par le curseur
			// Interlignes
				interligne=$( "#curseur_mm" ).slider("value"); // Taille du quadrillage, 30 par d�faut
				ctx.beginPath();
				ctx.lineWidth = epaisseur/2; // �paisseur des lignes trac�es
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
				ctx.lineWidth = epaisseur; // �paisseur des lignes trac�es
				ctx.strokeStyle=couleur_mm_ligne;
			// Verticales
			// D�marrage des boucle � "epaisseur" pour que les lignes p�riph�riques apparaissent en entier.
				for (i = epaisseur; i < largeur+1; i += 10*interligne) {
				     ctx.moveTo(i, epaisseur/2);// epaisseur/2 pour que l'angle en haut � gauche soit complet
				     ctx.lineTo(i, hauteur);
				}
			// Horizontales
			// D�marrage des boucle � "epaisseur" pour que les lignes p�riph�riques apparaissent en entier.
				for (i = epaisseur; i < largeur+1; i += 10*interligne) {
				     ctx.moveTo(epaisseur/2, i); // epaisseur/2 pour que l'angle en haut � gauche soit complet
				     ctx.lineTo(largeur, i);
				}
				ctx.stroke();
			break;
				
			case "point�":
				switch ($('input[type=radio][name=type_papier_point�]:checked').val()){//d�tection du type de papier point� carr� ou iso
				case "point�_carr�" :  // Papier Point� (maillage carr�)
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_point�;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Points
				epaisseur_point�=$( "#curseur_�paisseur_point�").slider("value"); // �paisseur du trait, 5 par d�faut
				maillage=$( "#curseur_point�" ).slider("value"); // Maillage du r�seau carr�, 30 par d�faut
				ctx.beginPath(); // D�but du trac�
				switch ($('input[type=radio][name=type_point_page_point�]:checked').val()) {
					case 'plus':
						ctx.strokeStyle = couleur_point�;
						ctx.lineWidth = epaisseur_point�/2; // �paisseur de la ligne du +
						// D�marrage des boucle � "epaisseur_point�" pour que les + en bordure apparaissent en entier.
						for (j=epaisseur_point�;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_point�;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_point�,j); // Dessin du +
								ctx.lineTo(i+epaisseur_point�,j);
								ctx.moveTo(i,j-epaisseur_point�);
								ctx.lineTo(i,j+epaisseur_point�);
							}
						}
						ctx.stroke(); // Affichage du trac�
						break;
					case 'croix':
						ctx.strokeStyle = couleur_point�;
						ctx.lineWidth = epaisseur_point�/2;// �paisseur de la ligne du x
						// D�marrage des boucle � "epaisseur_point�" pour que les croix en bordure apparaissent en entier.
						for (j=epaisseur_point�;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_point�;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_point�,j-epaisseur_point�); // Dessin de la croix
								ctx.lineTo(i+epaisseur_point�,j+epaisseur_point�);
								ctx.moveTo(i-epaisseur_point�,j+epaisseur_point�);
								ctx.lineTo(i+epaisseur_point�,j-epaisseur_point�);
							}
						}
						ctx.stroke(); // Affichage du trac�
						break;
					default : // Par d�faut, un point
						ctx.fillStyle = couleur_point�;
						// D�marrage des boucle � "epaisseur_point�" pour que les points en bordure apparaissent en entier.
						for (j=epaisseur_point�;j<hauteur;j+=maillage) { // Boucle sur la hauteur
							for (i=epaisseur_point�;i <largeur;i+=maillage) {// Boucle sur la largeur
								ctx.moveTo(i,j);
								ctx.arc(i, j, epaisseur_point�, 0, 2 * Math.PI);
							}
						}
						ctx.fill(); // Affichage du trac�
						break;
				}
			break;
				
			case "point�_iso" :  // Papier Point� Isom�trique (maillage triangulaire)
			// Couleur de fond du papier 
				ctx.fillStyle =couleur_fond_point�;//Couleur du fond s�lectionn� dans la palette
				ctx.fillRect(0, 0, largeur, hauteur); // On rempli le fond avec la couleur choisie
			// Points
				epaisseur_point�=$( "#curseur_�paisseur_point�").slider("value"); // �paisseur du trait, 5 par d�faut
				maillage=$( "#curseur_point�" ).slider("value"); // Maillage du r�seau carr�, 30 par d�faut
				var delta=maillage*Math.sqrt(3)/2; // Delta correspond � l'espace entre 2 lignes du fait de l'isom�trie des points
				ctx.beginPath(); // D�but du trac�
				switch ($('input[type=radio][name=type_point_page_point�]:checked').val()) {
					case 'plus':
						ctx.strokeStyle = couleur_point�;
						ctx.lineWidth = epaisseur_point�/2; // �paisseur de la ligne du +
						i2=0;j2=0;// D�part en haut � gauche
						// D�marrage des boucle � "epaisseur_point�" pour que les + en bordure apparaissent en entier.
						for(i=epaisseur_point�;i<largeur;i+=maillage){ // Boucle sur la hauteur
							for (j=epaisseur_point�;j<hauteur;j+=delta*2) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_point�,j); // Dessin du +
								ctx.lineTo(i+epaisseur_point�,j);
								ctx.moveTo(i,j-epaisseur_point�);
								ctx.lineTo(i,j+epaisseur_point�);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous d�call� pour former un triangle �quilat�ral
								ctx.moveTo(i2,j2); // + du dessous d�call� pour former un triangle �quilat�ral
								ctx.moveTo(i2-epaisseur_point�,j2); // Dessin du +
								ctx.lineTo(i2+epaisseur_point�,j2);
								ctx.moveTo(i2,j2-epaisseur_point�);
								ctx.lineTo(i2,j2+epaisseur_point�);
							}
						}
						ctx.stroke(); // Affichage du trac�
						break;
					case 'croix':
						ctx.strokeStyle = couleur_point�;
						ctx.lineWidth = epaisseur_point�/2; // �paisseur de la ligne du +
						i2=0;j2=0;// D�part en haut � gauche
						// D�marrage des boucle � "epaisseur_point�" pour que les croix en bordure apparaissent en entier.
						for(i=epaisseur_point�;i<largeur;i+=maillage){ // Boucle sur la hauteur
							for (j=epaisseur_point�;j<hauteur;j+=delta*2) {// Boucle sur la largeur
								ctx.moveTo(i-epaisseur_point�,j-epaisseur_point�); // Dessin du +
								ctx.lineTo(i+epaisseur_point�,j+epaisseur_point�);
								ctx.moveTo(i-epaisseur_point�,j+epaisseur_point�);
								ctx.lineTo(i+epaisseur_point�,j-epaisseur_point�);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous d�call� pour former un triangle �quilat�ral
								ctx.moveTo(i2,j2); 
								ctx.moveTo(i2-epaisseur_point�,j2-epaisseur_point�); // Dessin du +
								ctx.lineTo(i2+epaisseur_point�,j2+epaisseur_point�);
								ctx.moveTo(i2-epaisseur_point�,j2+epaisseur_point�);
								ctx.lineTo(i2+epaisseur_point�,j2-epaisseur_point�);
							}
						}
						ctx.stroke(); // Affichage du trac�
						break;
					default : // Par d�faut, un point
						ctx.fillStyle =  couleur_point�;
						i2=0;j2=0;// D�part en haut � gauche
						// D�marrage des boucle � "epaisseur_point�" pour que les points en bordure apparaissent en entier.
						for(i=epaisseur_point�;i<largeur;i+=maillage) {// Boucle sur la largeur
							for (j=epaisseur_point�;j<hauteur;j+=delta*2) { // Boucle sur la hauteur
								ctx.moveTo(i,j); // point
								ctx.arc(i, j, epaisseur_point�, 0, 2 * Math.PI);
								
								i2=i+maillage/2;j2=j+delta;// + du dessous d�call� pour former un triangle �quilat�ral
								ctx.moveTo(i2,j2); // point du dessous d�call� pour former un triangle �quilat�ral
								ctx.arc(i2, j2, epaisseur_point�, 0, 2 * Math.PI);
							}
						}
						ctx.fill();// Affichage du trac�
						break;
				}
			break;
			}
			break;
				
		}
}