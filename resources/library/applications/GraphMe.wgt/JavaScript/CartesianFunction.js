
function CartesianFunction(txtFct){

	this.setFct = function(txt){
		this.fct = txt;
		this.f = new Function("x", "y", "return "+this.fct);
	}
	
	if(txtFct){
		this.setFct(txtFct);
	}
	this.couleur = fct.couleur;
	this.width = document.getElementById("inputTaille").value;
	this.aire = document.getElementById("checkAire").checked ? true : false;
	this.derivee1 = document.getElementById("checkDerivee1").checked ? true : false;
	this.couleurDerivee1 = "rgba(255,128,0,1)";
	this.derivee2 = document.getElementById("checkDerivee2").checked ? true : false;
	this.couleurDerivee2 = "rgba(255,255,0,1)";
	this.primitive1 = document.getElementById("checkPrimitive1").checked ? true : false;
	this.couleurPrimitive1 = "rgba(255,50,0,1)";
	this.conditionInitiale = 0;
	this.style = document.getElementById("selectStyle").value;
	
	this.getX = function(x){
		return x;
	};
	
	this.getY = function(x){
		return this.f(x);
	};
	
	this.set = function(f){
		this.setFct(fct.remplacer(fct.verifier(f.fct)));
		this.couleur = f.couleur;
		this.couleurDerivee1 = f.couleurD1;
		this.couleurDerivee2 = f.couleurD2;
		this.couleurPrimitive1 = f.couleurP1;
		this.width = f.width;
		this.style = f.style;
		this.aire = f.aire;
		this.derivee1 = f.derivee1;
		this.derivee2 = f.derivee2;
		this.primitive1 = f.primitive1;
		this.conditionInitiale = f.conditionInitiale;
		return this;
	};
	
	this.get = function(){
		var f = {};
		f.type = "cartesian";
		f.fct = this.fct;
		f.couleur = this.couleur;
		f.couleurD1 = this.couleurDerivee1;
		f.couleurD2 = this.couleurDerivee2;
		f.couleurP1 = this.couleurPrimitive1;
		f.width = this.width;
		f.style = this.style;
		f.aire = this.aire;
		f.derivee1 = this.derivee1;
		f.derivee2 = this.derivee2;
		f.primitive1 = this.primitive1;
		f.conditionInitiale = this.conditionInitiale;
		return f;
	};
	
	this.readableText = function(){
		return "f(x) = " + this.fct;
	};
	
	this.plot = function(ctx, precision, affichage){
		var x, y;
		var pointX, pointY;
		var nonDefini;
		
		if(this.style == "points"){
			precision *= 2;
		}
		
		// Dérivée première
		if(this.derivee1){
			ctx.beginPath();
			ctx.strokeStyle = this.couleurDerivee1;
			ctx.fillStyle = this.couleurDerivee1;
			ctx.moveTo(-100,-100);
			nonDefini = 1;
			var pente;
			var x0 = affichage.xGauche-precision;
			var y0 = this.f(x0);
		
			for(x = affichage.xGauche-precision*2; x <= affichage.xDroite+precision; x+=precision){
				// Evaluer la valeur y du point x
				y = this.f(x);
				if(!isNaN(y)){
					// Transformation des coordonnées
					pente = (y-y0)/precision;
					pointX = (x-precision/2 - affichage.xGauche) * affichage.multX;
					pointY = affichage.hauteur - (pente - affichage.yBas) * affichage.multY;
					y0 = y;
					
					// Dessiner la dérivée
					if(nonDefini>0){
						nonDefini--;
						ctx.moveTo(pointX, pointY);
					}
					else{
						if(this.style == "continu"){
							ctx.lineTo(pointX, pointY);
						}
						else if(this.style == "points"){
							ctx.beginPath();
							ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
							ctx.fill();
						}
						else{
							ctx.lineTo(pointX, pointY);
							nonDefini = 1;
						}
					}
				}
				else{
					nonDefini = 2;
				}
			}
			if(this.style != "points"){
				ctx.stroke();
			}
		}
		
		// Dérivée seconde
		if(this.derivee2){
			ctx.beginPath();
			ctx.strokeStyle = this.couleurDerivee2;
			ctx.fillStyle = this.couleurDerivee2;
			ctx.moveTo(-100,-100);
			nonDefini = 1;
			var pente, pente2;
			var x0 = affichage.xGauche-precision;
			var y0 = this.f(x0);
			var p0 = 0;
		
			for(x = affichage.xGauche-precision*3; x <= affichage.xDroite+precision; x+=precision){
				// Evaluer la valeur y du point x
				y = this.f(x);
				if(!isNaN(y)){
					// Transformation des coordonnées
					p = (y-y0)/precision;
					pente = (p-p0)/precision;
					pointX = (x-precision/2 - affichage.xGauche) * affichage.multX;
					pointY = affichage.hauteur - (pente - affichage.yBas) * affichage.multY;
					y0 = y;
					p0 = p;
					
					// Dessiner la dérivée seconde
					if(nonDefini>0){
						nonDefini--;
						ctx.moveTo(pointX, pointY);
					}
					else{
						if(this.style == "continu"){
							ctx.lineTo(pointX, pointY);
						}
						else if(this.style == "points"){
							ctx.beginPath();
							ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
							ctx.fill();
						}
						else{
							ctx.lineTo(pointX, pointY);
							nonDefini = 1;
						}
					}
				}
				else{
					nonDefini = 3;
				}
			}
			if(this.style != "points"){
				ctx.stroke();
			}
		}
		
		// Primitive première
		if(this.primitive1){
			ctx.beginPath();
			ctx.strokeStyle = this.couleurPrimitive1;
			ctx.fillStyle = this.couleurPrimitive1;
			ctx.moveTo(-affichage.xGauche*affichage.multX,affichage.hauteur-(this.conditionInitiale-affichage.yBas)*affichage.multY);
			nonDefini = 0;
			var pente, y0;
			var conditionInitiale = this.conditionInitiale;
// 				precision /= 4;
			
			// Partie droite
			y0 = conditionInitiale;
			for(x = precision; x <= affichage.xDroite+precision; x+=precision){
				// Evaluer la valeur y du point x
				pente = this.f(x);
				if(!isNaN(pente)){
					// Transformation des coordonnées
					y = y0 + pente * precision;
					
					pointX = (x - affichage.xGauche) * affichage.multX;
					pointY = affichage.hauteur - (y - affichage.yBas) * affichage.multY;
					y0 = y;
					
					// Dessiner la primitive
					if(nonDefini>0){
						nonDefini--;
						ctx.moveTo(pointX, pointY);
					}
					else{
						if(this.style == "continu"){
							ctx.lineTo(pointX, pointY);
						}
						else if(this.style == "points"){
							ctx.beginPath();
							ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
							ctx.fill();
						}
						else{
							ctx.lineTo(pointX, pointY);
							nonDefini = 1;
						}
					}
				}
				else{
					nonDefini = 2;
				}
			}
			if(this.style != "points"){
				ctx.stroke();
			}
			// Partie gauche
			ctx.beginPath();
			ctx.moveTo(-affichage.xGauche*affichage.multX,affichage.hauteur-(this.conditionInitiale-affichage.yBas)*affichage.multY);
			nonDefini = 0;
			y0 = conditionInitiale;
			for(x = -precision; x >= affichage.xGauche-precision; x-=precision){
				// Evaluer la valeur y du point x
				pente = this.f(x);
				if(!isNaN(pente)){
					// Transformation des coordonnées
					y = y0 - pente * precision;
					
					pointX = (x - affichage.xGauche) * affichage.multX;
					pointY = affichage.hauteur - (y - affichage.yBas) * affichage.multY;
					y0 = y;
					
					// Dessiner la primitive
					if(nonDefini>0){
						nonDefini--;
						ctx.moveTo(pointX, pointY);
					}
					else{
						if(this.style == "continu"){
							ctx.lineTo(pointX, pointY);
						}
						else if(this.style == "points"){
							ctx.beginPath();
							ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
							ctx.fill();
						}
						else{
							ctx.lineTo(pointX, pointY);
							nonDefini = 1;
						}
					}
				}
				else{
					nonDefini = 2;
				}
			}
			if(this.style != "points"){
				ctx.stroke();
			}
		}
		
		// Fonction
		ctx.beginPath();
		ctx.lineWidth = this.width;
		if(affichage.methode == "uniboard"){
			ctx.lineWidth *= 3;
		}
		ctx.strokeStyle = this.couleur;
		ctx.fillStyle = this.couleur;
		ctx.moveTo(-100,-100);
		nonDefini = true;
		for(x = affichage.xGauche-precision; x <= affichage.xDroite+precision; x+=precision){
			// Evaluer la valeur y du point x
			y = this.f(x);
			if(!isNaN(y) && y <= affichage.yHaut+Math.abs(affichage.yHaut) && y >= affichage.yBas-Math.abs(affichage.yBas)){
				// Transformation des coordonnées
				pointX = (x - affichage.xGauche) * affichage.multX;
				pointY = affichage.hauteur - (y - affichage.yBas) * affichage.multY;
				
				// Dessiner la fonction
				if(nonDefini){
					nonDefini = false;
					ctx.moveTo(pointX, pointY);
				}
				else{
					if(this.style == "continu"){
						ctx.lineTo(pointX, pointY);
					}
					else if(this.style == "points"){
						ctx.beginPath();
						ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
						ctx.fill();
					}
					else{
						ctx.lineTo(pointX, pointY);
						nonDefini = true;
					}
				}
				
				// Dessiner l'aire sous la fonction
				if(this.aire){
					var zero = affichage.hauteur + affichage.yBas * affichage.multY;
					var l = precision*affichage.multX;
					ctx.fillStyle = "rgba(0,180,255,0.3)";
					ctx.fillRect(pointX-l/2, zero, l , pointY-zero);
					ctx.fillStyle = this.couleur;
				}
			}
			else{
				nonDefini = true;
			}
		}
		if(this.style != "points"){
			ctx.stroke();
		}
		
		if(this.style == "points"){
			precision /= 2;
		}
	};
}
