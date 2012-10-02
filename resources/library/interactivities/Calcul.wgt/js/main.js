function onTemplateLoadedCallback() {

}

function change() {
	$(".pagination .current").removeClass("current");
	$(".operations .current").removeClass("current");
}

function open(index) {
	$(".pagination li").eq(index).addClass("current");
	$(".operations li").eq(index).addClass("current");
}

function reloadApp(app) {
	var operator = app.parameters.value("operator");
	var range = parseInt(app.parameters.value("range"));
	var count = parseInt(app.parameters.value("count"));
	var operations = parseInt(app.parameters.value("operations"));
	
	$scene = $("#scene");
	$scene.empty();

	$operationContainer = $("<div id='operations'></div>");
	$scene.append($operationContainer);

	$pagination = $("<ul class='pagination'></ul>");
	$operationContainer.append($pagination);
	for (var i = 0; i < operations; i++)
		$("<li><span>"+(i+1)+"</span></li>").appendTo($pagination);

	$operations = $("<ul class='operations'></ul>");
	$operationContainer.append($operations);

	var opes = [];
	var submitted = [];

	for (var i = 0; i < operations; i++) {
		var ope = [];
		for (var j = 0; j < count; j++) {
			ope.push(Math.floor(Math.random()*range));
		}
		opes.push(ope);

		var $text = $("<div>"+ope.join("<span class='operator'>"+operator+"</span>")+"<span class='equal'>=</span><span class='input'>...</span><button type='button' disabled>Submit</button></div>");
		var $li = $("<li/>");
		$li.append($text);
		$operations.append($li);
	}

	if(!app.onEdit) {
		$(".input", $operations).each(function(){
			var $input = $(this);
			$input.get(0).contentEditable = true;
			$input.focus(function(){
				if($(this).text() == "...")
					$(this).text("");
			});
			$input.keyup(function(){
				var v = parseInt($(this).text().trim());
				if(isNaN(v)) {
					$(this).next().attr("disabled", true);
				}else{
					$(this).next().attr("disabled", false);
				}
			});
			$input.bind('blur', function(){
				var v = parseInt($(this).text().trim());
				if(isNaN(v)) {
					$(this).text("...");
					$(this).next().attr("disabled", true);
				}else{
					$(this).next().attr("disabled", false);
				}
			});
		});
		$("button", $operations).click(function(){
			var $button = $(this);
			var v = $button.prev().text();
			if(!isNaN(v)) {
				var $li = $button.closest('li');
				$button.remove();
				var index = $li.index();
				submitted[index] = v;
				var waiting = eval(opes[index].join(operator));
				var clazz = (waiting == v ? "succeed" : "failed");
				$pagination.find("li").eq(index).addClass(clazz);
				$li.addClass(clazz);
				setTimeout(function(){
					change();
					$li.addClass("closed");
					if(index == (operations-1)) {
						$operationContainer.addClass("finish");
					}else
						open(index+1);
				}, 2000);
			}
		});
	}
	open(0);
}

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