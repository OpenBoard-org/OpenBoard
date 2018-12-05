
// -------------------- Etudes de fonctions --------------------

function consoleInfos(){
	var table = [];
	for(var i in arguments){
		table.push(arguments[i]);
	}
	document.getElementById("infos").innerHTML += table +"<br/>";
}

Array.prototype.first = function(){
	return this[0];
}
Array.prototype.last = function(){
	return this[this.length-1];
}
Array.prototype.removeDoubles = function(){
	// Sort numerically
	this.sort(function(nbr1,nbr2){return nbr1-nbr2});
	// Remove doubles
	for(var i=1; i<this.length; i++){
		if(this[i] == this[i-1]){
			this.splice(i, 1);
			i--;
		}
	}
}

var etude = {
	f : null,
	fct : "",
	
	xGauche : -10,
	xDroite : 10,
	precision: 0.001,
	
	listeX : [],
	listeY : [],
	listeD : [],
	listeD2 : [],
	
	listeIndefini : [],
	listeZeros : [],
	
	etudier : function(txtFct){
// 		document.getElementById("infos").innerHTML = "";
		this.fct = fct.remplacer(fct.verifier(txtFct));
// 		this.fct = txtFct;
		this.f = new Function("x", "return "+this.fct);
		
		this.listeX = [];
		this.listeY = [];
		this.listeD = [];
		this.listeD2 = [];
		
		document.getElementById("etudeFdeX").innerHTML = ""+this.fct;
		
		this.preparer();
		this.ensembleDefNum();
		this.pariteNum();
		this.zerosNum();
		this.signeNum();
		this.asymptotesNum();
		this.extremumsNum();
		this.pointI();
	},
	
	round : function(nbr){
		return Math.round(nbr*1000)/1000;
	},
	
	preparer : function(){
		var f = this.f;
		var a, b;
		for(var x=this.xGauche; x<=this.xDroite; x+=this.precision){
			x = this.round(x);
			this.listeX.push(x);
			this.listeY.push(f(x));
			a = (f(x+0.0001)-f(x))/0.0001;
			b = (f(x)-f(x-0.0001))/0.0001;
			this.listeD.push((a+b)/2);
			this.listeD2.push((b-a)/0.0001);
// 			this.listeD2.push(Math.round((b-a)*10000000)/100000000000);
			if(x==0){
				this.posZero = this.listeX.length-1;
			}
// 			consoleInfos(x, f(x));
		}
	},
	
	ensembleDefNum : function(){
		var EDF = "";
		var definie = false;
		var listeIndefini = [];
		if(isFinite(this.listeY.first())){
			EDF += "]-∞;"
			definie = true;
		}
		
		for(var i=0; i<this.listeY.length; i++){
// 			consoleInfos(i, definie, this.listeX[i])
			if(definie){
				if(!isFinite(this.listeY[i]) && isFinite(this.listeY[i-1])){
// 					consoleInfos(this.listeY[(i)], !isFinite(this.listeY[i+1]), definie)
					if(!isFinite(this.listeY[i+1])){
						definie = false;
						// Définition de [ ou ] en fonction de l'entier le plus proche
						if(Math.round(this.listeX[i-1]) == this.listeX[i-1]){
							EDF += this.round(this.listeX[i-1])+"]";
						}
						else{
							if(Math.round(this.listeX[i]) != this.listeX[i]){
// 								consoleInfos(Math.round(this.listeX[i-1]*10)/10, this.listeX[i-1]);
								if(Math.round(this.listeX[i-1]*10)/10 == this.listeX[i-1]){
									EDF += this.round(this.listeX[i-1])+"]";
								}
								else{
									EDF += this.round(this.listeX[i])+"[";
								}
							}
							else{
								EDF += this.round(this.listeX[i])+"[";
							}
						}
					}
					else{
						listeIndefini.push(this.listeX[i]);
					}
				}
			}
			else{
				if(isFinite(this.listeY[i])){
					definie = true;
					if(EDF != ""){
						EDF += " u ";
					}
					// Définition de [ ou ] en fonction de l'entier le plus proche
					if(Math.round(this.listeX[i-1]) == this.listeX[i-1]){
						EDF += "]"+this.round(this.listeX[i-1])+";";
					}
					else{
						if(Math.round(this.listeX[i]) != this.listeX[i]){
							if(Math.round(this.listeX[i-1]*10)/10 == this.listeX[i-1]){
								EDF += "]"+this.round(this.listeX[i-1])+";";
							}
							else{
								EDF += "["+this.round(this.listeX[i])+";";
							}
						}
						else{
							EDF += "["+this.round(this.listeX[i])+";";
						}
					}
				}
			}
		}
		
		if(definie && isFinite(this.listeY.last())){
			EDF += "+∞["
			definie = true;
		}
		
		for(var i=0; i<listeIndefini.length; i++){
			if(i==0){
				EDF += " - {"
			}
			EDF += listeIndefini[i]
			if(i==listeIndefini.length-1){
				EDF += "}"
			}
			else{
				EDF += ";"
			}
		}
		
		this.listeIndefini = listeIndefini;
		document.getElementById("etudeEDF").textContent = EDF;
// 		consoleInfos("EDF = "+EDF);
	},
	
	pariteNum : function(){
		var paire = true;
		var impaire = true;
		var yPositif, yNegatif;
		for(var i=1; i<this.listeX.length/2; i++){
			yPositif = this.listeY[this.posZero+i];
			yNegatif = this.listeY[this.posZero-i];
			if(yPositif<100000 && yPositif>-100000){
				if(yPositif != yNegatif){
					paire = false
				}
				if(yPositif != (-yNegatif)){
					impaire = false
				}
			}
		}
		
		if(paire){
			var text = "even";
			try{
				text = languages.getText("even");
			}
			catch(e){}
			document.getElementById("etudeParite").textContent = text;
		}
		else if(impaire){
			var text = "odd";
			try{
				text = languages.getText("odd");
			}
			catch(e){}
			document.getElementById("etudeParite").textContent = text;
		}
		else{
			var text = "odd";
			try{
				text = languages.getText("none_f");
			}
			catch(e){}
			document.getElementById("etudeParite").textContent = text;
		}
// 		consoleInfos("paire = "+paire);
// 		consoleInfos("impaire = "+impaire);
	},
	
	zerosNum : function(){
		var f = this.f;
		// Rechercher des zéros dans les points calculés
		var listeZerosA = [];
		for(var i=0; i<this.listeX.length; i++){
			if(this.listeY[i] == 0){
				listeZerosA.push(this.listeX[i]);
			}
		}
// 		consoleInfos("zérosA : "+listeZerosA);
		
		// Recherche des zéros en prenant 2 points qui ne sont pas du même côté de l'axe
		var listeZerosB = [];
		var a, b;
		for(var i=1; i<this.listeX.length-1; i++){
			a = sign(this.listeY[i-1]);
			b = sign(this.listeY[i+1]);
			if(a!=b && a!=undefined && b!=undefined){
// 				consoleInfos(this.listeX[i]);
				var zeros = [];
				var check = false;
				for(var x=this.listeX[i-1]; x<=this.listeX[i]; x+=this.precision/100){
// 					consoleInfos(f(x), this.round(f(x)));
					if(this.round(f(x)) == 0){
						check = true;
						zeros.push(this.round(x));
// 						break;
					}
				}
				if(check){
					listeZerosB.push(zeros[Math.floor(zeros.length/2)]);
				}
			}
		}
// 		consoleInfos("zérosB : "+listeZerosB);
		
		// Résultat des deux méthodes
		var listeZeros = listeZerosA.concat(listeZerosB);
		listeZeros.removeDoubles();
		this.listeZeros = listeZeros;
		if(this.listeZeros.length == 0){
			document.getElementById("etudeZeros").innerHTML = "Ø";
		}
		else{
			document.getElementById("etudeZeros").innerHTML = "{"+listeZeros+"}";
		}
// 		consoleInfos("zéros : "+listeZeros);
	},
	
	signeNum : function(){
		var f = this.f;
		var txtSigne = "";
		var listeNbr = [];
		// Remplire la liste des zéros et points indéf.
		// true pour défini, false pour indéfini
		// liste[0] <= point  ;  liste[1] <= true/false
		for(var i=0; i<this.listeZeros.length; i++){
			listeNbr.push([this.listeZeros[i], true]);
		}
		for(var i=0; i<this.listeIndefini.length; i++){
			listeNbr.push([this.listeIndefini[i], false]);
		}
// 		consoleInfos(listeNbr);
		listeNbr.sort(function(nbr1,nbr2){return nbr1[0]-nbr2[0]});
// 		consoleInfos(listeNbr);
		var dernierSigne;
		var a, b;
		switch(sign(this.listeY.first())){
			case -1:
				txtSigne += "<td>-</td>";
				dernierSigne = "-";
				break;
			case 1:
				txtSigne += "<td>+</td>";
				dernierSigne = "+";
				break;
		}
		for(var i=0; i<listeNbr.length; i++){
			a = sign(f(listeNbr[i][0]-this.precision));
			b = sign(f(listeNbr[i][0]+this.precision));
			switch(a){
				case -1:
					if(dernierSigne != "-"){
						txtSigne += "<td>-</td>";
					}
					break;
				case 1:
					if(dernierSigne != "+"){
						txtSigne += "<td>+</td>";
					}
					break;
			}
			if(listeNbr[i][1]){
				txtSigne += '<td class="bordure">'+listeNbr[i][0]+'</td>'
			}
			else{
				txtSigne += '<td class="bordure fondNoir">'+listeNbr[i][0]+'</td>'
			}
			switch(b){
				case -1:
					txtSigne += "<td>-</td>";
					dernierSigne = "-";
					break;
				case 1:
					txtSigne += "<td>+</td>";
					dernierSigne = "+";
					break;
			}
		}
		document.getElementById("etudeSigneRow").innerHTML = txtSigne;
// 		consoleInfos("<table><tr><td>Signe : </td>"+txtSigne+"</tr></table>");
	},
	
	asymptotesNum : function(){
		var f = this.f;
		// Verticales
		var texteA = "";
		for(var i=0; i<this.listeIndefini.length; i++){
			if(texteA == ""){
				texteA += "AV : " ;
			}
			if(i!=0){
				texteA += " ; ";
			}
			texteA += "x = "+this.listeIndefini[i];
		}
// 		consoleInfos("AV : "+texteA);
		
		// Horizontales et obliques
		var lim1, lim2, lim3, lim4, pente1, pente2;
		var val1 = 100000;
		var val2 = 10000;
		// Gauche
		lim1 = this.round(f(-val1));
		lim2 = this.round(f(-val2));
		lim3 = this.round(f(-val1+1));
		lim4 = this.round(f(-val2+1));
// 		consoleInfos(lim1, lim2, lim3, lim4)
		if(lim1==lim2){
// 			consoleInfos("AHG : y="+lim1);
			if(texteA!=""){
				texteA += "<br/>";
			}
			texteA += "AHG : y="+lim1;
		}
		else{
			pente1 = this.round(lim3-lim1);
			pente2 = this.round(lim4-lim2);
// 			consoleInfos(pente1, pente2);
			if(pente1 == pente2 && pente1!=0){
// 				consoleInfos("AOG : y="+pente1+"x+"+(lim1+pente1*val1));
				if(texteA!=""){
					texteA += "<br/>";
				}
 				texteA += "AOG : y="+pente1+"x+"+(lim1+pente1*val1);
			}
		}
		// Droite
		lim1 = this.round(f(val1));
		lim2 = this.round(f(val2));
		lim3 = this.round(f(val1+1));
		lim4 = this.round(f(val2+1));
// 		consoleInfos(lim1, lim2, lim3, lim4)
		if(lim1==lim2 && isFinite(lim1)){
// 			consoleInfos("AHD : y="+lim1);
			if(texteA!=""){
				texteA += "<br/>";
			}
			texteA += "AHD : y="+lim1;
		}
		else{
			pente1 = this.round(lim3-lim1);
			pente2 = this.round(lim4-lim2);
// 			consoleInfos(pente1, pente2);
			if(pente1 == pente2 && pente1!=0){
// 				consoleInfos("AOD : y="+pente1+"x+"+(lim1-pente1*val1));
				if(texteA!=""){
					texteA += "<br/>";
				}
				texteA += "AOD : y="+pente1+"x+"+(lim1-pente1*val1);
			}
		}
		
		if(texteA == ""){
			texteA = "none";
			try{
				texteA = languages.getText("none_f");
			}
			catch(e){}
		}
		document.getElementById("etudeA").innerHTML = texteA;
	},
	
	extremumsNum : function(){
		// Rechercher des zéros dans les points calculés
		var listeID = [];
		for(var i=1; i<this.listeX.length; i++){
			if(Math.round(this.listeD[i]*10000000)/10000000 == 0){
				listeID.push(i);
			}
		}
		
		// Recherche des zéros en prenant 2 points qui ne sont pas du même côté de l'axe
		var a, b;
		for(var i=1; i<this.listeX.length-1; i++){
			a = sign(this.listeD[i-1]);
			b = sign(this.listeD[i+1]);
			if(a!=b && a!=undefined && b!=undefined){
				listeID.push(i);
			}
		}
// 		consoleInfos(listeID);
		
		// Résultat des deux méthodes
		listeID.removeDoubles();
		var nbrI = 0;
		var txtMinMax = "";
		for(var j=0; j<listeID.length; j++){
			var i = listeID[j];
			var a = this.listeD[i-1];
			var b = this.listeD[i+1];
// 			consoleInfos(a, b);
			
            if(a<0 && b>0){
				if(txtMinMax != ""){
					txtMinMax += "<br/>";
				}
                txtMinMax += "Min("+this.listeX[i]+";"+this.round(this.listeY[i])+")";
            }
            else if(a>0 && b<0){
				if(txtMinMax != ""){
					txtMinMax += "<br/>";
				}
                txtMinMax += "Max("+this.listeX[i]+";"+this.round(this.listeY[i])+")";
            }
		}
// 		consoleInfos(txtMinMax);
		if(txtMinMax == ""){
			txtMinMax = "none";
			try{
				txtMinMax = languages.getText("none_m");
			}
			catch(e){}
		}
		document.getElementById("etudeMinMax").innerHTML = txtMinMax;
	},
	
	pointI : function(){
		// Rechercher des zéros dans les points calculés
		var listeID = [];
// 		for(var i=1; i<this.listeX.length; i++){
// 			if(Math.round(this.listeD2[i]*100000000)/100000000 == 0){
// 				listeID.push(i);
// // 				consoleInfos(this.listeD2[i-2], this.listeD2[i-1], this.listeD2[i], this.listeD2[i+1] ,this.listeD2[i+2]);
// 			}
// 		}
		
		// Recherche des zéros en prenant 2 points qui ne sont pas du même côté de l'axe
		var a, b;
		for(var i=1; i<this.listeX.length-1; i++){
			a = sign(this.listeD2[i-1]);
			b = sign(this.listeD2[i+1]);
			c1 = this.listeY[i-1];
			c2 = this.listeY[i];
			c3 = this.listeY[i+1];
// 			if(a!=b && a!=undefined && b!=undefined){
			if(a!=b && a!=undefined && b!=undefined && isFinite(c1) && isFinite(c2) && isFinite(c3)){
// 				consoleInfos(this.listeD2[i-1],this.listeD2[i+1]);
				listeID.push(i);
			}
		}
// 		consoleInfos(listeID);
		// Résultat des deux méthodes
// 		listeID.removeDoubles();
		var txtI = "";
		for(var j=0; j<listeID.length; j++){
			var i = listeID[j];
			if(txtI != ""){
				txtI += "<br/>";
			}
			txtI += 'I<span style="vertical-align:sub;font-size:50%;">'+(j+1)+"</span>("+this.listeX[i]+";"+this.round(this.listeY[i])+")";
		}
// 		consoleInfos(txtI);
		if(txtI == ""){
			txtI = "none";
			try{
				txtI = languages.getText("none_m");
			}
			catch(e){}
		}
		document.getElementById("etudeI").innerHTML = txtI;
	},
	
}
