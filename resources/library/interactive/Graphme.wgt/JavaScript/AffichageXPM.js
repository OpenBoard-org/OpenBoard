// -------------------- XPM --------------------
// Diverses fonctions permettant de dessiner la fonction mathématique à
// l'aide d'une image au format XPM

// Permet de calculer tous les points de l'image et de les placer dans un tableau.
// Chaque "case" du tableau est un point de l'image.
// Plus tard, chaque pixel de l'image correspondra a une case du tableau.
// Le pixel sera blanc si la "case" vaut 0 et sera noir si la "case" vaut 1
	function evaluerXPM(eq){
	   tableauUni(0)
	   var y = 0
	   BoucleFor: for (x=0; x<largeur; x++){
	      y = Math.round(eval(eq))
	      if (y>hauteur-1){ break BoucleFor }
	      tableau[y][x] = 1
	   }
	   tableau.reverse()
	   graphique = ""
	   calculerGraphXPM()
	}

// Permet de remplir le tableau avec uniquement des 1 ou des 0.
// (pour que l'image soit toute blanche ou toute noir)
	function tableauUni(valeurCouleur){
	   for (i=0; i<hauteur; i++){
	      ligne = new Array()
	      for (k=0; k<largeur; k++){
		 ligne.push(valeurCouleur)
	      }
	      tableau[i] = ligne
	   }
	   graphique = ""
	   calculerGraphXPM()
	}

// Cette fonction regarde chaque "case" du tableau.
// Elle permet de générer le texte ASCII de l'image XPM.
	function calculerGraphXPM() {
	   graphique = graphique + '"'
	   for (i=0; i<largeur; i++){
	      switch (tableau[j][i]){
	      case 1:
		 graphique = graphique + ' '
		 break
	      default:
		 graphique = graphique + '.'
		 break
	      }
	   }
	   j++
	   graphique = graphique + '",'
	   if (j == hauteur){
	      afficherGraphXPM()
	      j = 0
	      return true
	   }
	   calculerGraphXPM()
	}

// Cette fonction génére le code HTML nécessaire à afficher l'image XPM.
// Elle place ce code dans la div "affichage" ce qui permet d'afficher le graphique.
	function afficherGraphXPM(){
	   image = "<img src='data:image/xpm;ASCII,"+'/* XPM */static char * test_xpm[] = {"'+largeur+' '+hauteur+' 2 1"," 	c #000000",". 	c #FFFFFF",' + graphique + "};'"+" />"
	   document.getElementById("affichage").innerHTML = image
	}