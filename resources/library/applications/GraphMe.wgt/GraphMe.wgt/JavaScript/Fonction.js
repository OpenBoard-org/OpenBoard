
var fct = {
	list : [],
	couleur : "rgba(193,255,0,1)",
	colorsList : ["rgba(255,0,0,1)","rgba(255,128,0,1)","rgba(255,255,0,1)","rgba(128,255,0,1)","rgba(0,255,0,1)","rgba(0,255,128,1)","rgba(0,255,255,1)","rgba(0,128,255,1)","rgba(0,0,255,1)","rgba(128,0,255,1)","rgba(255,0,255,1)","rgba(255,0,128,1)"],
	interdit : [";", "interdit", "'", '"', "eval", "new", "uniboard", "sankore", "=", "document", "window", "alert", "fct", "affichage", "colorPicker", "languages"],
	
	functionFromObject: function(f){
		if(f.type == "cartesian"){
			return new CartesianFunction().set(f);
		}
		else if(f.type == "implicit"){
			return new ImplicitFunction().set(f);
		}
		else if(f.type == "polar"){
			return new PolarFunction().set(f);
		}
		else if(f.type == "parametric"){
			return new ParametricFunction().set(f);
		}
		console.err("Unknown function:", f);
	},
	
	addCartesian : function(txt){
		var txtFct = this.remplacer(this.verifier(txt));
		if(txtFct == ""){
			return false;
		}
		
		var f = new CartesianFunction(txtFct);
		this.add(f, true);
		
		return true;
	},
	
	addImplicit : function(txtFct){
		var equalPos = txtFct.indexOf("=");
		if(equalPos >= 0){
			txtFct = txtFct.replace("=", "-(") + ")";
		}
		txtFct = this.remplacer(this.verifier(txtFct));
		if(txtFct == ""){
			return false;
		}
		
		var f = new ImplicitFunction(txtFct);
		this.add(f, true);
		
		return true;
	},
	
	addPolar : function(txt){
		var txtFct = this.remplacer(this.verifier(txt));
		if(txtFct == ""){
			return false;
		}
		
		var f = new PolarFunction(txtFct);
		this.add(f, true);
		
		return true;
	},
	
	addParametric : function(txtX, txtY){
		var fctX = this.remplacer(this.verifier(txtX));
		var fctY = this.remplacer(this.verifier(txtY));
		if(fctX == "" || fctY == ""){
			return false;
		}
		
		var f = new ParametricFunction(fctX, fctY);
		this.add(f, true);
		
		return true;
	},
	
	add : function(f, useRandomColor){
		this.list.unshift(f);
		if(useRandomColor){
			f.couleur = this.colorsList[alea(0,this.colorsList.length-1)];
		}
		historique.ajouter(f);
		this.updateList();
		editeur.editer(0);
	},
	
	ajouter : function(){
		var type = document.getElementById("functionType").value;
		if(type == "cartesian"){
			var input = document.getElementById("input");
			if(this.addCartesian(input.value)){
				input.value = "";
			}
		}
		else if(type == "implicit"){
			var input = document.getElementById("implicitInput");
			if(this.addImplicit(input.value)){
				input.value = "";
			}
		}
		else if(type == "polar"){
			var input = document.getElementById("polarInput");
			if(this.addPolar(input.value)){
				input.value = "";
			}
		}
		else if(type == "parametric"){
			var inputX = document.getElementById("parametricInputX");
			var inputY = document.getElementById("parametricInputY");
			if(this.addParametric(inputX.value, inputY.value)){
				inputX.value = "";
				inputY.value = "";
			}
		}
	},
	
	changeType : function(type){
		document.getElementById("cartesianDiv").style.display = "none";
		document.getElementById("implicitDiv").style.display = "none";
		document.getElementById("polarDiv").style.display = "none";
		document.getElementById("parametricDiv").style.display = "none";
		document.getElementById(type+"Div").style.display = "initial";
	},
	
	enlever : function(id){
		this.list.splice(id, 1);
		this.updateList();
	},
	
	dupliquer : function(id){
		var newFct = this.functionFromObject(this.list[id].get());
		newFct.couleur = this.colorsList[alea(0,this.colorsList.length-1)];
		this.list.splice(id+1, 0, newFct);
		this.updateList();
	},
	
	etudier : function(id){
		etude.etudier(this.list[id].fct);
		afficherMenu("menuEtude");
	},
	
	updateList : function(){
		var texteFctSupp = "";
		for(var i=0; i<this.list.length; i++){
			var className = "spanFonction";
			if(i == editeur.idFct){
				className += " spanFonctionSelect";
			}
			texteFctSupp += '<div class="'+className+'" onclick="editeur.editer('+i+')" style="text-shadow: 0px 0px 3px '+this.list[i].couleur+'">'+this.list[i].fct+'</div>';
		}
		document.getElementById("fonctionsSupp").innerHTML = texteFctSupp;
		affichage.dessiner();
		saveOptions();
	},
	
	verifier : function(txtFonction){
		for(var i=0; i<this.interdit.length; i++){
			var condition = "";
			for(var k=0; k<this.interdit[i].length; k++){
				if(k==0){
					condition = condition + "txtFonction.charAt(j) == this.interdit[i].charAt(0)";
				}
				else{
					condition = condition + "&& txtFonction.charAt(j+"+k+") == this.interdit[i].charAt("+k+")";
				}
			}
			for(var j=0; j<txtFonction.length; j++){
				if(eval(condition)){
					document.getElementById("spanFctInterdite").innerHTML = this.interdit[i];
					afficherMenu("fctInterdite");
					return 0;
				}
			}
		}
		return txtFonction;
	},
	
	remplacer : function(txtFonction){
		// Remplace a^b par pow(a, b)
		var start = 0;
		var end = 0;
		var pos1, pos2;

		while(true){
// 			start = end;
			end = txtFonction.indexOf("^",0);
			if(end != -1){
				pos1 = txtFonction.getLastIndexOf(["+", "-", "*", "/", "%", "(", ","], start, end);
				if(pos1 == -1){
					pos1 = start-1;
				}
				
				pos2 = txtFonction.getIndexOf(["+", "-", "*", "/", "%", "^"], end+1);
				if(pos2 == -1){
					pos2 = txtFonction.length;
				}
				
				txtFonction = txtFonction.substring(0,pos1+1)+"pow("+txtFonction.substring(pos1+1,end)+","+txtFonction.substring(end+1,pos2)+")"+txtFonction.substring(pos2,txtFonction.length);
// 				alert(txtFonction)
			}
			else{
				break;
			}
		}
		return txtFonction;
	},
	
	get : function(){
		var list = [];
		for(var i=0; i<this.list.length; i++){
			list.push(this.list[i].get());
		}
		return {list: list};
	},
	
	set: function(obj){
		var list = obj.list;
		for(var i=0; i<list.length; i++){
			this.list.push(fct.functionFromObject(list[i]));
		}
		this.updateList();
		if(this.list.length > 0){
			editeur.editer(0);
		}
	}
};
