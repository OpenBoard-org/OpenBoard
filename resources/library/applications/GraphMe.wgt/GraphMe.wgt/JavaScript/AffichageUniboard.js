// -------------------- Uniboard --------------------
// Ces fonctions permettent de dessiner le graphique directement dans Uniboard.

function colorToHex(color) {
	var addZero = function(myString){
		if (myString.length == 1) return "0" + myString;
		else return myString;
	}
	
	var digits = /(.*?)rgb\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.exec(color);
	if (digits == null){
		digits = /(.*?)rgba\(\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\)/.exec(color);
	}
	if(digits == null){
		return "";
	}
	var red = parseInt(digits[2]);
	var green = parseInt(digits[3]);
	var blue = parseInt(digits[4]);
	var hexcode = addZero(red.toString(16)) + addZero(green.toString(16)) + addZero(blue.toString(16));
	return '#' + hexcode.toUpperCase();
} 

function initUniboard(){
	if(!window.uniboard && window.sankore){
		uniboard = sankore;
	}
	uniboard.centerOn(337,245);
// 	uniboard.setTool("pen");
	
	// Paramètres par défaut
	uniboard.lineWidth = 1;
	uniboard.fillStyle = "black";
	uniboard.strokeStyle = "black";
	
	// Position du curseur pour écrire des nombres
	uniboard.cursorX = 0;
	uniboard.cursorY = 0;
	
	// Fonctions
	uniboard.strokeColor = function(){
		this.setPenColor(colorToHex(this.strokeStyle));
	};
	
	uniboard.fillColor = function(){
		this.setPenColor(colorToHex(this.fillStyle));
	};
	
	uniboard.lineTo = function(x, y){
		if(y>-20 && y<affichage.hauteur+20){
			this.strokeColor();
			this.drawLineTo(x, y, this.lineWidth/2);
		}
	};
	
	uniboard.arc = function(x, y, r){
		this.fillColor();
		this.moveTo(x, y);
		this.drawLineTo(x, y, r*2);
	};
	
	uniboard.clearRect = function(x, y, width, height){
		if(height<0){
			height = Math.abs(height);
			y -= height;
		}
		if(width<0){
			width = Math.abs(width);
			x -= width;
		}
		this.moveTo(x, y)
		this.eraseLineTo(x+width, y+height, this.lineWidth);
	};
	
	uniboard.fillRect = function(x, y, width, height){
		this.fillColor();
		if(height<0){
			height = Math.abs(height);
			y -= height;
		}
		if(width<0){
			width = Math.abs(width);
			x -= width;
		}
		this.moveTo(x, y)
		this.drawLineTo(x+width, y+height, this.lineWidth);
	};
	
	uniboard.fillText = function(txt, x, y){
		// TODO
		this.fillNumber(txt, x, y);
	};
	
	uniboard.beginPath = function(){
		this.moveTo(0, 0);
	};
	
	uniboard.fill = function(){};
	uniboard.stroke = function(){};
    
	uniboard.fillNumber = function(nbr, x, y){
		var w = 5;
		var number = "" + nbr;
		
		if(!x || !y){
			x = this.cursorX;
			y = this.cursorY;
		}
		else{
			x += 3;
			y -= 12;
		}
		this.moveTo(x, y);
		this.fillColor();
		
		var move = function(){
			uniboard.moveTo(x, y);
		};
		var draw = function(){
			uniboard.drawLineTo(x, y, uniboard.lineWidth)
		};
		var space = function(){
			x += 2*w/3;
			move();
		};
		
		for(var i=0; i<number.length; i++){
			switch(number.charAt(i)){
				case "1":
					y += w
					move()
					x += w
					y -= w
					draw()
					y += 2*w
					draw()
					y -= 2*w
					space()
					break;
				case "2":
					x += w
					draw()
					y += w
					draw()
					x -= w
					draw()
					y += w
					draw()
					x += w
					draw()
					y -= 2*w
					space()
					break;
				case "3":
					x += w
					draw()
					y += w
					draw()
					x -= w
					draw()
					x += w
					move()
					y += w
					draw()
					x -= w
					draw()
					y -= 2*w
					x += w
					space()
					break;
				case "4":
					y += w
					draw()
					x += w
					draw()
					y -= w
					draw()
					y += w
					move()
					y += w
					draw()
					y -= 2*w
					space()
					break;
				case "5":
					x += w
					move()
					x -= w
					draw()
					y += w
					draw()
					x += w
					draw()
					y += w
					draw()
					x -= w
					draw()
					x += w
					y -= 2*w
					space()
					break;
				case "6":
					x += w
					move()
					x -= w
					draw()
					y += w
					draw()
					x += w
					draw()
					y += w
					draw()
					x -= w
					draw()
					y -= w
					draw()
					x += w
					y -= w
					space()
					break;
				case "7":
					x += w
					draw()
					x -= w/2
					y += 2*w
					draw()
					x += w/2
					y -= 2*w
					space()
					break;
				case "8":
					x += w
					draw()
					y += 2*w
					draw()
					x -= w
					draw()
					y -= 2*w
					draw()
					y += w
					move()
					x += w
					draw()
					y -= w
					move()
					space()
					break;
				case "9":
					x += w
					move()
					x -= w
					draw()
					y += w
					draw()
					x += w
					draw()
					y += w
					draw()
					x -= w
					draw()
					y -= w
					x += w
					move()
					y -= w
					draw()
					space()
					break;
				case "0":
					x += w
					draw()
					y += 2*w
					draw()
					x -= w
					draw()
					y -= 2*w
					draw()
					x += w
					move()
					space()
					break;
				case "-":
					y += w
					move()
					x += w
					draw()
					y -= w
					move()
					space()
					break;
				default: // Nombre inconnu
// 					this.showMessage("Error : "+number.charAt(i)+" isn't a number");
			}
		}
		this.cursorX = x;
		this.cursorY = y;
// 		this.showMessage(number);
	}
}

