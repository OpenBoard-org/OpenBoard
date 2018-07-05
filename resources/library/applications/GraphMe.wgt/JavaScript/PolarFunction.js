 
function PolarFunction(txtFct){

	this.setFct = function(txt){
		this.fct = txt;
		this.f = new Function("t", "return "+this.fct);
	};
	
	if(txtFct){
		this.setFct(txtFct);
	}
	this.couleur = fct.couleur;
	this.width = document.getElementById("inputTaille").value;
	this.startAngle = 0;
	this.endAngle = 2*Math.PI;
	this.from = "0";
	this.to = "2*pi";
	this.style = document.getElementById("selectStyle").value;
	
	this.getX = function(t){
		var r = this.f(t);
		return r * Math.cos(t);
	};
	
	this.getY = function(t){
		var r = this.f(t);
		return r * Math.sin(t);
	};
	
	this.set = function(f){
		this.setFct(fct.remplacer(fct.verifier(f.fct)));
		this.couleur = f.couleur;
		this.width = f.width;
		this.style = f.style;
		this.from = f.from;
		this.to = f.to;
		this.startAngle = eval(fct.remplacer(fct.verifier(f.from)));
		this.endAngle = eval(fct.remplacer(fct.verifier(f.to)));
		return this;
	};
	
	this.get = function(){
		var f = {};
		f.type = "polar";
		f.fct = this.fct;
		f.couleur = this.couleur;
		f.width = this.width;
		f.style = this.style;
		f.from = this.from;
		f.to = this.to;
		return f;
	};
	
	this.readableText = function(){
		return "r(t) = " + this.fct;
	};
	
	this.setStartEnd = function(start, end){
		if(start > end){
			var tmp = start;
			start = end;
			end = start;
		}
		this.startAngle = start;
		this.endAngle = end;
	};
	
	this.plot = function(ctx, precision, affichage){
		if(this.style == "points"){
			precision *= 2;
		}
		
		ctx.beginPath();
		ctx.lineWidth = this.width;
		ctx.strokeStyle = this.couleur;
		ctx.fillStyle = this.couleur;
		notDefined = true;
		for(var t = this.startAngle; t <= this.endAngle; t+=precision){
			var r = this.f(t);
			if(!isNaN(r)){
				// Transform coordinates
				var x = r * Math.cos(t);
				var y = r * Math.sin(t);
				var pointX = (x - affichage.xGauche) * affichage.multX;
				var pointY = affichage.hauteur - (y - affichage.yBas) * affichage.multY;
				
				// Draw point
				if(notDefined){
					notDefined = false;
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
						notDefined = true;
					}
				}
			}
		}
		if(this.style != "points"){
			ctx.stroke();
		}
	};
}
