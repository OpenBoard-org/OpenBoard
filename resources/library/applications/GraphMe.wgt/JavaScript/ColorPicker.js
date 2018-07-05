
// -------------------- Color Picker --------------------

var idColor = ""

// Différentes fonctions nécessaire au Color Picker (menu du choix de la couleur)
function colorSV(e){
	var element = document.getElementById('colorSV')
	var posDivY = 0
	var posDivX = 0
	while(element){
		posDivY = posDivY + element.offsetTop
		posDivX = posDivX + element.offsetLeft
		element = element.offsetParent
	}
	document.getElementById("info").innerHTML = posDivX+" ; "+posDivY
	S = 100-Math.round((e.clientY - posDivY -0)/255*100)
	V = Math.round((e.clientX - posDivX -0)/255*100)
	document.getElementById("inputValeur").value = V
	document.getElementById("inputSaturation").value = S
	document.getElementById("ligneValeur").setAttribute("x1", e.clientX-posDivX)
	document.getElementById("ligneValeur").setAttribute("x2", e.clientX-posDivX)
	document.getElementById("ligneSaturation").setAttribute("y1", e.clientY-posDivY)
	document.getElementById("ligneSaturation").setAttribute("y2", e.clientY-posDivY)
	colorRGB()
}

function colorT(e){
	var element = document.getElementById('colorT')
	var posDiv = 0
	while(element){
		posDiv = posDiv + element.offsetTop
		element = element.offsetParent
	}
	T = (e.clientY - posDiv -0) / 256*360
	r,g,b = 0
	while (true){
		if (T<60){
			r = 255
			g = T/60*255
			b = 0
			break
		}
		if (T<=120){
			r = (255-(T%60/60*255))%256
			g = 255
			b = 0
			break
		}
		if (T<180){
			r = 0
			g = 255
			b = T%60/60*255
			break
		}
		if (T<=240){
			r = 0
			g = (255-(T%60/60*255))%256
			b = 255
			break
		}
		if (T<300){
			r = T%60/60*255
			g = 0
			b = 255
			break
		}
		if (T<360){
			r = 255
			g = 0
			b = (255-(T%60/60*255))%256
			break
		}
		r = 255
		g = 0
		b = 0
		break
	}
	T = Math.round(T)
	r = Math.round(r)
	g = Math.round(g)
	b = Math.round(b)
	document.getElementById("inputTeinte").value = T
	document.getElementById("colorSV").style.backgroundColor = "rgb("+r+","+g+","+b+")"
	document.getElementById("ligneTeinte").setAttribute("y2", e.clientY-posDiv)
	document.getElementById("ligneTeinte").setAttribute("y1", e.clientY-posDiv)
	colorRGB()
}

function colorRGB(){
	var S = document.getElementById("inputSaturation").value
	var V = document.getElementById("inputValeur").value
	var rouge = Math.round((r + (255-r) * (-1) * (S-100) / 100 )* V / 100)
	var vert = Math.round((g + (255-g) * (-1) * (S-100) / 100 )* V / 100)
	var bleu = Math.round((b + (255-b) * (-1) * (S-100) / 100 )* V / 100)
	document.getElementById("inputRouge").value = rouge
	document.getElementById("inputVert").value = vert
	document.getElementById("inputBleu").value = bleu
	eval(document.getElementById(idColor).title+' = "rgba(" + rouge +", " + vert + ", " + bleu +", "+document.getElementById("inputOpacity").value+")"')
	document.getElementById(idColor).style.backgroundColor = eval(document.getElementById(idColor).title)
	document.getElementById("apercuCouleur").style.backgroundColor = eval(document.getElementById(idColor).title)
}

function colorPicker(id){
	idColor = id
	document.getElementById("apercuCouleur").style.backgroundColor = eval(document.getElementById(idColor).title)
	document.getElementById("apercuCouleur2").style.backgroundColor = eval(document.getElementById(idColor).title)
}
