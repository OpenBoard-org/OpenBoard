var weighValues = [500,200,100,50,20,10];

function onTemplateLoadedCallback(app) {
	$.each(weighValues, function(){
		$("#weights").append($(Mustache.render(weightTemplate, {weight: this})));
	});
	
	$("#weights > .weight").draggable({helper: "clone"});	

	$("#leftScale").droppable({
		accept: ".object",
		drop: function(event, ui) {
			if($(ui.draggable).hasClass("inScale"))
				return;

			var object = $(ui.draggable).clone();
			object.data("weight", $(ui.draggable).data("weight"));

			object.addClass("inScale");
			$("#leftScale").append(object);
			
			placeObject($("#leftScale"), object);

			refreshScales();

			object.draggable({
				stop: function(event, ui) {
					if($(ui.helper).hasClass("onOut")) {
						$(ui.helper).remove();
						refreshScales();
					}
				}
			});
		},
		out: function(event, ui) {
			$(ui.draggable).addClass("onOut");
		},
		over: function(event, ui) {
			$(ui.draggable).removeClass("onOut");
		}
	});

	$("#rightScale").droppable({
		accept: ".weight.right",
		drop: function(event, ui) {
			if($(ui.draggable).hasClass("inScale"))
				return;
			var object = createWeight(app, $(ui.draggable).data("weight"));
			
			$("#rightScale").append(object);

			placeObject($("#rightScale"), object);

			refreshScales();			
		},
		out: function(event, ui) {
			$(ui.draggable).addClass("onOut");
		},
		over: function(event, ui) {
			$(ui.draggable).removeClass("onOut");
		}
	});

	$("#objects button[role=add]").click(function(){
		var object = createObject(app);
		$(this).before(object);
	});
}

function addObjectWithId(app, id) {
	var objects = app.parameters.value("Objects");
	app.parameters.value("Objects", objects === undefined ? id : objects+","+id);
}

function removeObjectWithId(app, id) {
	var objects = app.parameters.value("Objects");
	objects = objects.split(",");
	for(var i=0 ; i<objects.length ; i++) {
		if(id === objects[i]) {
			objects.splice(i, 1);
			break;
		}
	}
	app.parameters.value("Objects", objects.join(","));	
}

function addWeightWithId(app, id) {
	var objects = app.parameters.value("Weights");
	app.parameters.value("Weights", objects === undefined ? id : objects+","+id);
}

function removeWeightWithId(app, id) {
	var objects = app.parameters.value("Weights");
	objects = objects.split(",");
	for(var i=0 ; i<objects.length ; i++) {
		if(id === objects[i]) {
			objects.splice(i, 1);
			break;
		}
	}
	app.parameters.value("Weights", objects.join(","));	
}

function objectForGUID(app, guid) {
	window.object = guid;
	var objectUi = $(Mustache.render(objectTemplate, window));
	var weight = getWeightFor(app.parameters, guid);
	
	if(!app.onEdit && (weight == undefined || $.trim(weight) == ""))
		weight = weighValues[Math.floor(Math.random()*weighValues.length)];
	
	if(weight !== undefined) {
		objectUi.data("weight", weight);
		objectUi.find("input[name=weight]").val(weight);
	}
	if(app.onEdit) {
		objectUi.find("input[name=weight]").change(function(event) {
			var o = objectUi;
			o.data("weight", $(this).val());
			setWeightFor(app.parameters, o.attr("id"), $(this).val());
		});
		objectUi.find("button[role=remove]").click(function(){
			removeObjectWithId(app, guid);	
			objectUi.remove();
		});
		objectUi.find("button[role=duplicate]").click(function(){
			$("#objects button[role=add]").before(createObject(app, guid));	
		});
	}else{
		objectUi.find("input[name=weight]").attr("disabled","disabled");
		objectUi.draggable({
			containment: "#scene",
			scroll: false,
			helper: "clone"
		});
	}
	return objectUi;
}

function weightForGUID(app, guid) {
	var weight = getWeightValueFor(app.parameters, guid);
	window.object = guid;
	window.weight = weight;
	var objectUi = $(Mustache.render(weightTemplate, window));
	var position = getPositionFor(app.parameters, guid);
	objectUi.css("left", position.left);
	objectUi.css("top", position.top);
	objectUi.addClass("inScale");
	if(app.onEdit) {
		objectUi.draggable({
			stop: function(event, ui) {
				if($(ui.helper).hasClass("onOut")) {
					removeWeightWithId(app, $(ui.helper).attr('id'));
					$(ui.helper).remove();
					refreshScales();
				}else{
					var id = ui.helper.attr("id");
					var left = ui.position.left ;
					var top = ui.position.top;
					log(left+"-"+top);
					setPositionFor(app.parameters, id, ui.position);
				}
			}
		});
	}else{
	}
	return objectUi;
}

function placeObject($container, $object) {

	var width = $object.outerHeight();
	var height = $object.outerHeight()+15;

	var count = $container.children().size() - 1;

	var left = width * (count % 4) + 5;
	var bottom = height * (Math.floor(count / 4)) + 12;
	
	log("Place at ["+left+" , "+bottom+"]");
	$object.css("left", left+"px").css("bottom", bottom+"px");
}

function createObject(app, duplicateID) {
	var id = app.utils.guid();
	addObjectWithId(app, id);
	
	//TODO Duplication : Recupérer les images du dupliqué
		
	return objectForGUID(app, id);
}

function createWeight(app, weight) {
	var id = app.utils.guid();
	addWeightWithId(app, id);	
	setWeightValueFor(app.parameters, id, weight);
	return weightForGUID(app, id);
}

function getWeightFor(parameters, id) {
 	return parameters.value("Object#"+id+"Weight");
}

function setWeightFor(parameters, id, weight) {
	parameters.value("Object#"+id+"Weight", weight);
}

function getWeightValueFor(parameters, id) {
 	return parameters.value("Weight#"+id+"Weight");
}

function setWeightValueFor(parameters, id, weight) {
	parameters.value("Weight#"+id+"Weight", weight);
}

function getPositionFor(parameters, id) {
	var left = parameters.value("Weight#"+id+"left");
	var top = parameters.value("Weight#"+id+"top");
	return {left: parseInt(left), top: parseInt(top)};
}

function setPositionFor(parameters, id, position) {
	parameters.value("Weight#"+id+"left", position.left);
	parameters.value("Weight#"+id+"top", position.top);
}

function reloadApp(app) {
	$("#leftScale, #rightScale").empty();
	$("#objects > .object").remove();
	
	refreshScales();
	
	var objects = app.parameters.value("Objects");
	if(objects === undefined) {
		var object = createObject(app);	
		setWeightFor(app.parameters, object.attr("id"), 5);
		objects = app.parameters.value("Objects")
	}
	if(objects !== undefined) {
		objects = objects.split(",");
		for(var i=0 ; i<objects.length ; i++) {
			var id = objects[i];
			var object = objectForGUID(app, id);
			$("#objects button[role=add]").before(object);
		}
	}
	
	var weights = app.parameters.value("Weights");
	if(weights === undefined) {
		createWeight(app, 20);
		weights = app.parameters.value("Weights");
	}
	if(weights !== undefined) {
		weights = weights.split(",");
		for(var i=0 ; i<weights.length ; i++) {
			var id = weights[i];
			var weight = weightForGUID(app, id);
			$("#rightScale").append(weight);
			placeObject($("#rightScale"), weight);
		}
	}
	
	refreshScales();
	
	if(app.onEdit) {
		
	}
}

function refreshScales() {
	var leftWeight = getLeftWeight();
	var rightWeight = getRightWeight();
	var dWeight = rightWeight - leftWeight;
	var angle = maxAngle * dWeight / maxWeight;
	angle = angle >= 0 ? Math.min(maxAngle, angle) : Math.max(-maxAngle, angle);
	var rad = angle * Math.PI / 180;

	var w = $("#tray").width()/2;
	var dx = w - (w * Math.cos(rad));
	var dy = -(w * Math.sin(rad));

	log("Rotate to "+angle);
	log("Tanslate to ["+dx+" , "+dy+"]");

	var rotate = "rotate("+angle+"deg)";
	var leftTranslate = "translate("+dx+"px, "+dy+"px)";
	var rightTranslate = "translate("+(-dx)+"px, "+(-dy)+"px)";
	
	$("#tray").css("transform", rotate);
	$("#leftScale").css("transform", leftTranslate);
	$("#rightScale").css("transform", rightTranslate);
}

function getWeightForSelector(selector) {
	var weight = 0;
	$(selector).each(function(){
		var temp = $(this).data("weight");
		if(temp !== undefined)
			weight += parseInt(temp);
	});
	return weight;
}

function getLeftWeight() {
	return getWeightForSelector("#leftScale > div");
}

function getRightWeight() {
	return getWeightForSelector("#rightScale > div");
}

function reloadCallback(parameter) {
	if(parameter === undefined)
		reloadApp(this);
}

var maxAngle;
var maxWeight;

$(document).ready(function(){
	var w = $("#scales").width();
	var h = $("#scales").height();
	maxAngle = 17;//Math.atan((h/2)/(w/2)) * (180/Math.PI);
	maxWeight = 10;
	
	log(w+"-"+h+":"+maxAngle);
	
	var callbacks = {
		onTemplatesLoaded: onTemplateLoadedCallback,
		onEdit: reloadApp,
		onView: reloadApp
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);
});