// -------------------- sankore --------------------
// Ces fonctions permettent de dessiner le graphique directement dans sankore.

// Calcule tous les points de la fonction mathématique et les place dans des tableaux.
	function evaluerUniboard(eq) {
	   largeur += 100
	   hauteur += 100
	   decalageX -= 250
	   decalageY -= 200
	   borneXGauche = parseFloat(document.getElementById("borneXGauche").value)
	   borneXDroite = parseFloat(document.getElementById("borneXDroite").value)
	   borneYGauche = parseFloat(document.getElementById("borneYGauche").value)
	   borneYDroite = parseFloat(document.getElementById("borneYDroite").value)
	   multiplicateurX = (largeur)/Math.abs(borneXDroite - borneXGauche)
	   multiplicateurY = (hauteur)/Math.abs(borneYDroite - borneYGauche)
	   lineWidth = document.getElementById("inputTaille").value
	   var i = 0

	   BoucleFor: for(x=borneXGauche; x<=(borneXDroite+0); x=x+precision){
	      i++
	      y = eval(eq)
	      pointX[i] = Math.round((x - borneXGauche) * multiplicateurX)
	      pointY[i] = hauteur - Math.round((y - borneYGauche) * multiplicateurY)
	   }
	   calculerGraphUniboard(i)
	}

// Regarde chaque coordonnées stockées dans le tableau et dessine le graphique
	function calculerGraphUniboard(fin){
	   document.getElementById("affichage").innerHTML = ""
	   sankore.setTool('pen')
	   sankore.moveTo(pointX[2]+decalageX, pointY[2]+decalageY)
	   for (i=3; i<fin; i++){
	     if ((pointY[i]<0) || (pointY[i]>hauteur)){
		sankore.moveTo(pointX[i+1]+decalageX,pointY[i+1]+decalageY)
		continue
	     }
	       sankore.drawLineTo(pointX[i]+decalageX, pointY[i]+decalageY, lineWidth)
	   }

	   //dessiner le cadre
	   sankore.moveTo(0+decalageX,0+decalageY)
	   sankore.drawLineTo(largeur+decalageX, 0+decalageY, lineWidth)
	   sankore.drawLineTo(largeur+decalageX, hauteur+decalageY, lineWidth)
	   sankore.drawLineTo(0+decalageX, hauteur+decalageY, lineWidth)
	   sankore.drawLineTo(0+decalageX, 0+decalageY, lineWidth)

	   //dessiner les axes
	   sankore.moveTo((-borneXGauche*multiplicateurX)+decalageX, 0+decalageY)
	   sankore.drawLineTo((-borneXGauche*multiplicateurX)+decalageX, hauteur+decalageY, lineWidth)
	   sankore.moveTo(0+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
	   sankore.drawLineTo(largeur+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY, lineWidth)

	   decalageX += 250
	   decalageY += 200
	   largeur -= 100
	   hauteur -= 100
	   sankore.setTool('arrow')
	}
	