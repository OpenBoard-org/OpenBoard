
// -------------------- Etude de fonctions --------------------

var etudeX = new Array()
var etudeY = new Array()
var etudeD = new Array()
var limGauche = new Array()
var limDroite = new Array()
var listeIndefini = new Array()
var listeZeros = new Array()
var intervalEtudeGauche = -50
var intervalEtudeDroite = 50
var precisionEtude = 0.001

function etudier(fct){
	if(check(fct)){
		prepareEtude(fct)
		pariteFct()
		signeFct(fct)
        asymptotes()
        courbure()
		ensembleDefinition()
	}
}

// Préparation à l'étude de fonction
function prepareEtude(fct){
	listeIndefini = new Array()
    var f = function(x){
        return eval(fct)
    }
	// Calcul de tous les points de la fonction
	var i = 0
	for(var x=intervalEtudeGauche; x<=intervalEtudeDroite; x+=precisionEtude){
		etudeX[i] = x
		etudeY[i] = Math.round(f(x)*10000)/10000
		etudeD[i] = Math.round(1000000*(f(x+0.0001)-f(x-0.0001))/(2*0.0001))/1000000
		i++
	}
	// Calcul des extrémités de la fonction
	var x = -100000
	limGauche[0] = f(x)
	x = -10000
	limGauche[1] = f(x)
	x = 100000
	limDroite[0] = f(x)
	x = 10000
	limDroite[1] = f(x)
}

// Recherche de l'ensemble de définition
function ensembleDefinition(){
	var fctDefinie = false
	var dejaDefinie = false
	var EDF = ""
	if(!isNaN(limGauche[0])){
		fctDefinie = true
		dejaDefinie = true
		EDF += "]-∞;"
	}
	if(!isNaN(limGauche[1]) && !fctDefinie){
		fctDefinie = true
		dejaDefinie = true
		EDF += "]-∞;"
	}
	for(var i=0; i<etudeX.length; i++){
		if(fctDefinie && isNaN(etudeY[i])){
			fctDefinie = false
			EDF += (parseInt(etudeX[i]/precisionEtude)*precisionEtude)+"["
		}
		else if(!fctDefinie && !isNaN(etudeY[i])){
			fctDefinie = true
			if(dejaDefinie){
				EDF += " u "
			}
			EDF += "["+(parseInt(etudeX[i]/precisionEtude)*precisionEtude)+";"
			dejaDefinie = true
		}
	}
	if(fctDefinie && !isNaN(limDroite[1])){
		fctDefinie = false
		EDF += "+∞["
	}
	if(fctDefinie && !isNaN(limDroite[0])){
		fctDefinie = false
		EDF += "+∞["
	}
	for(var i=0; i<listeIndefini.length; i++){
		if(i==0){
			EDF += " - {"
		}
		EDF += listeIndefini[i]
		if(i==listeIndefini.length-1){
			EDF += "}"
		}
		else
			EDF += ";"
	}
	document.getElementById('etudeEDF').innerHTML = EDF
}

// Etude de la parité
function pariteFct(){
	var posZero
	var paire = true
	var impaire = true
	var positif, negatif
	var precisionParite = 1000
	for(var i=0;i<etudeX.length;i++){
		if(Math.round(etudeX[i]*precisionParite)/precisionParite==0){
			posZero = i
		}
	}
	for(var i=1; i<etudeX.length/2; i++){
		if(etudeY[posZero+i]>0){
			positif = Math.floor(etudeY[posZero+i])
		}
		else{
			positif = Math.ceil(etudeY[posZero+i])
		}
		if(etudeY[posZero-i]>0){
			negatif = Math.floor(etudeY[posZero-i])
		}
		else{
			negatif = Math.ceil(etudeY[posZero-i])
		}

		if(positif<25000 && positif>-25000){
			if(positif != negatif){
				paire = false
				//alert(i+" ; "+posZero+" ; "+etudeX.length+" ; "+positif+" ; "+negatif)
			}
			if(positif != (-negatif)){
				impaire = false
			}
		}
	}
	if(paire){
		document.getElementById("etudeParite").innerHTML = "paire"
	}
	else if(impaire){
		document.getElementById("etudeParite").innerHTML = "impaire"
	}
	else{
		document.getElementById("etudeParite").innerHTML = "aucune"
	}
}

function zeroFct(){
	listeZeros = new Array()
	var listeZerosArr = new Array()
	var precZeros = 5
	for(var i=0; i<etudeX.length; i++){
		if(Math.round(etudeY[i]*1000)/1000==0){
			var nouveauZero = Math.round(etudeX[i]*precZeros)/precZeros
			if(listeZerosArr[listeZerosArr.length-1]!=nouveauZero){
				listeZeros.push(Math.round(etudeX[i]*10)/10)
				listeZerosArr.push(nouveauZero)
			}
		}
	}
}

function signeFct(fct){
	var signe = new Array()
	var signePlus = new Array()
	var signeMoins = new Array()
	var dernier = 0
	if(limGauche[0]>0){
		signe[0] = "+"
		signePlus[0] = limGauche[1]
		signeMoins[0] = limGauche[0]
	}
	else{
		signe[0] = "-"
		signePlus[0] = limGauche[1]
		signeMoins[0] = limGauche[0]
	}
	for(var i=0; i<etudeY.length; i++){
		if(signe[signe.length-1]=="+"){
			if(etudeY[i]<0){
				signe.push("-")
				signePlus.push(etudeX[i])
				signeMoins.push(etudeX[dernier])
			}
			else if(etudeY[i]>0){
				dernier = i
			}
		}
		if(signe[signe.length-1]=="-"){
			if(etudeY[i]>0){
				signe.push("+")
				signePlus.push(etudeX[i])
				signeMoins.push(etudeX[dernier])
				//alert(signe+" ; "+signePlus+'\n'+etudeY[i+1]+" ; "+etudeY[i]+" ; "+etudeY[i-1]+" ; "+etudeY[i-2]+'\n'+etudeX[i+1]+" ; "+etudeX[i]+" ; "+etudeX[i-1]+" ; "+etudeX[i-2])
			}
			else if(etudeY[i]<0){
				dernier = i
			}
		}
	}
	var valeurPlus, valeurMoins, millieu, precisionZero
	listeZeros = new Array()
	for(var i=1; i<signePlus.length; i++){
		valeurPlus = signePlus[i]
		valeurMoins = signeMoins[i]
		for(var k=0;k<10000;k++){
			millieu = (valeurPlus + valeurMoins)/2
			x = valeurPlus
			if(eval(fct)>0){
				x = millieu
				if(eval(fct)>0){
					valeurPlus=millieu
				}
				else if(eval(fct)<0){
					valeurMoins=millieu
				}
				else{
					break
				}
			}
			else if(eval(fct)<0){
				x = millieu
				if(eval(fct)<0){
					valeurPlus=millieu
				}
				else if(eval(fct)>0){
					valeurMoins=millieu
				}
				else{
					break
				}
			}
			else{
				millieu = valeurPlus
				break
			}
			x = valeurPlus
			precisionZero = eval(fct)
			x = valeurMoins
			precisionZero -= eval(fct)
			if(precisionZero>0.0001){
				break
			}
		}
		//alert(signePlus[i]+" et "+signeMoins[i]+" -> "+millieu)
		listeZeros.push(Math.round(millieu*100)/100)
	}
	var texteZeros = ""
	if(listeZeros==""){
		zeroFct()
		texteZeros = "~ "
	}
	var texte = "Signe : "
	var aSupprimer = new Array()
	for(var i=0; i<signe.length; i++){
		texte += "<td>"+signe[i]+"</td>"
		if(listeZeros[i]!=undefined){
			x = listeZeros[i]
			if(isFinite(eval(fct))){
				texte += "<td class='border'>"+listeZeros[i]+"</td>"
			}
			else{
				texte += "<td class='border infini'>"+listeZeros[i]+"</td>"
				listeIndefini.push(listeZeros[i])
				aSupprimer.push(i)
			}
		}
	}
	document.getElementById("etudeSigne").innerHTML = texte
	for(var i=0; i<aSupprimer.length; i++){
		listeZeros.splice((aSupprimer[i]-i), 1)
	}
	if(listeZeros==""){
		texteZeros = "aucuns"
	}
	document.getElementById("etudeZeros").innerHTML = texteZeros+listeZeros
}

function asymptotes(){
	// Horizontales
	if(Math.abs(limGauche[0])<1000){
		var limRound = Math.round(limGauche[0]*100)/100
		if(limGauche[0]<limRound){
			document.getElementById("etudeAHG").innerHTML = "y = "+limRound +" <span class='texteSecondaire'>(courbe au-dessous de l'AH)</span>"
		}
		else if(limGauche[0]>limRound){
			document.getElementById("etudeAHG").innerHTML = "y = "+limRound +" <span class='texteSecondaire'>(courbe au-dessus de l'AH)</span>"
		}
		else{
			document.getElementById("etudeAHG").innerHTML = "y = "+limRound
		}
	}
	else{
		document.getElementById("etudeAHG").innerHTML = "aucune"
	}
	if(Math.abs(limDroite[0])<1000){
		var limRound = Math.round(limDroite[0]*100)/100
		if(limDroite[0]<limRound){
			document.getElementById("etudeAHD").innerHTML =  "y = "+limRound +" <span class='texteSecondaire'>(courbe au-dessous de l'AH)</span>"
		}
		else if(limDroite[0]>limRound){
			document.getElementById("etudeAHD").innerHTML =  "y = "+limRound +" <span class='texteSecondaire'>(courbe au-dessus de l'AH)</span>"
		}
		else{
			document.getElementById("etudeAHD").innerHTML =  "y = "+limRound
		}
	}
	else{
		document.getElementById("etudeAHD").innerHTML = "aucune"
	}
	// Verticales
	var texteAV = ""
	for(var i=0; i<listeIndefini.length; i++){
		texteAV += "x = "+listeIndefini[i]
		if(listeIndefini[i+1]!=undefined){
			texteAV += " ; "
		}
	}
	if(texteAV==""){
		texteAV = "aucune <br/>"
	}
	document.getElementById("etudeAV").innerHTML = texteAV
}

function courbure(){
    var texteMin = "";
    var texteMax = "";
    var texteI = "";
    var nbrI = 0;
    var i;
    for(i=0; i<etudeD.length; i++){
        if(etudeD[i]==0){
            if(etudeD[i-1]<0 && etudeD[i+1]>0){
                texteMin += "<br/>Min("+Math.round(etudeX[i]*10000)/10000+";"+etudeY[i]+")";
            }
            else if(etudeD[i-1]>0 && etudeD[i+1]<0){
                texteMax += "<br/>Max("+Math.round(etudeX[i]*10000)/10000+";"+etudeY[i]+")";
            }
            else{
                nbrI++;
                texteI += '<br/>I<span style="vertical-align:sub;font-size:50%;">'+nbrI+"</span>("+Math.round(etudeX[i]*10000)/10000+";"+etudeY[i]+")";
            }
        }
    }
    if(texteMin==""){texteMin = "<br/>Aucun Minimum";}
    if(texteMax==""){texteMax = "<br/>Aucun Maximum";}
    if(texteI==""){texteI = "<br/>Aucun I";}
    document.getElementById("etudeMin").innerHTML = texteMin;
    document.getElementById("etudeMax").innerHTML = texteMax;
    document.getElementById("etudeI").innerHTML = texteI;
}