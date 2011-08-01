var ctx
var centreX, centreY
var echelle3D = 50
var precisionDroite3D = 0.02
var precisionFonction3D = 0.2
var fonction3D = false
var angle = Math.PI/8
var valeurZoom3D = 1
var gauche3D = -6.5
var droite3D = 6.5
var outilPrecedent = ""
var rouge3D = 0
var vert3D = 1
var bleu3D = 2
var couleurGenerale = 0

function activer3D(){
   if(fonction3D){ // Si activé alors on le désative
      fonction3D = false
      choixOutil(outilPrecedent)
      document.getElementById('onglet3D').innerHTML = "3D"
	  largeur = 500
      document.getElementById("affichage").style.width = largeur+"px"
      document.getElementById("affichage").style.left = "129px"
      document.getElementById("flecheGauche").style.left = "137px"
      document.getElementById("flecheHaut").style.left = "345px"
      document.getElementById("flecheBas").style.left = "345px"
      document.getElementById("gauche").style.display = "block"
      document.getElementById("gauche3D").style.display = "none"
      var elements = document.getElementsByClassName("menu")
      for(var i=0; i<elements.length; i++){
		  elements[i].style.left = "121px"
	  }
      reinitialiserZoom(5)
   }
   else{ // Sinon on l'active
	  fonction3D = true
	  outilPrecedent = outil
	  choixOutil("deplacement")
      document.getElementById('onglet3D').innerHTML = "2D"
	  largeur = 570
      document.getElementById("affichage").style.width = largeur+"px"
      document.getElementById("affichage").style.left = "59px"
      document.getElementById("flecheGauche").style.left = "67px"
      document.getElementById("flecheHaut").style.left = "290px"
      document.getElementById("flecheBas").style.left = "290px"
      document.getElementById("gauche").style.display = "none"
      document.getElementById("gauche3D").style.display = "block"
      var elements = document.getElementsByClassName("menu")
      for(var i=0; i<elements.length; i++){
		  elements[i].style.left = "61px"
	  }
      initialise3D()
   }
}

function initialise3D() {
   centreX = largeur / 2
   centreY = hauteur / 2
   document.getElementById("affichage").innerHTML = '<canvas id="canvas" width="'+largeur+'" height="'+hauteur+'"></canvas>'
   ctx = document.getElementById('canvas').getContext('2d')
   ctx.clearRect(0, 0, 640, 480)
   ctx.fillStyle = "rgba(0, 0, 0, 0.5)"
   ctx.fillRect(0,0,640,480)
   axes()
}

function dessiner3D(eq){
	initialise3D()
	if(document.getElementById("selectAffichage3D").value == "points"){
		var coordX, coordY, coordZ
		for(var x=gauche3D; x<droite3D; x+=precisionFonction3D){
			for(var y=gauche3D; y<droite3D; y+=precisionFonction3D){
				coordX = x
				coordY = y
				coordZ = eval(eq)
				if(isNaN(coordZ)){
					continue
				}
				point3D(coordX, coordY, coordZ)
			}
		}
	}
	else{
		var x1, y1, z1, x2, y2, z2, x3, y3, z3
		var x, y
		for(var x1=gauche3D; x1<droite3D; x1+=precisionFonction3D){
			for(var y1=gauche3D; y1<droite3D; y1+=precisionFonction3D){
				x = x1
				y = y1
				z1 = eval(eq)
				x2 = x1 + precisionFonction3D
				y2 = y1
				x = x2
				//y = y2
				z2 = eval(eq)
				x3 = x2
				y3 = y2 + precisionFonction3D
				//x = x3
				y = y3
				z3 = eval(eq)
				x4 = x3 - precisionFonction3D
				y4 = y3
				x = x4
				//y = y4
				z4 = eval(eq)
				if(isNaN(z1)||isNaN(z2)||isNaN(z3)||isNaN(z4)){
					continue
				}
				polygone3D(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4)
			}
		}
	}
}

// Dessine un point à la position (x, y, z)
function point3D(x, y, z){ 
	var posX = (Math.sin(angle)*x + Math.cos(angle)*y)*echelle3D
	var posZ = -(z - Math.cos(angle)*x/2.6 + Math.sin(angle)*y/2.6)*echelle3D

	var opacity = Math.round((1-((5+y*Math.sin(angle)-x*Math.cos(angle)) / 450)*echelle3D)*1000)/1000
	var couleur = new Array()
	couleur[0] = Math.round((5+z)*echelle3D)
	couleur[1] = Math.round(510 - (5+z)*echelle3D)
	couleur[2] = couleurGenerale
	if(opacity > 1){ opacity = 1 }
	if(opacity < 0){ opacity = 0 }
	if(couleur[0] > 255){ couleur[0] = 255 }
	if(couleur[0] < 0){ couleur[0] = 0 }
	if(couleur[1] > 255){ couleur[1] = 255 }
	if(couleur[1] < 0){ couleur[1] = 0 }

	ctx.save()
	ctx.translate(centreX, centreY)
	ctx.scale(valeurZoom3D, valeurZoom3D)
	ctx.fillStyle = "rgba("+couleur[rouge3D]+","+couleur[vert3D]+", "+couleur[bleu3D]+", "+opacity+")"
	ctx.fillRect(posX-1, posZ-1, 2, 2)
	ctx.restore()
}

// Dessine un polygone qui a comme sommets : (x1, y1, z1) , (x2, y2, z2), (x3, y3, z3) et (x4, y4, z4)
function polygone3D(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4){
	ctx.save()
	ctx.translate(centreX, centreY)
	ctx.scale(valeurZoom3D, valeurZoom3D)
	ctx.beginPath()
	ctx.moveTo((Math.sin(angle)*x1 + Math.cos(angle)*y1)*echelle3D , -(z1 - Math.cos(angle)*x1/2.6 + Math.sin(angle)*y1/2.6)*echelle3D)
	ctx.lineTo((Math.sin(angle)*x2 + Math.cos(angle)*y2)*echelle3D , -(z2 - Math.cos(angle)*x2/2.6 + Math.sin(angle)*y2/2.6)*echelle3D)
	ctx.lineTo((Math.sin(angle)*x3 + Math.cos(angle)*y3)*echelle3D , -(z3 - Math.cos(angle)*x3/2.6 + Math.sin(angle)*y3/2.6)*echelle3D)
	ctx.lineTo((Math.sin(angle)*x4 + Math.cos(angle)*y4)*echelle3D , -(z4 - Math.cos(angle)*x4/2.6 + Math.sin(angle)*y4/2.6)*echelle3D)

	var opacity = Math.round((1-((5+y1*Math.sin(angle)-x1*Math.cos(angle)) / 450)*echelle3D)*1000)/1000
	var couleur = new Array()
	couleur[0] = Math.round((5+z1)*echelle3D)
	couleur[1] = Math.round(510 - (5+z1)*echelle3D)
	couleur[2] = couleurGenerale
	if(opacity > 1){ opacity = 1 }
	if(opacity < 0){ opacity = 0 }
	if(couleur[0] > 255){ couleur[0] = 255 }
	if(couleur[0] < 0){ couleur[0] = 0 }
	if(couleur[1] > 255){ couleur[1] = 255 }
	if(couleur[1] < 0){ couleur[1] = 0 }

	ctx.fillStyle = "rgba("+couleur[rouge3D]+","+couleur[vert3D]+", "+couleur[bleu3D]+", "+opacity+")"
	ctx.closePath()
	ctx.fill()
	ctx.restore()
}

// Dessine les axes
function axes(){
   for(var i=-5; i<5; i+=precisionDroite3D){
      point3D(0, 0, i)
   }
   for(var i=-5.5; i<5.5; i+=precisionDroite3D){
      point3D(i, 0, 0)
   }
   for(var i=-5.5; i<5.5; i+=precisionDroite3D){
      point3D(0, i, 0)
   }
}

function cube(x, y, z, r){
   // Face de devant
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+i, y, z)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r, y+i, z)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r-i, y+r, z)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x, y+r-i, z)
   }
   // Face de derrière
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+i, y, z+r)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r, y+i, z+r)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r-i, y+r, z+r)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x, y+r-i, z+r)
   }
   // Arrêtes
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x, y, z+i)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x, y+r, z+i)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r, y, z+i)
   }
   for(var i=0; i<r; i+=precisionDroite3D){
      point3D(x+r, y+r, z+i)
   }
}

// Change les couleurs de l'affichage 3D et affiche l'aperçu de celles-ci.
function checkCouleurs3D(){
	var rouge, vert, bleu
	var sensRouge, sensVert, sensBleu
	var precisionApercu = 64
	couleurGenerale = parseInt(document.getElementById("couleur3Dgenerale").value)
	if(!couleurGenerale){ couleurGenerale = 0 }
	if(couleurGenerale < 0){ couleurGenerale = 0 }
	if(couleurGenerale > 255){ couleurGenerale = 255 }
	switch(document.getElementById("selectRouge3D").value){
		case "plus":
			rouge3D = 0
			rouge = 255
			sensRouge = -1
			break
		case "moins":
			rouge3D = 1
			rouge = 0
			sensRouge = 1
			break
		case "tout":
			rouge3D = 2
			rouge = couleurGenerale
			sensRouge = 0
			break
	}
	switch(document.getElementById("selectVert3D").value){
		case "plus":
			vert3D = 0
			vert = 255
			sensVert = -1
			break
		case "moins":
			vert3D = 1
			vert = 0
			sensVert = 1
			break
		case "tout":
			vert3D = 2
			vert = couleurGenerale
			sensVert = 0
			break
	}
	switch(document.getElementById("selectBleu3D").value){
		case "plus":
			bleu3D = 0
			bleu = 255
			sensBleu = -1
			break
		case "moins":
			bleu3D = 1
			bleu = 0
			sensBleu = 1
			break
		case "tout":
			bleu3D = 2
			bleu = couleurGenerale
			sensBleu = 0
			break
	}
	document.getElementById("apercuCouleur3D").innerHTML = "<span style='background-color:rgba("+rouge+","+vert+","+bleu+",1);color:rgba(0,0,0,0);'>.</span>"
	for(var i=0; i<precisionApercu; i++){
		rouge = rouge + sensRouge*(256/precisionApercu)
		vert = vert + sensVert*(256/precisionApercu)
		bleu = bleu + sensBleu*(256/precisionApercu)
		document.getElementById("apercuCouleur3D").innerHTML = document.getElementById("apercuCouleur3D").innerHTML + "<span style='background-color:rgba("+rouge+","+vert+","+bleu+",1);color:rgba(0,0,0,0);'>.</span>"
	}
}

function zoom3D(valeur){
	valeurZoom3D *= valeur
	gauche3D /= valeur
	droite3D /= valeur
	precisionFonction3D /= valeur
	precisionDroite3D /= valeur
	//alert(valeurZoom3D+" ; "+gauche3D+" ; "+droite3D+" ; "+precisionFonction3D+" ; "+precisionDroite3D)
	actualiserGraph()
}