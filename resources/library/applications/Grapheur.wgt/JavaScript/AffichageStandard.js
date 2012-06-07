// ------------------ SVG et canvas ------------------

// Cette fonction calcule tous les points de la fonction mathématique.
// Elle place chaque coordonnée "x" et "y" dans les tableaux "pointX" et "pointY"
// A la fin, elle choisi la méthode d'affichage entre :
// 1) une seule image SVG
// 2) plusieurs images SVG
	function evaluerSVG(eq) {
		borneXGauche = parseFloat(document.getElementById("borneXGauche").value)
		borneXDroite = parseFloat(document.getElementById("borneXDroite").value)
		borneYGauche = parseFloat(document.getElementById("borneYGauche").value)
		borneYDroite = parseFloat(document.getElementById("borneYDroite").value)
		multiplicateurX = largeur/Math.abs(borneXDroite - borneXGauche)
		multiplicateurY = hauteur/Math.abs(borneYDroite - borneYGauche)
		lineWidth = document.getElementById("inputTaille").value
		var i = 0
		var y1, p1

		for(x=borneXGauche; x<=(borneXDroite+5*precision); x=x+precision){
			y = eval(eq)
			if(!isNaN(y)){
				i++
				pointX[i] = (x - borneXGauche) * multiplicateurX
				pointY[i] = hauteur - ((y - borneYGauche) * multiplicateurY)
				pente[i] = hauteur - (((y-y1)/precision - borneYGauche)* multiplicateurY)
				pente2[i] = hauteur - ((((y-y1)/precision-p1)/precision - borneYGauche)* multiplicateurY)
				p1 = (y-y1)/precision
				y1 = y
			}
		}
		pente[1]=pente[2]
		pente2[2]=pente2[3]
		pente2[1]=pente2[2]

		//alert(pointX+'\n'+pointY)
		if(document.getElementById("selectMethodeAffichage").value == "svg2"){
			calculerGraphSVG2(i)
		}
		else{
			calculerGraphSVG(i)
		}
	}

// Même fonction mais pour dessiner à l'aide de canvas
	function evaluerCanvas(eq) {
		borneXGauche = parseFloat(document.getElementById("borneXGauche").value)
		borneXDroite = parseFloat(document.getElementById("borneXDroite").value)
		borneYGauche = parseFloat(document.getElementById("borneYGauche").value)
		borneYDroite = parseFloat(document.getElementById("borneYDroite").value)
		multiplicateurX = largeur/Math.abs(borneXDroite - borneXGauche)
		multiplicateurY = hauteur/Math.abs(borneYDroite - borneYGauche)
		lineWidth = document.getElementById("inputTaille").value
		var i = 0
		var y1, p1

		for(x=borneXGauche; x<=(borneXDroite+5*precision); x=x+precision){
			y = eval(eq)
			i++
			if(!isNaN(y)){
				pointX[i] = (x - borneXGauche) * multiplicateurX
				pointY[i] = hauteur - ((y - borneYGauche) * multiplicateurY)
				pente[i] = hauteur - (((y-y1)/precision - borneYGauche)* multiplicateurY)
				pente2[i] = hauteur - ((((y-y1)/precision-p1)/precision - borneYGauche)* multiplicateurY)
				p1 = (y-y1)/precision
				y1 = y
			}
			else{
				pointX[i] = "undefined"
				pointY[i] = "undefined"
				pente[i] = "undefined"
				pente2[i] = "undefined"
			}
		}
		pente[1]=pente[2]
		pente2[2]=pente2[3]
		pente2[1]=pente2[2]

		calculerGraphCanevas(i)
	}


// ---- SVG (une image) ----
// Génère le code HTML qui permet d'afficher le graphique et le place dans la div "affichage"
	function calculerGraphSVG2(fin){
		image = ""
		for (i=1; i<fin; i++){
			image = image + '<line x1="'+pointX[i]+'" y1="'+pointY[i]+'" x2="'+pointX[i+1]+'" y2="'+pointY[i+1]+'" style="stroke:'+couleurFonction+';stroke-width:2;"/>'
		}
		graphique = '<line x1="'+(-borneXGauche*multiplicateurX)+'" y1="'+0+'" x2="'+(-borneXGauche*multiplicateurX)+'" y2="'+hauteur+'" style="stroke:rgb(0,0,0);stroke-width:2;opacity:0.3;"/>'
		graphique = graphique + '<line x1="'+0+'" y1="'+(hauteur-(-borneYGauche*multiplicateurY))+'" x2="'+largeur+'" y2="'+(hauteur-(-borneYGauche*multiplicateurY))+'" style="stroke:rgb(0,0,0);stroke-width:2;opacity:0.3;"/>'
		image = '<svg width="100%" height="100%" version="1.1" xmlns="http://www.w3.org/2000/svg">'+image+graphique+'</svg> '
		document.getElementById("affichage").innerHTML = image
		//alert(image)
	}

// ---- SVG (images multiples) ----
// Créé les différents éléments pour dessiner la fonction mathématique
// et les place dans la div "affichage"
	function calculerGraphSVG(fin){
		document.getElementById("affichage").innerHTML = ""
		svg = document.createElementNS("http://www.w3.org/2000/svg", "svg")
		svg.setAttribute("width", "100%")
		svg.setAttribute("height", "100%")
		for (i=1; i<fin; i++){
			if ((pointY[i]<0) && (pointY[i+1]>hauteur)){
				i++
			}
			if ((pointY[i]>hauteur) && (pointY[i+1]<0)){
				i++
			}
			var ligne = document.createElementNS("http://www.w3.org/2000/svg", "line")
			ligne.setAttribute("x1", pointX[i]+decalageX)
			ligne.setAttribute("x2", pointX[i+1]+decalageX)
			ligne.setAttribute("y1", pointY[i]+decalageY)
			ligne.setAttribute("y2", pointY[i+1]+decalageY)
			ligne.setAttribute("stroke", couleurFonction)
			ligne.setAttribute("stroke-width", lineWidth)
			svg.appendChild(ligne)
		}

	// dérivée
	if(document.getElementById("checkDerivee").checked){
		for (i=1; i<fin; i++){
			var ligne = document.createElementNS("http://www.w3.org/2000/svg", "line")
			ligne.setAttribute("x1", pointX[i]+decalageX)
			ligne.setAttribute("x2", pointX[i+1]+decalageX)
			ligne.setAttribute("y1", pente[i]+decalageY)
			ligne.setAttribute("y2", pente[i+1]+decalageY)
			ligne.setAttribute("stroke", "white")
			ligne.setAttribute("stroke-width", lineWidth)
			ligne.setAttribute("opacity", "0.8")
			svg.appendChild(ligne)
		}
	}

	if(document.getElementById("checkAxes").checked){
		calculerAxes()
	}
	calculerGrilleEchelle()
		document.getElementById("affichage").appendChild(svg)
	}

	// axes
	function calculerAxes(){
		var ligne = document.createElementNS("http://www.w3.org/2000/svg", "line")
		ligne.setAttribute("x1", (-borneXGauche*multiplicateurX)+decalageX)
		ligne.setAttribute("y1", 0+decalageY)
		ligne.setAttribute("x2", (-borneXGauche*multiplicateurX)+decalageX)
		ligne.setAttribute("y2", hauteur+decalageY)
		ligne.setAttribute("stroke", "rgb(0,0,0)")
		ligne.setAttribute("stroke-width", "2")
		ligne.setAttribute("opacity", "0.3")
		svg.appendChild(ligne)

		var ligne = document.createElementNS("http://www.w3.org/2000/svg", "line")
		ligne.setAttribute("x1", 0+decalageX)
		ligne.setAttribute("y1", (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
		ligne.setAttribute("x2", largeur+decalageX)
		ligne.setAttribute("y2", (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
		ligne.setAttribute("stroke", "rgb(0,0,0)")
		ligne.setAttribute("stroke-width", "2")
		ligne.setAttribute("opacity", "0.3")
		svg.appendChild(ligne)
	}
	
	// grille et échelle
	function calculerGrilleEchelle(){
		var intervalX = Math.round(Math.abs(borneXGauche-borneXDroite)/10)
		var intervalY = Math.round(Math.abs(borneYGauche-borneYDroite)/10)
		var initialX = Math.round(-borneXGauche) % intervalX
		var initialY = Math.round(-borneYGauche) % intervalY
		for(var i=initialX;i<=Math.round(borneXDroite-borneXGauche);i=i+intervalX){
			var position = Math.round((Math.round(borneXGauche) - borneXGauche +i) * multiplicateurX)
			if(document.getElementById("checkGrille").checked){
				var grille = document.createElementNS("http://www.w3.org/2000/svg", "line")
				grille.setAttribute("x1", position)
				grille.setAttribute("y1", 0)
				grille.setAttribute("x2", position)
				grille.setAttribute("y2", hauteur)
				grille.setAttribute("stroke", "rgb(0,0,0)")
				grille.setAttribute("stroke-width", "2")
				grille.setAttribute("opacity", "0.05")
				svg.appendChild(grille)
			}

			if(document.getElementById("checkEchelle").checked){
				var txt = document.createElementNS("http://www.w3.org/2000/svg", "text")
				txt.appendChild(document.createTextNode(i+borneXGauche))
				txt.setAttribute("x", position-6)
				txt.setAttribute("y", (hauteur-(-borneYGauche*multiplicateurY))+decalageY-2)
				txt.setAttribute("fill-opacity", 0.6)
				txt.setAttribute("fill", couleurEchelle)
				svg.appendChild(txt)
			}
		}
		for(var i=initialY;i<=Math.round(borneYDroite-borneYGauche);i=i+intervalY){
			var position = hauteur - Math.round((Math.round(borneYGauche) - borneYGauche +i) * multiplicateurY)
			if(document.getElementById("checkGrille").checked){
			var grille = document.createElementNS("http://www.w3.org/2000/svg", "line")
			grille.setAttribute("x1", 0)
			grille.setAttribute("y1", position)
			grille.setAttribute("x2", largeur)
			grille.setAttribute("y2", position)
			grille.setAttribute("stroke", "rgb(0,0,0)")
			grille.setAttribute("stroke-width", "2")
			grille.setAttribute("opacity", "0.05")
			svg.appendChild(grille)
			}

			if(document.getElementById("checkEchelle").checked){
			var txt = document.createElementNS("http://www.w3.org/2000/svg", "text")
			txt.appendChild(document.createTextNode(i+borneYGauche))
			txt.setAttribute("x", (-borneXGauche*multiplicateurX)+decalageX+2)
			txt.setAttribute("y", position+6)
			txt.setAttribute("fill-opacity", 0.6)
			txt.setAttribute("fill", couleurEchelle)
			svg.appendChild(txt)
			}
		}
	}

// ---- Canevas ----
// Permet de générer le code HTML nécessaire à canvas et dessine la fonction
// à l'aide de lignes dans la zone de canvas.
	function calculerGraphCanevas(fin){
		document.getElementById("affichage").innerHTML = '<canvas id="canvas" width="'+largeur+'" height="'+hauteur+'"></canvas>'
		ctx = document.getElementById('canvas').getContext('2d')
		var undefined = true
		// Autres fonctions
		for(var i=0; i<listeFonctions.length; i++){
			ctx.beginPath()
			x = borneXGauche-3*precision
			y = eval(listeFonctions[i])
			if(isNaN(y)){
				ctx.moveTo(x, 0)
			}
			else{
				ctx.moveTo(x, y)
			}
			ctx.strokeStyle = listeCouleurs[i]
			ctx.fillStyle = listeCouleurs[i]
			ctx.lineWidth = lineWidth
			for(x=borneXGauche-2*precision; x<=(borneXDroite+2*precision); x+=precision){
				y = eval(listeFonctions[i])
				if(!isNaN(y)){
					ctx.lineTo((x - borneXGauche) * multiplicateurX+decalageX, hauteur - ((y - borneYGauche) * multiplicateurY)+decalageY)
					/*pente[i] = hauteur - ((10*(y-y1) - borneYGauche) * multiplicateurY)*/
				}
			}
			ctx.stroke()
		}

		// aire sous la fonctions
		if(document.getElementById("checkAire").checked){
			ctx.strokeStyle = "rgba(255,255,255,0)";
			ctx.fillStyle = "rgba(0,180,255,0.3)";
			var a = parseInt(document.getElementById("aireG").value);
			var b = parseInt(document.getElementById("aireD").value);
			var fct = document.getElementById("inputEq").value;
			var f = function(x){
				return eval(fct);
			};
			var convertX = function(nbr){
				return (nbr - borneXGauche) * multiplicateurX;
			};
			var convertY = function(nbr){
				return hauteur - ((nbr - borneYGauche) * multiplicateurY)
			};
			var n = 5000/(borneXDroite-borneXGauche);
			var aire, largeurRect, gaucheRect, droiteRect, millieuRect, hauteurRect, aireRect;
			aire = 0;
			largeurRect = (b-a)/n;
			for(var i=0; i<n; i++){
				gaucheRect = a + i*largeurRect;
				droiteRect = a + (i+1)*largeurRect;
				millieuRect = (gaucheRect+droiteRect) / 2;
				hauteurRect = f(millieuRect);
				//aireRect = largeurRect * hauteurRect;
				//aire = aire + aireRect;
				try{
					ctx.fillRect(convertX(gaucheRect), convertY(hauteurRect), 2, hauteurRect*multiplicateurY);
				}
				catch(err){
					
				}
			}
		}

		// fonction
		ctx.beginPath()
		//ctx.moveTo(pointX[1]+decalageX, pointY[1]+decalageY)
		ctx.strokeStyle = couleurFonction
		ctx.fillStyle = couleurFonction
		ctx.lineWidth = lineWidth
		if(document.getElementById("selectMethodeAffichage").value == "canvas"){
			for (i=1; i<fin; i++){
				if(isNaN(pointX[i]) || isNaN(pointY[i])){
					undefined = true
				}
				else{
					if(undefined){
						ctx.moveTo(pointX[i]+decalageX, pointY[i]+decalageY)
					}
					undefined = false
					ctx.lineTo(pointX[i]+decalageX, pointY[i]+decalageY)
				}
			}
			ctx.stroke()
		}
		else{
			for (i=1; i<fin; i++){
				if(!isNaN(pointX[i]) && !isNaN(pointY[i])){
					ctx.moveTo(pointX[i]+decalageX, pointY[i]+decalageY)
					ctx.arc(pointX[i]+decalageX, pointY[i]+decalageY, lineWidth/2, 0, 2*Math.PI, false)
				}
			}
			ctx.fill()
			//ctx.stroke()
		}

		// dérivée première
		if(document.getElementById("checkDerivee").checked){
			undefined = true
			ctx.beginPath()
			ctx.strokeStyle = "rgba(255, 255, 255, 0.8)"
			ctx.fillStyle = "rgba(255, 255, 255, 0.8)"
			ctx.lineWidth = lineWidth
			//ctx.moveTo(pointX[1]+decalageX, pente[1]+decalageY)
			for (i=1; i<fin; i++){
				if(isNaN(pointX[i]) || isNaN(pente[i])){
					undefined = true
				}
				else{
					if(undefined){
						try{
							ctx.moveTo(pointX[i]+decalageX, pente[i]+decalageY)
						}
						catch(err){
							//alert(";"+pente[i]+" ; "+pointX[i])
						}
					}
					undefined = false
					try{
						ctx.lineTo(pointX[i]+decalageX, pente[i]+decalageY)
					}
					catch(err){
						//alert(pente[i]+" ; "+pointX[i])
					}
				}
			}
			ctx.stroke()
		}

		// dérivée seconde
		if(document.getElementById("checkDerivee2").checked){
			undefined = true
			ctx.beginPath()
			ctx.strokeStyle = "rgba(150, 150, 150, 0.8)"
			ctx.fillStyle = "rgba(150, 150, 150, 0.8)"
			ctx.lineWidth = lineWidth
			//ctx.moveTo(pointX[1]+decalageX, pente2[1]+decalageY)
			for (i=1; i<fin-1; i++){
				if(isNaN(pointX[i]) || isNaN(pente2[i])){
					undefined = true
				}
				else{
					if(undefined){
						try{
							ctx.moveTo(pointX[i+2]+decalageX, pente2[i+2]+decalageY)
						}
						catch(err){
							
						}
					}
					undefined = false
					try{
						ctx.lineTo(pointX[i+2]+decalageX, pente2[i+2]+decalageY)
					}
					catch(err){
						
					}
				}
			}
			ctx.stroke()
		}

		// grille et échelle
		var intervalX = Math.round(Math.abs(borneXGauche-borneXDroite)/10)
		var intervalY = Math.round(Math.abs(borneYGauche-borneYDroite)/10)
		var initialX = Math.round(-borneXGauche) % intervalX
		var initialY = Math.round(-borneYGauche) % intervalY
		ctx.beginPath()
		ctx.fillStyle = couleurEchelle
		ctx.lineWidth = "2"
		for(var i=initialX;i<=Math.round(borneXDroite-borneXGauche);i=i+intervalX){
			var position = Math.round((Math.round(borneXGauche) - borneXGauche +i) * multiplicateurX)
			if(document.getElementById("checkGrille").checked){
				ctx.moveTo(position, 0)
				ctx.lineTo(position, hauteur)
			}

			if(document.getElementById("checkEchelle").checked){
				ctx.fillText(Math.ceil(i+borneXGauche), position-6, (hauteur-(-borneYGauche*multiplicateurY))+decalageY-2)
			}
		}
		for(var i=initialY;i<=Math.round(borneYDroite-borneYGauche);i=i+intervalY){
			var position = hauteur - Math.round((Math.round(borneYGauche) - borneYGauche +i) * multiplicateurY)
			if(document.getElementById("checkGrille").checked){
				ctx.moveTo(0, position)
				ctx.lineTo(largeur, position)
			}

			if(document.getElementById("checkEchelle").checked){
				ctx.fillText(Math.ceil(i+borneYGauche), (-borneXGauche*multiplicateurX)+decalageX+2, position+6)
			}
		}
		ctx.strokeStyle = couleurGrille
		ctx.stroke()

		// axes
		if(document.getElementById("checkAxes").checked){
			ctx.beginPath()
			ctx.strokeStyle = couleurAxes
			ctx.lineWidth = "2"
			ctx.moveTo((-borneXGauche*multiplicateurX)+decalageX, 0+decalageY)
			ctx.lineTo((-borneXGauche*multiplicateurX)+decalageX, hauteur+decalageY)

			ctx.moveTo(0+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
			ctx.lineTo(largeur+decalageX, (hauteur-(-borneYGauche*multiplicateurY))+decalageY)
			ctx.stroke()
		}
	}
