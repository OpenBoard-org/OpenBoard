
var souris = {
	active : false,
	xInit : 0,
	yInit : 0,
	
	down : function(event){
		var x = event.clientX;
		var y = event.clientY;
		this.active = true;
		this.xInit = x;
		this.yInit = y;
		if(event.button != 2 && !ctxMenu.ouvert){
			outil.down(x, y);
		}
		ctxMenu.fermer();
	},
	up : function(){
		this.active = false;
	},
	move : function(event){
		var x = event.clientX;
		var y = event.clientY;
		outil.move(x, y, souris.xInit, souris.yInit, souris.active);
	},
	out : function(event){
		outil.dessinerListe();
		if(event.relatedTarget && event.relatedTarget.className != "flecheDeplacement"){
			this.active = false;
		}
	},
	wheel : function(event){
		if(!event) event = window.event;
		if(event.wheelDelta){
			if(event.wheelDelta < 0){
				affichage.zoom(1.25);
			}
			else{
				affichage.zoom(0.8);
			}
		}
		else if(event.detail){
			if(event.detail > 0){
				affichage.zoom(1.25);
			}
			else{
				affichage.zoom(0.8);
			}
		}
	},
	dblClick : function(event){
		if(event.ctrlKey){
			affichage.zoom(1.25)
			display3D.zoom(1.25)
		}
		else{
			affichage.zoom(0.8)
			display3D.zoom(0.8)
		}
	}
}

var ctxMenu = {
	id : "ctxMenu",
	ouvert : false,
	
	ouvrir : function(){
		if(!fonction3D){
			ctxMenu.ouvert = true;
			
			var element = document.getElementById("ctxMenu");
			var x = souris.xInit;
			var y = souris.yInit;
	// 		var x = 300;
	// 		var y = 300;
			element.style.display = "block";
			element.style.left = (x+1)+"px";
			element.style.top = (y+1)+"px";
			
			// Désactive le menu du navigateur
			return false;
		}
	},
	
	fermer : function(){
		ctxMenu.ouvert = false;
		
		var element = document.getElementById("ctxMenu");
		element.style.display = "none";
	}
}

