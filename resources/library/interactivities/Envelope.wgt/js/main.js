function onTemplateLoadedCallback(app) {
	/*
	$("button[role=addObject]").click(function() {
		createObject(app);
	});
	*/
}

function reloadApp(app) {
	$("#objects").empty();
	var count = parseInt(app.parameters.value("count"));
	
	for(var i=0 ; i<count ; i++) {
		var objectUi = $(Mustache.render(objectTemplate, window));
		objectUi.draggable({
			containment: "#scene",
			scroll: false,
			revert: app.onEdit ? false : "invalid"
		});
		$("#objects").append(objectUi);
	}
	
	if(!app.onEdit) {
		$("#box").droppable({
			hoverClass: "ui-state-hover",
			activeClass: "ui-state-active",  
			drop: function(event, ui) {
				ui.draggable.addClass("inBox");
			}
		});
		$("#box").click(function(){
			$(this).addClass("onReset");
			var objects = $("#objects .inBox");
			var n = objects.size();
			if(n==0) {
				$(this).removeClass("onReset");
			}else {
				objects.each(function(i, o){
					$(this).removeClass("inBox");
					$(this).addClass("hasBeenInBox");
					$(this).animate({
						left: null,
						top: null
					}, function(){
						n--;
						if(n==0) {
							$("#box").removeClass("onReset");
						}
					});
				});
			}
		});
	}
	
	/**
	* Ce code est pour la version objet personnalisable.
	* L'app a été simplifié
	*/
	/*
	var objects = app.parameters.value("Objects");
	
	if(objects !== undefined) {
		objects = objects.split(",");
		for(var i=0 ; i<objects.length ; i++) {
			var id = objects[i];
			objectForGUID(app, id, getPositionFor(app.parameters, id));
		}
	}
	
	if(!app.onEdit) {
		$("#box").droppable({
			hoverClass: "ui-state-hover",
			activeClass: "ui-state-active",  
			drop: function(event, ui) {
				ui.draggable.addClass("inBox");
			}
		});
		$("#box").click(function(){
			$("#objects .inBox").each(function(i, o){
				$(this).removeClass("inBox");
				var position = getPositionFor(app.parameters, $(this).attr("id"));
				log(position);
				$(this).animate({
					left: position.left,
					top: position.top
				});
			});
		});
	}
	*/
}

/**
* Ce code est pour la version objet personnalisable.
* L'app a été simplifié
*/
/*
function getIdFor(i) {
	return "o"+i;
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

function createObject(app, duplicateID) {
	var id = app.utils.guid();
	addObjectWithId(app, id);
	
	var last = $("#objects>div:last");
	var left = 0;
	var top = 0;

	if(last.size() > 0) {
		left = parseInt((last.css("left") || "0").replace("px","")) + 10;
		top = parseInt((last.css("top") || "0").replace("px",""));

		var count = last.parent().find(">div").size();
		log(last.parent().width() +"-"+ (left+last.width() * count));
		if(last.parent().width() <= (left+last.width() * count)){
			left = -last.width() * count;
			top += last.height() + 10;
		}
	}
	
	objectForGUID(app, id, {left: left, top: top});
	
	//TODO Duplication : Recupérer les images du dupliqué
}

function objectForGUID(app, guid, position) {
	window.object = guid;
	var objectUi = $(Mustache.render(objectTemplate, window));
	objectUi.css("left", position.left);
	objectUi.css("top", position.top);
	objectUi.draggable({
		containment: "#scene",
		scroll: false,
		revert: app.onEdit ? false : "invalid"
	});
	$("#objects").append(objectUi);
	
	if(app.onEdit) {
		objectUi.bind("dragstop", function(event, ui){
			var id = ui.helper.attr("id");
			log(id);
			var left = ui.position.left ;
			var top = ui.position.top;
			log(left+"-"+top);
			setPositionFor(app.parameters, id, ui.position);
		});
		objectUi.find("button[role=remove]").click(function(){
			removeObjectWithId(app, guid);	
			objectUi.remove();
		});
		objectUi.find("button[role=duplicate]").click(function(){
			createObject(app, guid);	
		});
	}
}

function getPositionFor(parameters, id) {
	var left = parameters.value("Object#"+id+"left");
	var top = parameters.value("Object#"+id+"top");
	if(left === undefined)
		left = 0;
	if(top === undefined)
		top = 0;
	return {left: parseInt(left), top: parseInt(top)};
}

function setPositionFor(parameters, id, position) {
	parameters.value("Object#"+id+"left", position.left);
	parameters.value("Object#"+id+"top", position.top);
}
*/

function reloadCallback(parameter) {
	reloadApp(this);
}

$(document).ready(function(){
	var callbacks = {
		onTemplatesLoaded: onTemplateLoadedCallback,
		onEdit: reloadApp,
		onView: reloadApp
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);
});