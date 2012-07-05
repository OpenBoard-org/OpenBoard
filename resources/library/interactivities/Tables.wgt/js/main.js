function head(text) {
	var c = $("<div/>");
	c.addClass("head");	
	c.text(text);
	return c;
}
function cell(text) {
	if(isNaN(text) || text === Infinity)
		text = "-";
	else
		text = parseInt(text);
	var c = $("<div/>");
	c.text(text);
	return c;
}

function onTemplateLoadedCallback() {
	$("select[name=row], select[name=col]").each(function(i, select) {
		var min = $(select).data("min");
		var max = $(select).data("max");
		for(var i = min ; i <= max ; i++) {
			var option = $("<option/>");
			option.val(i);
			option.text(i);
			$(select).append(option);
		}
	});
}

function reloadCallback(parameters) {
	var row = parseInt(this.parameters.value("row"));
	var col = parseInt(this.parameters.value("col"));
	var operator = this.parameters.value("operator");
	var showReste = this.parameters.value("reste");
	showReste = typeof showReste === "string" ? showReste === "true" : showReste;
	
	var scene = $("#scene");
	var table = $("<div id='table'/>");
	scene.empty();
	scene.append(table);
				
	for(var i=-1 ; i<=row ; i++) {
		var r = $("<div/>");
		table.append(r);
							
		if(i!=-1) {
			var rh = $("<div/>");
			rh.append(head(i));
			r.append(rh);
		}else{
			var op = $("<div/>");
			op.append("<div id='lecture'/>");
			op.append(head(operator));
			r.append(op);
		}

		for(var j=0 ; j<=col ; j++) {
			var c = $("<div/>");
			r.append(c);
			
			if(i==-1) {
				c.append(head(j));
			}else{
				c.addClass("card-container");
				var card = $("<div class='card'/>");
				c.append(card);
				card.append(cell("?"));					
				var compute = cell(eval(i+operator+j));
				card.append(compute);
				if(operator === "/" && showReste)
					compute.append(cell(eval(i%j)));
			}
		}
		var c = $("<div/>");
		r.append(c);
	}
	
	var r = $("<div/>");
	table.append(r);
	for(var j=0 ; j<=col+2 ; j++) {
		var c = $("<div/>");
		r.append(c);
	}
	
	$(".card>div").click(function(){
		$(this).parent().toggleClass("flip");
	});
}

$(document).ready(function(){
	var callbacks = {
		onTemplatesLoaded: onTemplateLoadedCallback
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);
});