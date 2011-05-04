	
	var opaciteDIV=0 //opacité de la div organites
	var opaciteINFOS=0 //opacité de la div informations
	var INFOS_affiche=false //bool pour la fonction clicINFOS()
	var t_div //pour les setinterval d'animation
	var t_inf
	var tempsAnim=25 //durée en milisecondes des animations de fondu
	var t_aura//pour l'animation de l'aura au passage de la souris
	var opaciteAura=0// idem que t_aura
	var noyauSel = false //pour pouvoir accèder à l'ADN depuis le noyau
	var loupe_on = false // permet d'accéder aux zooms de l'ADN et de l'ARN
	var interactif_on = false // permet d'accéder aux parties interactives s'il y en a
	var elt_a_zoomer = "" //pour savoir s'il faut afficher adn_zoom.jpg ou arn_zoom.jpg
	var ZOOM_affiche=false //pour savoir si la div de zoom est affichées ou pas
	var opaciteZOOM = 0
	var t_zoom
	var loaded = true

	
	function vesicule() {
		loupe_on = false
		
		afficherInfosOrganite(txt_vesicule)
		document.getElementById("img_organite").src="images/vesic_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function lysosome() {
		loupe_on = false
		
		afficherInfosOrganite(txt_lysosome)
		document.getElementById("img_organite").src="images/lys_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function mitoch() {
		loupe_on = false
		
		afficherInfosOrganite(txt_mitoch)
		document.getElementById("img_organite").src="images/mitoch_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function golgi () {
		loupe_on = false
		
		afficherInfosOrganite(txt_golgi)
		document.getElementById("img_organite").src="images/golgi_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function rer() {
		loupe_on = false
		
		afficherInfosOrganite(txt_rer)
		document.getElementById("img_organite").src="images/rer_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function noy() {
		loupe_on = false
		
		afficherInfosOrganite(txt_noy)
		document.getElementById("img_organite").src="images/noy_in.jpg"
		noyauSel = true
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function rel() {
		loupe_on = false
		
		afficherInfosOrganite(txt_rel)
		document.getElementById("img_organite").src="images/rel_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function adn() {
		if(noyauSel) {
			elt_a_zoomer="ADN"
			loupe_on = true
			noyauSel = false
			
			afficherInfosOrganite(txt_adn)
			document.getElementById("div_zoom").innerHTML='<img src="images/'+elt_a_zoomer+'_zoom.jpg" alt="" />'
			
			/*Uniboard 4 ne supportant pas bien les gifs...*/
			if(window.uniboard){
				document.getElementById("img_ADN").src="images/adn.jpg"
			} else {
				document.getElementById("img_ADN").src="images/ADN.gif"
			}
			
			document.getElementById("img_organite").src="images/fond_ADN.png"
			document.getElementById("div_organite").style.zIndex="3"
			document.getElementById("div_aura").style.zIndex="-1"
			document.getElementById("div_ADN").style.zIndex="2"
		}
	}
	
	function rib() {
		interactif_on="ARN"
		loupe_on = false
		
		afficherInfosOrganite(txt_rib)
		document.getElementById("img_organite").src="images/rib_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function centriole(){
		loupe_on = false
		
		afficherInfosOrganite(txt_centr)
		document.getElementById("img_organite").src="images/centriole_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	function arn(){
		elt_a_zoomer="ARN"
		loupe_on = true
		
		afficherInfosOrganite(txt_arn)
		document.getElementById("div_zoom").innerHTML='<img src="images/'+elt_a_zoomer+'_zoom.jpg" alt="" />'
		
		document.getElementById("img_organite").src="images/ARN_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
	}
	
	function nucl(){
		loupe_on = false
		
		afficherInfosOrganite(txt_nucl)
		document.getElementById("img_organite").src="images/nucl_in.jpg"
		document.getElementById("div_organite").style.zIndex="1"
		document.getElementById("div_aura").style.zIndex="-1"
		t_div=setInterval(afficherDIV,tempsAnim)
	}
	
	/*Fonctions relatives à la div d'infos*/
	function infos() {
		if(!INFOS_affiche){
			document.getElementById("div_infos").style.zIndex="4"
			t_inf=setInterval(afficherINFOS,50)
		}
	}
	
	function afficherINFOS() {
		if (opaciteINFOS<1) {
			opaciteINFOS=opaciteINFOS+0.1
			document.getElementById("div_infos").style.opacity=opaciteINFOS
		} else {
			clearInterval(t_inf)
			INFOS_affiche=true
		}
		
	}
	
	function clicINFOS() {
		if (INFOS_affiche) {
			t_inf=setInterval(cacherINFOS,50)
		}
		
		if (ZOOM_affiche) {
			t_zoom=setInterval(cacherZOOM,50)
		}
	}
	
	function cacherINFOS() {
		if (opaciteINFOS>0) {
			opaciteINFOS=opaciteINFOS-0.1
			document.getElementById("div_infos").style.opacity=opaciteINFOS
		} else {
			document.getElementById("div_infos").style.zIndex="-1"
			clearInterval(t_inf)
			INFOS_affiche=false
		}
	}
	
	/*Animations pour la div de zoom*/
	
	function loupe_interact(){
		
		if(loupe_on&&!ZOOM_affiche){
			
			zoom()
		}
		if(interactif_on) {
			
		}
		if(interactif_on=="ARN") {
			arn()
		}
		
	}
	
	function zoom() {
		document.getElementById("div_zoom").style.zIndex="4"
		t_zoom=setInterval(afficherZOOM,50)
	}
	
	function afficherZOOM() {
		
		if (opaciteZOOM<1) {
			opaciteZOOM=opaciteZOOM+0.1
			document.getElementById("div_zoom").style.opacity=opaciteZOOM
		} else {
			clearInterval(t_zoom)
			ZOOM_affiche=true
		}
	}
	
	function cacherZOOM() {
		if (opaciteZOOM>0) {
			opaciteZOOM=opaciteZOOM-0.1
			document.getElementById("div_zoom").style.opacity=opaciteZOOM
		} else {
			document.getElementById("div_zoom").style.zIndex="-2"
			clearInterval(t_zoom)
			ZOOM_affiche=false
		}
	}
	
	
	/*Animations relatives à la div organites*/
	function afficherDIV() {
		if (opaciteDIV<1) {
			opaciteDIV=opaciteDIV+0.1
			document.getElementById("div_organite").style.opacity=opaciteDIV
		} else {
			clearInterval(t_div)
		}
	}
	
	
	function cacherDIV() {
	
		/*Instructions relatives à l'ADN*/
		interactif_on=false
		noyauSel = false
		document.getElementById("div_ADN").style.zIndex="-2"
		document.getElementById("img_ADN").src=""
		
		if (opaciteDIV>0) {
			opaciteDIV=opaciteDIV-0.1
			document.getElementById("div_organite").style.opacity=opaciteDIV
		} else {
			document.getElementById("div_organite").style.zIndex="-1"
			clearInterval(t_div)
		}
	}
	
	function afficherInfosOrganite(nomOrg) {
		document.getElementById("div_infos").innerHTML=nomOrg
	}
	
	/*Animation d'intro
	function intro() {
		if (opaciteDIV<1) {
			opaciteDIV=opaciteDIV+0.1
			document.getElementById("div_body").style.opacity=opaciteDIV
		} else {
			opaciteDIV=0
			clearInterval(t_div)
		}
	}*/
	
	/*Affichage de l'aura au passage de la souris*/
	function aura(txt) {
		document.getElementById("div_aura").style.zIndex="2"
		document.getElementById("img_aura").src="images/"+txt+"_aura.png"
		t_aura=setInterval(animAura,tempsAnim)
	}
	
	function animAura(){
		if (opaciteAura<1) {
			opaciteAura=opaciteAura+0.1
			document.getElementById("div_aura").style.opacity=opaciteAura
		} else {
			opaciteAura=0
			clearInterval(t_aura)
		}
	}
	
	function auraOut(){
		clearInterval(t_aura)
		document.getElementById("div_aura").style.opacity=0
		document.getElementById("div_aura").style.zIndex="-1"
		document.getElementById("img_aura").src=""
	}
	
