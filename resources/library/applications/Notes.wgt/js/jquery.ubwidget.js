(function($) {
	jQuery.fn.ubwidget = function(options) {
		var settings = jQuery.extend({}, jQuery.fn.ubwidget.defaults, options);
		
		$(window)
			.bind("blur", function(event){ 
  				$("#ubw-catcher").trigger("mousedown");
  				//$("#ibutton").hide();
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
				});
				//.mouseenter(function(){$("#ibutton").show(0)});
			
			$('body').append(ubwcontainer);
		});
	};
	
	// Default options
	
	jQuery.fn.ubwidget.defaults = {
	};
	
	// Shadows
	
	jQuery.fn.ubwshadows = function(settings){
			
			var shadow = $("<div class='ubw-shadow'></div>")
				.addClass("ubw-standard-corners")
				.css({
					backgroundColor:"#333377",
					opacity:".08",
  					filter: "alpha(opacity = 18)",				
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
			var scale = 1.15;
						
			return this.each(function() {
				button = $(this)
					.addClass("ubw-button-wrapper")
					.disableTextSelect();	
				
				button.data("size", size);														
					
				var buttonContent = $("<div></div>")
					.addClass("ubw-button-content")
					.html($(this).html());
						
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
					.addClass("ubw-button-corners")
					.append(buttonContent)
					.bind("mouseenter", buttonOverHandler)
					.bind("mouseleave", buttonOutHandler)
					.bind("mousedown", buttonDownHandler)
					.bind("mouseup", buttonUpHandler)
					.css({
						width:size.w,
						height:size.h
					});	
					
				setTimeout(function(){button.find(".ubw-button-content").center()}, 10);				
			});	
	
			function buttonOverHandler(e) {			
				var buttonbody = button.find(".ubw-button-body");
				var sizeover = {w:size.w*1.15, h:size.h*1.15};
				
				button.find(".ubw-button-body").ubwshadows({
					w:sizeover.w+12, 
					h:sizeover.h+12, 
					l:-2, 
					t:1
				});
												
				button.css({zIndex:1})
					.children(".ubw-button-canvas")
						.css({
							marginLeft:(buttonbody.width()-sizeover.w)/2,
							marginTop:(buttonbody.height()-sizeover.h)/2
						});
				buttonbody.removeClass("ubw-button-out")
					.addClass("ubw-button-over")
					.css({
						fontSize:"115%",
						width:sizeover.w,
						height:sizeover.h
					});	
										
				button.find(".ubw-button-canvas").find(".ubw-button-arrowTop").children("img").attr("src", "images/arrows_over/top.png");
				button.find(".ubw-button-canvas").find(".ubw-button-arrowBottom").children("img").attr("src", "images/arrows_over/bottom.png");
			};
	
			function buttonOutHandler(e){
				var buttonbody = button.find(".ubw-button-body");
				
				button.find(".ubw-shadow").remove();
				button.css({zIndex:0});
				buttonbody.removeClass("ubw-button-over")
					.addClass("ubw-button-out")					
					.css({
						fontSize:"100%",
						width:size.w,
						height:size.h
					});
				button.children(".ubw-button-canvas")
					.css({
						marginLeft:0,
						marginTop:0
					});
					
				button.find(".ubw-button-canvas").find(".ubw-button-arrowTop").children("img").attr("src", "images/arrows_out/top.png");
				button.find(".ubw-button-canvas").find(".ubw-button-arrowBottom").children("img").attr("src", "images/arrows_out/bottom.png");			};	
			
			function buttonDownHandler(e){
				var buttonbody = button.find(".ubw-button-body");
				
				buttonbody.css({fontSize:"125%"});
			};
			
			function buttonUpHandler(e){
				var buttonbody = button.find(".ubw-button-body");
				
				buttonbody.css({fontSize:"115%"});
			};
	};
		
	jQuery.fn.ubwtoggle = function(activated) {
			var activated = typeof(activated) != "undefined" ? activated = 1 : activated = 0;
	
			return this.each(function(){
				var button = $(this);
				var buttonBody = button.find(".ubw-button-body");
				var img = buttonBody.find("img");
				var imgsrc = img.attr("src");
								
				buttonBody
					.toggle(
						function(){
							img.css({visibility:"hidden"});
						}, 
						function(){
							img.css({visibility:"visible"});
						}
					);	
				
				if(!activated){
					buttonBody.trigger("click");
				};			
			});
	};
	
	jQuery.fn.ubwdropdown = function(size, list, func){
		return this.each(function(){
		
			var button = $(this);
						
			var dropdownList = $("<ul></ul>")
				.addClass("ubw-dropdown")
				.hide();
       	
       		for(var i=0; i<list.length; i++){       		
       			var newLine = $("<li>"+list[i]+"</li>")
       				.addClass("out")
       				.bind("mouseenter mouseleave", function(){
       					$(this).toggleClass("over");
       				})
       				.bind("click", {i:i}, function(e){       					
       					return function(){
       						func(list[e.data.i]);
       						button.find(".ubw-button-content")
       							.empty()
       							.append(list[e.data.i]);
       						button.find(".ubw-button-body")
       							.trigger("click");
       							//.bind("mouseenter", jQuery.fn.ubwbutton.buttonOverHandler);
       					}();
       				});
        		dropdownList.append(newLine);
        		
        		if(i==0){
        			newLine.addClass("ubw-dropdown-top-corners");
        		}else if(i==list.length-1){
        			newLine.addClass("ubw-dropdown-bottom-corners");
        		}
        	}
        	
        	$(this).ubwbutton(size, {top:0, bottom:1, left:0, right:0})
        		.find(".ubw-button-body")
        			.mouseenter(function(){
        				$(this).css({
							backgroundImage:"url(images/button_out_dark.png)",
							color:"#eeeeee",
							border:"none"
        				})
        			})
        			.mouseleave(function(){
        				$(this).css({
							backgroundImage:"url(images/button_out.gif)",
							color:"#555555",
							borderLeft:"2px solid rgb(231, 231, 233)",
							borderRight:"2px solid rgb(231, 231, 233)",
							borderBottom:"2px solid rgb(221, 221, 223)",
							borderTop:"2px solid rgb(241, 241, 244)"
        				});
        			})
        			.toggle(
        				function(){
        					dropdownList.show();
        					$(this).trigger("mouseleave");
        					//$(this).unbind("mouseenter");
        				},
        				function(){
        					dropdownList.hide();
        				}
        			);
        	button.find(".ubw-button-canvas").append(dropdownList);
        });
	}
	
	jQuery.fn.scrollHandler = function(){
		return this.each(function(){
			$(this).mouseenter(function(){
						$(this).css({
							backgroundImage:"url(images/button_out_dark.png)",
							border:"none",
							color:"#eeeeee"
						})
					})
					.mouseleave(function(){
						$(this).css({
							backgroundImage:"url(images/button_out.gif)",
							color:"#555555",
							borderLeft:"2px solid rgb(231, 231, 233)",
							borderRight:"2px solid rgb(231, 231, 233)",
							borderBottom:"2px solid rgb(221, 221, 223)",
							borderTop:"2px solid rgb(241, 241, 244)"
						})
					});
		});	
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
				resizeubcanvas()
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
			.addClass("ubw-button-corners");
		
			var inspectorWidth = inspector.width();
			var inspectorHeight = inspector.height();	
			var windowWidth = $(window).width();
			var windowHeight = $(window).height();					 	 
			
			$("body").prepend(button);
			//button.addClass("ubw-standard-corners")
			button.click(function(){
					catcher.show();
					inspector.show();
					dropShadow();
					resizeubcanvas()
				})
				.attr("id", "ibutton");
					
			function dropShadow (){
				inspector.ubwshadows({w:inspectorWidth+23,h:inspectorHeight+22,l:50,t:80})}
			function removeDropShadow (){
				$(".ubw-shadow").remove()}
			
			// !!
			$(".ubw-shadow")
				.mousedown(function(){
					inspector.hide();
					removeDropShadow();
					catcher.hide();
					resizeubcanvas();
				});
			
			function resizeubcanvas(){
				
				if(inspector.css("display")=="none"){
					window.resizeTo($(".ubw-container").width()+68, $(".ubw-container").height()+68);
				}
				else{
					var inspectorbottom = inspector.position().top+inspector.height()+60;
					var inspectorright = inspector.position().left+inspector.width()+45;
								
					if($(window).width()<inspectorright || $(window).height()<inspectorbottom){
						if($(window).width()<inspectorright){
							window.resizeTo(inspectorright, $(".ubw-container").height()+68)}
						if($(window).height()<inspectorbottom){
							window.resizeTo($(".ubw-container").width()+68, inspectorbottom)}
						if($(window).width()<inspectorright && $(window).height()<inspectorbottom){
							window.resizeTo(inspectorright, inspectorbottom);
						}
					}
					else{
						window.resizeTo($(".ubw-container").width()+68, $(".ubw-container").height()+68);
					}
				};
			}
	};
		
})(jQuery);