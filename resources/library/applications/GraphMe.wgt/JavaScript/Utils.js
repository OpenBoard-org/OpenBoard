
// Lance la procédure pour dessiner la fonction qui se trouve dans l'input en haut du widget.
// Permet aussi d'actualiser le graphique.
function actualiserGraph(){
	affichage.calculer();
}

// Retourne l'index du dernier endroit où se trouve listCharacter dans une chaîne de caractère. 
String.prototype.getLastIndexOf = function(listCharacter, minPos, maxPos){
	var count = 0;
	var parenthese = false;
	if(!maxPos){
		maxPos = this.length;
	}
	for(var i=maxPos; i>=minPos; i--){
		if(this.charAt(i) == ")"){
			count++;
			parenthese = true;
		}
		else if(this.charAt(i) == "("){
			count--;
		}
// 					window.console.log(this.charAt(i)+" ; "+count);
		for(var j=0; j<listCharacter.length; j++){
			if(this.charAt(i) == listCharacter[j]){
				if(count<=0){
					if(parenthese){
						parenthese = false;
					}
					else{
						return i;
					}
				}
			}
		}
	}
	return -1;
}
String.prototype.getIndexOf = function(listCharacter, minPos, maxPos){
	var count = 0;
	if(!maxPos){
		maxPos = this.length;
	}
	for(var i=minPos; i<=maxPos; i++){
		if(this.charAt(i) == "("){
			count++;
		}
		else if(this.charAt(i) == ")"){
			count--;
		}
		for(var j=0; j<listCharacter.length; j++){
			if(this.charAt(i) == listCharacter[j]){
				if(count==0){
					return i;
				}
			}
		}
	}
	return -1;
}

// Retourne un nombre aléatoire entre a et b.
function alea(a,b){
return Math.floor((b-a+1)*Math.random()+a);
}

// ---- Fonctions canvas personnalisées ----
var ctxPrototype = Object.getPrototypeOf(document.createElement("canvas").getContext("2d"));

// Dessiner une bulle 
ctxPrototype.bulle = function(x, y, largeur, hauteur, rayon){
if(!rayon){
	rayon = 5;
}
this.beginPath();
this.moveTo(x+0, y-rayon);
this.quadraticCurveTo(x+0, y-0, x+rayon, y-0);
this.lineTo(x+largeur-rayon, y);
this.quadraticCurveTo(x+largeur, y-0, x+largeur, y-rayon);
this.lineTo(x+largeur, y-hauteur+rayon);
this.quadraticCurveTo(x+largeur, y-hauteur, x+largeur-rayon, y-hauteur);
this.lineTo(x+rayon, y-hauteur);
this.quadraticCurveTo(x+0, y-hauteur, x+0, y-hauteur+rayon);
this.closePath();
this.fill();
}


// ---- Fonctions Mathématiques et constantes ----
// (rempalce sin() par Math.sin(), cos() par Math.cos(), tan() par Math.tan(), etc.
var pi = 4 * atan(1);
var PI = pi;
var e = exp(1);

function sin(valeur){
	return Math.sin(valeur);
}
function cos(valeur){
	return Math.cos(valeur);
}
function tan(valeur){
	return Math.tan(valeur);
}
function cot(valeur){
	return 1/Math.tan(valeur);
}
function sec(valeur){
	return 1/cos(valeur);
}
function csc(valeur){
	return 1/sin(valeur);
}

function asin(valeur){
	return Math.asin(valeur);
}
function acos(valeur){
	return Math.acos(valeur);
}
function atan(valeur){
	return Math.atan(valeur);
}
function acot(valeur){
	return Math.atan(1/valeur);
}
function asec(valeur){
	return Math.acos(1/valeur);
}
function acsc(valeur){
	return Math.asin(1/valeur);
}

var arcsin = asin;
var arccos = acos;
var arctan = atan;
var arccot = acot;
var arcsec = asec;
var arccsc = acsc;

function sinh(valeur){
	return (Math.exp(valeur)-Math.exp(-valeur))/2;
}
function cosh(valeur){
	return (Math.exp(valeur)+Math.exp(-valeur))/2;
}
function tanh(valeur){
	return (Math.exp(valeur)-Math.exp(-valeur))/(Math.exp(valeur)+Math.exp(-valeur));
}
function coth(valeur){
	return (Math.exp(valeur)+Math.exp(-valeur))/(Math.exp(valeur)-Math.exp(-valeur));
}
function sech(valeur){
	return 1/cosh(valeur);
}
function csch(valeur){
	return 1/sinh(valeur);
}

function asinh(valeur){
	return Math.log(valeur+Math.sqrt(Math.pow(valeur,2)+1));
}
function acosh(valeur){
	return Math.log(valeur+Math.sqrt(Math.pow(valeur,2)-1));
}
function atanh(valeur){
	return Math.log((1+valeur)/(1-valeur))/2;
}
function acoth(valeur){
	return Math.log((valeur+1)/(valeur-1))/2;
}

var arcsinh = asinh;
var arccosh = acosh;
var arctanh = atanh;
var arccoth = acoth;

function sqrt(valeur){
	return Math.sqrt(valeur);
}
function pow(valeur1, valeur2){
	return Math.pow(valeur1, valeur2);
}
function root(valeur1, valeur2){
	if(valeur2%2==1 && valeur1<0){
		return -Math.pow(-valeur1, (1/valeur2));
	}
	else{
		return Math.pow(valeur1, (1/valeur2));
	}
}

function exp(valeur){
	return Math.exp(valeur);
}
function log(valeur){
	return Math.log(valeur)/Math.log(10);
}
function ln(valeur){
	return Math.log(valeur);
}

function abs(valeur){
	return Math.abs(valeur);
}
function sign(valeur){
	if(valeur<0){
		return -1;
	}
	else if(valeur>0){
		return 1;
	}
	else{
		return undefined;
	}
}

function round(valeur){
	return Math.round(valeur);
}
function ceil(valeur){
	return Math.ceil(valeur);
}
function floor(valeur){
	return Math.floor(valeur);
}
function random(){
	return Math.random();
}
