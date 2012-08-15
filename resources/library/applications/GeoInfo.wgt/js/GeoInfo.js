/*
  Script utilisé par GeoInfo.
  
  Copyright (C) 2010 Baptiste Sottas — Tous droits réservés.
  
  Ce programme est un logiciel libre ; vous pouvez le redistribuer ou le
  modifier suivant les termes de la “GNU General Public License” telle que
  publiée par la Free Software Foundation : soit la version 3 de cette
  licence, soit (à votre gré) toute version ultérieure.
  
  Ce programme est distribué dans l’espoir qu’il vous sera utile, mais SANS
  AUCUNE GARANTIE : sans même la garantie implicite de COMMERCIALISABILITÉ
  ni d’ADÉQUATION À UN OBJECTIF PARTICULIER. Consultez la Licence Générale
  Publique GNU pour plus de détails.
  
  Vous devriez avoir reçu une copie de la Licence Générale Publique GNU avec
  ce programme ; si ce n’est pas le cas, consultez :
  http://www.gnu.org/licenses
  
  baptiste.sottas@bluewin.ch
*/

// variables diverses
var coords = ""
var opacity = 0
var divInfo = true;
var imageAfrique = "<img src='images/Cartes/Afrique.bmp' usemap='#afrique'/>";
var imageEurasie = "<img src='images/Cartes/Eurasie.bmp' usemap='#eurasie'/>";
var imageOceanie = "<img src='images/Cartes/Oceanie.bmp' usemap='#oceanie'/>";
var imageAmeriqueNord = "<img src='images/Cartes/AmeriqueNord.bmp' usemap='#ameriqueNord'/>";
var imageAmeriqueSud = "<img src='images/Cartes/AmeriqueSud.bmp' usemap='#ameriqueSud'/>";
var imageEurope = "<img src='images/Cartes/EuropeOuest.bmp' usemap='#europe'/>";
var imageEurope2 = "<img src='images/Cartes/Eurasie2.bmp' usemap='#eurasie'/>";
var imageAsie2 = "<img src='images/Cartes/EuropeOuestAsie.bmp' usemap='#europe'/>";

var lang = ""; //locale language
if(window.sankore){
    try{
        lang = sankore.locale().substr(0,2);
        sankoreLang[lang].europe;
    } catch(e){
        lang = "en";
    }
} else 
    lang = "en";

// adresse du serveur
var url = "http://bapsot.dyndns.org/GeoInfo-GeoInfoServer/GeoInfoService";

// variables pour la gestion de la comparaison des pays
var currentInfo = 0;
var compareActive = false;

// permet d'effacer le contenu des divs 2 à 5 si on ne désire pas comparer
function vide() 
{
    // regarder si la case est cochée
    compareActive = document.getElementById('compare').checked;
	
    // si elle ne l'est pas vider les divs 2 à 5
    if (!compareActive)
    {
        for (i = 2; i < 6; i++)
        {
            var element = document.getElementById("info" + i);
            element.innerHTML = "";
            element.style.display = "none";
        }
		
        // remettre la grandeur initiale à la première div
        document.getElementById("info1").style.width = 654 + 'px';
		
        // initialiser le compteur
        currentInfo = 1;
    }
}

// permet de créer et remplir les divs d'informations
function popupInfo(text) 
{
    // regarder si on désire comparer les pays entre eux
    compareActive = document.getElementById('compare').checked;

    // si oui mettre l'info dans une nouvelle div, autrement mettre dans la div 1
    if (compareActive)
        currentInfo += 1;
    else
        currentInfo = 1;
	
    // max 5 comparaisons possibles, si plus on réécrit dans la dernière
    if (currentInfo > 5)
        currentInfo = 5;

    // obtenir la div à modifier
    var info = document.getElementById("info" + currentInfo);
	
    // la rendre visible et mettre le texte
    info.style.display = "block";
    info.innerHTML = text;
	
    // met la bonne grandeur à chaque div
    for (i = 1; i <= currentInfo; i++)
        document.getElementById("info" + i).style.width = (654/currentInfo) + 'px';
}

// affiche le text de l'erreur en cas de problème
function erreur(texteErreur) 
{
    //alert(texteErreur);
    sankore.showMessage(texteErreur);
}

// permet d'appeller la fonction pour recevoir les statistiques du serveur	
function recevoirStats()
{
    var pl = new SOAPClientParameters();
	
    // appel de la fonction getInfos du serveur
    SOAPClient.invoke(url, "getInfos", pl, true, recevoirStatsReponse);
}

// permet de recevoir les statistiques du serveur et les afficher
function recevoirStatsReponse(resultat, soapResponse)
{
    var objDom = new XMLDoc(((new XMLSerializer()).serializeToString(soapResponse)), erreur);

    var objDomTree = objDom.docNode;
	
    // prepare le texte à afficher à partir de la réponse
    var stats = ("Nombre d'accès total: " + objDomTree.getElements("env:Body")[0].getElements("ser:getInfosResponse")[0].getElements("GeoStats")[0].getElements("accesTotal")[0].getText()
        + "\n"
        + "Dernier accès le: " + objDomTree.getElements("env:Body")[0].getElements("ser:getInfosResponse")[0].getElements("GeoStats")[0].getElements("dernierAcces")[0].getText()
        + "\n"
        + "Adresse pour des questions/remarques: " + objDomTree.getElements("env:Body")[0].getElements("ser:getInfosResponse")[0].getElements("GeoStats")[0].getElements("email")[0].getText()
        + "\n"
        + "Microinformations sous: " + objDomTree.getElements("env:Body")[0].getElements("ser:getInfosResponse")[0].getElements("GeoStats")[0].getElements("twitter")[0].getText()
        + "\n"
        + "Version de l'application: " + objDomTree.getElements("env:Body")[0].getElements("ser:getInfosResponse")[0].getElements("GeoStats")[0].getElements("version")[0].getText());

    //alert(stats);
    sankore.showMessage(stats);
}

// permet d'appeller la fonction pour recevoir les informations du pays
function recevoirPays(id)
{
    // prépare le paramètre avec l'id du pays
    var pl = new SOAPClientParameters();
    pl.add("id",id);
	
    // appel de la fonction find du serveur
    SOAPClient.invoke(url, "find", pl, true, recevoirPaysReponse);
}

// permet de recevoir les informations du pays du serveur et les afficher
function recevoirPaysReponse(resultat, soapResponse)
{
    var objDom = new XMLDoc(((new XMLSerializer()).serializeToString(soapResponse)), erreur);

    var objDomTree = objDom.docNode;
	
    // prepare le texte à afficher à partir de la réponse
    var informations = ("<b>Pays:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("pays")[0].getText()
        + "<br/>"
        + "<b>Nom:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("nomPays")[0].getText()
        + "<br/>"
        + "<b>Capitale:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("capitale")[0].getText()
        + "<br/>"
        + "<b>Forme:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("forme")[0].getText()
        + "<br/>"
        + "<b>Adjectif:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("adjectif")[0].getText()
        + "<br/>"
        + "<b>Monnaie ISO:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("monnaieIso")[0].getText()
        + "<br/>"
        + "<b>Monnaie:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("monnaie")[0].getText()
        + "<br/>"
        + "<b>Monnaie centime:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("monnaieSubDiv")[0].getText()
        + "<br/>"
        + "<b>Plus grande ville:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("plusGrandeVille")[0].getText()
        + "<br/>"
        + "<b>Nbr habitants:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("nbrHabitant")[0].getText()
        + "<br/>"
        + "<b>Langues officielles:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("langueOfficielle")[0].getText()
        + "<br/>"
        + "<b>Système gouvernemental:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("politique")[0].getText()
        + "<br/>"
        + "<b>Remarques:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("remarques")[0].getText()
        + "<br/>"
        + "<b>Devise:</b> " + objDomTree.getElements("env:Body")[0].getElements("ser:findResponse")[0].getElements("GeoData")[0].getElements("devise")[0].getText());

    // appelle la fonction popupInfo qui affiche les informations dans les divs
    popupInfo(informations);
}
	
// permet l'affichage de la description lors du passage de la souris
function writeText(txt)
{
    switch(txt){
        case "North America":
            txt = sankoreLang[lang].north_america;
            break;
        case "Eurasia":
            txt = sankoreLang[lang].eurasia;
            break;
        case "South America":
            txt = sankoreLang[lang].south_america;
            break;
        case "Africa":
            txt = sankoreLang[lang].africa;
            break;
        case "Oceania":
            txt = sankoreLang[lang].oceania;
            break;
        default:
            break;
    }
    document.getElementById("description").innerHTML = txt;
}

// permet l'effaçage de la description lors du passage de la souris
function effaceTexte() 
{
    document.getElementById("description").innerHTML = "";
}

// apparition de la div contenant une carte d'un continent + carte de retour
function apparition(carte) 
{    
    document.getElementById(carte).style.display = 'block'
    i = setInterval('fondu("'+carte+'")', 25)
	
    document.getElementById("retour").style.display = 'block'
    document.getElementById("retour").style.opacity = 1
}

// permet de recharger la page
function disparition() 
{
    window.location.reload()
}

// permet de faire apparaître la carte d'un continent en fondu
function fondu(carte)
{
    opacity = opacity + 0.05
    document.getElementById(carte).style.opacity = opacity;
	
    if (opacity > 1) 
    {
        clearInterval(i)
    }
}

// permet de faire apparaître les infos du pays dans la petite fenêtre
function afficheInfoPays(pays) 
{
    $("#" + pays + " .nomPays").html(sankoreLang[lang][pays].country);
    $("#" + pays + " .capitalePays").html(sankoreLang[lang].capital + ": " + sankoreLang[lang][pays].city);
    document.getElementById("infoPays").innerHTML = "<div id='customCursor' class='customCursor'></div>" + document.getElementById(pays).innerHTML;
}

// permet de cacher la petite fenêtre
function cacherInfoPays()
{
    document.getElementById("infoPays").style.display = 'none';
}

// permet à la petite fenêtre de suivre la souris
function deplacerInfo(event)
{
    document.getElementById("infoPays").style.display = 'block';    
    document.getElementById("infoPays").style.top = event.clientY;
    document.getElementById("infoPays").style.left = event.clientX+40;
}
