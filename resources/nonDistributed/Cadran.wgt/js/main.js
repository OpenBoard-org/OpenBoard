function onTemplateLoadedCallback() {

}

function updateAnswer() {
	var flipped = $("#result").hasClass("flip");
	$("#result").removeClass("flip");

	setTimeout(function(){
		var nselected = $(".numbers .selected");
		var oselected = $(".operators .selected");
		var angle = 0;

		if(nselected.size() > 0 && oselected.size() > 0) {
			angle = nselected.data("angle");
			$(".result").removeClass("hide");
			var v = $(".input > div").text();
			var o = oselected.text();
			var n = nselected.text();
			var e = Math.floor(eval(v+o+n));
			if(isNaN(e) || e == Infinity)
				e = "...";
			else if(o == "/") {
				var r = v%n;
				e = $("<div>"+e+"</div>");
				e.append($("<span class='reste'>"+r+"</span>"));
			}
			$("#result .back").html(e);
		}else
			$(".result").addClass("hide");

		$(".result").css("transform","rotate("+angle+"deg)");
		$("#result > div").css("transform","rotate("+((-1)*angle)+"deg)");
	}, flipped ? 500 : 0);
}

function reloadApp(app) {
	var input = parseInt(app.parameters.value("input"));
	if(isNaN(input))
		input = Math.floor(Math.random()*12);
	var $input = $(".input > div");
	$input.text(input);

	//Clean
	$(".numbers .selected").removeClass("selected");
	$(".operators .selected").removeClass("selected");
	$input.unbind("focus").unbind("blur");
	$input.get(0).contentEditable = false;
	
	updateAnswer();

	if(app.onEdit) {
		$input.get(0).contentEditable = true;
		$input.focus(function(){
			if($(this).text() == "...")
				$(this).text("");
		});
		
		$input.bind('blur', function(){
			var v = parseInt($(this).text().trim());
			if(isNaN(v)) {
				$(this).text("...");
			}else{
			}
			app.parameters.value("input", v);
		});
	}
}

function reloadCallback(parameter) {
	if(parameter === undefined)
		reloadApp(this);
}

$(document).ready(function(){
	var callbacks = {
		onTemplatesLoaded: onTemplateLoadedCallback,
		onEdit: reloadApp,
		onView: reloadApp
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);

	var nr = $(".numbers").width()/2; 
	
	$(".numbers > div").each(function(i){
		var n = $(this);

		var angle = 30*i;

		var rotate = "rotate("+(angle)+"deg)";
		var irotate = "rotate("+(-angle)+"deg)";

		n.css("transform", rotate);
		$(">div",n).css("transform", irotate)
			.data("angle", angle)
			.click(function() {
				if($(this).hasClass("selected")) {
					$(this).removeClass("selected");	
				}else {
					$(".numbers .selected").removeClass("selected");
					$(this).addClass("selected");	
				}
				updateAnswer();
			});
	});

	$(".operators > div").each(function(i){
		var n = $(this);

		var angle = 90*i;

		var rotate = "rotate("+(angle)+"deg)";
		var irotate = "rotate("+(-angle)+"deg)";

		n.css("transform", rotate);
		$(">div",n).css("transform", irotate)
			.click(function() {
				if($(this).hasClass("selected")) {
					$(this).removeClass("selected");	
				}else {
					$(".operators .selected").removeClass("selected");
					$(this).addClass("selected");	
				}
				updateAnswer();
			});
	});

	$("#result").click(function(){
		$(this).toggleClass("flip");
	});
});