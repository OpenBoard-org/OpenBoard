(function($) {
	jQuery.fn.ubwidget = function(options) {
		var settings = jQuery.extend({}, jQuery.fn.ubwidget.defaults, options);
		
		$(window)
			.bind("blur", function(event){ 
  			})
  			.trigger("focus");
		
		return this.each(function() {	
			var ubwbody = $(this)	
				.addClass("ubw-body");
		
			var ubwcontainer = $("<div></div>")
				.append(ubwbody)					
				.addClass("ubw-container")
				.css({
					width:settings.width,
					height:settings.height
				})
				.disableTextSelect();
			
			$('body').append(ubwcontainer);
		});
	};
	
	// Default options
	
	jQuery.fn.ubwidget.defaults = {
		width:200,
		height:250
	};
	
	// Shadows
	
		jQuery.fn.ubwshadows = function(settings){
			
			var shadow = $("<div class='ubw-shadow'></div>")
				.addClass("ubw-standard-corners")
				.css({
					backgroundColor:"#333377",
					opacity:".1",
  					filter: "alpha(opacity = 10)",				
					position:"absolute",
					top:settings.t,
					left:settings.l,
					width:settings.w,
					height:settings.h
				});
			
			$(this).before(shadow);
	};
		
	jQuery.fn.ubwbutton = function(size, arrows) {
			var arrows = typeof(arrows) != "undefined" ? arrows = arrows : arrows = {top:0, right:0, bottom:0, left:0};
			var button = null;
			var scale = 0.20;
			var url = "";
			var buttonbody;

						
			return this.each(function() {
				button = $(this)
					.addClass("ubw-button-wrapper")
					.css({zIndex:0});	
					
				url = $(this).find("img").attr("src");
				url = url.split(".");
					
				var buttonContent = $("<table cellpadding='0' cellspacing='0' border='0' width='auto' height='100%'><tr><td height='auto' width='auto' valign='middle' align='center'></td></tr></table>")
					.addClass("ubw-button-content");
				buttonContent.find("td").html($(this).html());
				$(this).empty();
				
				var buttonCanvas = $("<div></div>")
					.addClass("ubw-button-canvas")
					.appendTo(button)
				 	.html('<table width="auto" height="auto" cellpadding="0" cellspacing="0"><tr><td class="ubw-button-arrowTop" align="center"><img style="visibility:hidden; margin-bottom:-1px" src="images/arrows_out/top.png"></td></tr><tr><td><table width="auto" height="auto" border="0" cellpadding="0" cellspacing="0"><tr><td class="ubw-button-arrowLeft"><img style="visibility:hidden; margin-right:-1px" src="images/arrows_out/left.png"></td><td class="ubw-button-body"></td><td class="ubw-button-arrowRight"><img style="visibility:hidden; margin-left:-1px" src="images/arrows_out/right.png"></td></tr></table></td></tr><tr><td class="ubw-button-arrowBottom" align="center"><img style="visibility:hidden; margin-top:-1px" src="images/arrows_out/bottom.png"></td></tr></table>');
															
				if(arrows.top)buttonCanvas.find(".ubw-button-arrowTop").children("img").css({visibility:"visible"});
				if(arrows.right)buttonCanvas.find(".ubw-button-arrowRight").children("img").css({visibility:"visible"});
				if(arrows.bottom)buttonCanvas.find(".ubw-button-arrowBottom").children("img").css({visibility:"visible"});
				if(arrows.left)buttonCanvas.find(".ubw-button-arrowLeft").children("img").css({visibility:"visible"});
								
				var buttonBody = buttonCanvas.find(".ubw-button-body")											
					.addClass("ubw-button-out")
					.append(buttonContent)
					.bind("mouseenter", buttonOverHandler)
					.bind("mouseleave", buttonOutHandler)
					.bind("mousedown", buttonDownHandler)
					.bind("mouseup", buttonUpHandler)
					.css({
						width:size.w,
						height:size.h
					});
				
				button.width(size.w+9).height(size.h+2);	
				buttonbody = buttonBody.find(".ubw-button-content");			
			});	
			
	
			function buttonOverHandler(e) {
				buttonbody.find("img").attr("src", url[0]+"_over.png");
				
				button.css({
					zIndex:1
				});
			};
	
			function buttonOutHandler(e){
				buttonbody.find("img").attr("src", url[0]+".png");
				
				button.css({
					zIndex:0
				});
			};	
			
			function buttonDownHandler(e){
				buttonbody.find("img").attr("src", url[0]+"_over_down.png");
			};
			
			function buttonUpHandler(e){
				url[0] = url[0].replace("_down", "");
				buttonbody.find("img").attr("src", url[0]+"_over.png");
			};
			
	};
		
	jQuery.fn.ubwtoggle = function(activated, _firstFunc, _secondFunc) {
			var activated = typeof(activated) != "undefined" ? activated = 1 : activated = 0;
	
			return this.each(function(){
				var button = $(this);
				var buttonBody = button.find(".ubw-button-body");
				var img = buttonBody.find("img");
				var imgsrc = img.attr("src");
				
				var firstFunc = _firstFunc;
				var secondFunc = _secondFunc;
												
				buttonBody
					.toggle(
						function(){
							img.css({visibility:"hidden"});
							firstFunc();
						}, 
						function(){
							img.css({visibility:"visible"});
							secondFunc();
						}
					);	
				
				if(activated){
					buttonBody.trigger("click");
				};			
			});
	};

	
	jQuery.fn.ubwidget.sliderbutton = function() {
		
	};	
		
	jQuery.fn.ubwidget.inspector = function(_position, content, button){
		
		var position = {x:_position.x, y:_position.y};
		
		var catcher = $("<div id='ubw-catcher'></div>")
			.css({
				position:"absolute",
				width:"100%",
				height:"100%"
			})
			.mousedown(function(){
				inspector.hide();
				removeDropShadow();
				catcher.hide();
				//resizeubcanvas()
			});
					
			$("body").append(catcher);
			catcher.hide();
		
		var inspector = $("<div class='ubw-inspector'></div>")
			.css({				
				left:position.x,
				top:position.y
			})
			.append(content)
			.appendTo($("body"))
			.hide()
			.disableTextSelect();
		
			var inspectorWidth = inspector.width();
			var inspectorHeight = inspector.height();	
			var windowWidth = $(window).width();
			var windowHeight = $(window).height();					 	 
			
			$("body").prepend(button);
			button.addClass("ubw-rounded")
				.click(function(){
					catcher.show();
					inspector.show();
					dropShadow();
					resizeubcanvas()
				});
					
			function dropShadow (){
				inspector.ubwshadows({w:inspectorWidth+23,h:inspectorHeight+22,l:55,t:55})}
			function removeDropShadow (){
				$(".ubw-shadow").remove()}
			
			// !!
			$(".ubw-shadow")
				.mousedown(function(){
					inspector.hide();
					removeDropShadow();
					catcher.hide();
					resizeubcanvas()
				});
			
			function resizeWidget(w, h){
				window.resizeTo(w+2, h+2);
				$("#indicator").remove();
				var indicator = $("<div id='indicator'></div>")
					.css({
						width:w,
						height:h,
						position:"absolute",
						left:0,
						top:0,
						border:"1px solid #ff0000"
					});
				//$("body").prepend(indicator);
			}
			
			function resizeubcanvas(){
				
				if(inspector.css("display")=="none"){
					resizeWidget(windowWidth, windowHeight);
					return 0;
				};
				
				var inspectorbottom = inspector.position().top+inspector.height()+40;
				var inspectorright = inspector.position().left+inspector.width()+40;
								
				if($(window).height()<inspectorbottom){
					resizeWidget($(window).width(), inspectorbottom)};
					
				if($(window).width()<inspectorright){
					resizeWidget(inspectorright, $(window).height())};
			}
	};
		
})(jQuery);