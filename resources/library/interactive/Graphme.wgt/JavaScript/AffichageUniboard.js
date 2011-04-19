// -------------------- Uniboard --------------------
// Ces fonctions permettent de dessiner le graphique directement dans Uniboard.

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
	   uniboard.setTool('pen')
	   uniboard.moveTo(pointX[2]+decalageX, pointY[2]+decalageY)
	   for (i=3; i<fin; i++){
	     if ((pointY[i]<0) || (pointY[i]>hauteur)){
		uniboard.moveTo(pointX[i+1]+decalageX,pointY[i+1]+decalageY)
		continue
	     }
	       uniboard.drawLineTo(pointX[i]+decalageX, pointY[i]+decalageY, lineWidth)
	   }

	   //dessiner le cadre
	   uniboard.moveTo(0+decalageX,0+decalageY)
	   uniboard.drawLineTo(largeur+decalageX, 0+decalageY, lineWidth)
	   uniboard.drawLineTo(largeur+decalageX, hauteur+decalageY, lineWidth)
	   uniboard.drawLineTo(0+decalageX, hauteur+decalageY, lineWidth)
	   uniboard.drawLineTo(0+decalageX, 0+decalageY, lineWidth)

	   //dessiner les axes
	   uniboard.moveTo((-borneXGauche*multiplicateurX)+decalageX, 0+decalageY)
	   uniboard.drawLineTo((-borneXGauche*multiplicateurX)+decalageX, hauteur+decalageY, lineWidth)
	   uniboard.moveTo(0+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
	   uniboard.drawLineTo(largeur+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY, lineWidth)

	   decalageX += 250
	   decalageY += 200
	   largeur -= 100
	   hauteur -= 100
	   uniboard.setTool('arrow')
	}
	