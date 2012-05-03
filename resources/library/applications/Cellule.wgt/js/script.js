	
var opaciteDIV=0 //opacitй de la div organites
var opaciteINFOS=0 //opacitй de la div informations
var INFOS_affiche=false //bool pour la fonction clicINFOS()
var t_div //pour les setinterval d'animation
var t_inf
var tempsAnim=25 //durйe en milisecondes des animations de fondu
var t_aura//pour l'animation de l'aura au passage de la souris
var opaciteAura=0// idem que t_aura
var noyauSel = false //pour pouvoir accиder а l'ADN depuis le noyau
var loupe_on = false // permet d'accйder aux zooms de l'ADN et de l'ARN
var interactif_on = false // permet d'accйder aux parties interactives s'il y en a
var elt_a_zoomer = "" //pour savoir s'il faut afficher adn_zoom.jpg ou arn_zoom.jpg
var ZOOM_affiche=false //pour savoir si la div de zoom est affichйes ou pas
var opaciteZOOM = 0
var t_zoom
var loaded = true
var click_INFO = false;
var click_ZOOM = false;
var click_EXIT = true;
var click_Cacher = true;
	
function vesicule() {
    loupe_on = false
    //console.log("vesicule");
    afficherInfosOrganite(txt_vesicule)
    document.getElementById("img_organite").src="images/vesic_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function lysosome() {
    loupe_on = false
    //console.log("lysosome");
    afficherInfosOrganite(txt_lysosome)
    document.getElementById("img_organite").src="images/lys_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function mitoch() {
    loupe_on = false
    //console.log("mitoch");
    afficherInfosOrganite(txt_mitoch)
    document.getElementById("img_organite").src="images/mitoch_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function golgi () {
    loupe_on = false
    //console.log("golgi");
    afficherInfosOrganite(txt_golgi)
    document.getElementById("img_organite").src="images/golgi_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function rer() {
    loupe_on = false
    //console.log("rer");
    afficherInfosOrganite(txt_rer)
    document.getElementById("img_organite").src="images/rer_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function noy() {
    loupe_on = false
    //console.log("noy");
    afficherInfosOrganite(txt_noy)
    document.getElementById("img_organite").src="images/noy_in.jpg"
    noyauSel = true
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function rel() {
    loupe_on = false
    //console.log("rel");
    afficherInfosOrganite(txt_rel)
    document.getElementById("img_organite").src="images/rel_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function adn() {
    //console.log("adn");
    if(noyauSel) {
        elt_a_zoomer="ADN"
        loupe_on = true
        noyauSel = false
			
        afficherInfosOrganite(txt_adn)
        document.getElementById("div_zoom").innerHTML='<img src="images/'+elt_a_zoomer+'_zoom.jpg" alt="" />'
			
        /*Uniboard 4 ne supportant pas bien les gifs...*/
        if(window.sankore){
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
    //console.log("rib");
    afficherInfosOrganite(txt_rib)
    document.getElementById("img_organite").src="images/rib_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function centriole(){
    loupe_on = false
    //console.log("centriole");
    afficherInfosOrganite(txt_centr)
    document.getElementById("img_organite").src="images/centriole_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
function arn(){
    elt_a_zoomer="ARN"
    loupe_on = true
    //console.log("arn");
    afficherInfosOrganite(txt_arn)
    document.getElementById("div_zoom").innerHTML='<img src="images/'+elt_a_zoomer+'_zoom.jpg" alt="" />'
		
    document.getElementById("img_organite").src="images/ARN_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
}
	
function nucl(){
    loupe_on = false
    //console.log(" nucl");
    afficherInfosOrganite(txt_nucl)
    document.getElementById("img_organite").src="images/nucl_in.jpg"
    document.getElementById("div_organite").style.zIndex="1"
    document.getElementById("div_aura").style.zIndex="-1"
    t_div=setInterval(afficherDIV,tempsAnim)
}
	
/*Fonctions relatives а la div d'infos*/
function infos() {
    //console.log(" infos");
    if(!INFOS_affiche && !click_INFO){
        document.getElementById("div_infos").style.zIndex="4"
        t_inf=setInterval(afficherINFOS,50)
        click_INFO = true;
    }
}
	
function afficherINFOS() {
    //console.log("afficherINFOS");
    if (opaciteINFOS<1) {
        opaciteINFOS=opaciteINFOS+0.1
        document.getElementById("div_infos").style.opacity=opaciteINFOS
    } else {
        clearInterval(t_inf);
        INFOS_affiche=true			
    }
		
}
	
function clicINFOS() {
		
    if (INFOS_affiche && click_INFO) {
        //console.log("INFOS_affiche");
        t_inf=setInterval(cacherINFOS,50)
        click_INFO = false;
    }
		
    if (ZOOM_affiche && click_ZOOM) {
        //console.log("ZOOM_affiche");
        t_zoom=setInterval(cacherZOOM,50)
        click_ZOOM = false;
    }
}
	
function cacherINFOS() {
    //console.log("cacherINFOS");
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
    //console.log("loupe_interact");
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
    //console.log("zoom");
    document.getElementById("div_zoom").style.zIndex="4"
    t_zoom=setInterval(afficherZOOM,50);
    ZOOM_affiche = true;
}
	
function afficherZOOM() {
    //console.log("afficherZOOM");
    if (opaciteZOOM<1) {
        opaciteZOOM=opaciteZOOM+0.1
        document.getElementById("div_zoom").style.opacity=opaciteZOOM
    } else {
        clearInterval(t_zoom)
        ZOOM_affiche=true
        click_ZOOM = true;
    }
}
	
function cacherZOOM() {
    //console.log("cacherZOOM");
    if (opaciteZOOM>0) {
        opaciteZOOM=opaciteZOOM-0.1
        document.getElementById("div_zoom").style.opacity=opaciteZOOM
    } else {
        document.getElementById("div_zoom").style.zIndex="-2"
        clearInterval(t_zoom)
        ZOOM_affiche=false
    }
}
	
	
/*Animations relatives а la div organites*/
function afficherDIV() {
    if(click_EXIT){
        //console.log("afficherDIV");
        if (opaciteDIV<1) {
            opaciteDIV=opaciteDIV+0.1
            document.getElementById("div_organite").style.opacity=opaciteDIV
        } else {
            clearInterval(t_div)
            click_EXIT = false;
            click_Cacher = true;
        }			
    }
}
	
	
function cacherDIV() {

    if(!click_EXIT){
        //console.log("cacherDIV");
        interactif_on=false
        noyauSel = false
        document.getElementById("div_ADN").style.zIndex="-2"
        document.getElementById("img_ADN").src=""
			
        if (opaciteDIV>0) {
            opaciteDIV=opaciteDIV-0.1
            document.getElementById("div_organite").style.opacity=opaciteDIV
        } else {
            document.getElementById("div_organite").style.zIndex="-1";
            clearInterval(t_div)
            click_EXIT = true;
        }			
    }
}
	
function cacherDivFunction(){
    if(!click_EXIT && click_Cacher){
        t_div=setInterval(cacherDIV,50);
        click_Cacher = false;
    }
}
	
	
function afficherInfosOrganite(nomOrg) {
    //console.log("afficherInfosOrganite");
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
    //console.log("aura");
    document.getElementById("div_aura").style.zIndex="2"
    document.getElementById("img_aura").src="images/"+txt+"_aura.png"
    t_aura=setInterval(animAura,tempsAnim)
}
	
function animAura(){
    //console.log("animAura");
    if (opaciteAura<1) {
        opaciteAura=opaciteAura+0.1
        document.getElementById("div_aura").style.opacity=opaciteAura
    } else {
        opaciteAura=0
        clearInterval(t_aura)
    }
}
	
function auraOut(){
    //console.log("auraOut");
    clearInterval(t_aura)
    document.getElementById("div_aura").style.opacity=0
    document.getElementById("div_aura").style.zIndex="-1"
    document.getElementById("img_aura").src=""
}
	
