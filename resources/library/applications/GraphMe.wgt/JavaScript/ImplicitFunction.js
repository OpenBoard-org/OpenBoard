 
function ImplicitFunction(txtFct){

	this.setFct = function(txt){
		this.fct = txt;
		this.f = new Function("x", "y", "return "+this.fct);
	}
	
	if(txtFct){
		this.setFct(txtFct);
	}
	this.couleur = fct.couleur;
	this.width = document.getElementById("inputTaille").value;
	this.style = document.getElementById("selectStyle").value;
	
	this.getX = function(x){
		return x;
	};
	
	this.getY = function(x){
		return -1000000;
	};
	
	this.set = function(f){
		var equalPos = f.fct.indexOf("=");
		if(equalPos >= 0){
			f.fct = f.fct.replace("=", "-(") + ")";
		}
		this.setFct(fct.remplacer(fct.verifier(f.fct)));
		this.couleur = f.couleur;
		this.width = f.width;
		this.style = f.style;
		return this;
	};
	
	this.get = function(){
		var f = {};
		f.type = "implicit";
		f.fct = this.fct;
		f.couleur = this.couleur;
		f.width = this.width;
		f.style = this.style;
		return f;
	};
		
	this.readableText = function(){
		return this.fct + " = 0";
	};
	
	this.plot = function(ctx, precision, affichage){
		var epsilon = 0.01;
		var width2 = this.width / 2;
		precision /= 20;
		
		ctx.beginPath();
		ctx.lineWidth = this.width;
		ctx.strokeStyle = this.couleur;
		ctx.fillStyle = this.couleur;
		ctx.moveTo(-100,-100);
		for(var x = affichage.xGauche-precision; x <= affichage.xDroite+precision; x+=precision){
			for(var y = affichage.yBas-precision; y <= affichage.yHaut+precision; y+=precision){
				// Compute function for current (x, y) position
				z = this.f(x, y);
				// Check condition f(x, y) = 0
				if(!isNaN(z) && Math.abs(z) < epsilon){
					// Transform coordinates
					var pointX = (x - affichage.xGauche) * affichage.multX;
					var pointY = affichage.hauteur - (y - affichage.yBas) * affichage.multY;
					
					ctx.fillRect(pointX-width2, pointY-width2, this.width, this.width);
// 					ctx.beginPath();
// 					ctx.arc(pointX, pointY, this.width, 0, 2*Math.PI, true);
// 					ctx.fill();
				}
			}
		}
// 		ctx.stroke();
	};
}
