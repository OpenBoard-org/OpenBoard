function init(){
	
	var ubwidget = $("#ubwidget").ubwidget({
			width:300,
			height:240
		});
	
	var text = "";
	var winwidth;
	var minimizedHeight;
	var winheight;
	var scroll = false;
	var defaultFontSize = 35;
	var currentFontSize = defaultFontSize;
	var header = $('<div id="head"><input id="headtitle" class="stickytitle"></input></div>');
	var fontUp = $('<img src="images/stick-but-aplus.png">');
	var fontDown = $('<img src="images/stick-but-aminus.png">');
	var minimize =  $('<img src="images/stick-but-minimize.png">');
	var maximize = $('<img src="images/stick-but-maximize.png">');
	var title = $('<div id="menu"></div>');
	var textField = $('<div id="txt"></div>');
		
		var shadow = $("<table width='100%' height='18' border='0' cellspacing='0' cellpadding='0'><tr><td id='sl' width='139'></td><td id='sc' style='background-image:url(images/shadowcenter.png)'>&nbsp;</td><td id='sr' width='139'></td></tr></table>")
			.appendTo("body");

		var shadowleft = $("<img src='images/shadowleft.png'>")
			.appendTo($("#sl"));
		var shadowright = $("<img src='images/shadowright.png'>")
			.appendTo($("#sr"));
		
		$("#ubwidget")
			.append(header)
			.append(textField);
		
		fontUp
			.addClass('menuElement');
		fontDown
			.addClass('menuElement');
		minimize
			.addClass('menuElement');
		maximize
			.addClass('menuElement')
			.hide();
		
		header
			.append(maximize)
			.append(minimize)
			.append(fontUp)
			.append(fontDown);
		var titletext = header.find('textarea');
		
		titletext.click(
			function(){
			
				titletext.focus();			
			
			});
			
		fontDown.click(
			function(){
				
				var newFontSize = parseInt(currentFontSize) - 3;
				
				textField.css({
					fontSize : newFontSize
				})
				
				controlTextField();
				
				if(window.uniboard){
					window.uniboard.setPreference("fontSize", newFontSize);
				};
				
			});
			
		fontUp.click(
			function(){
				
				var newFontSize = parseInt(currentFontSize) + 3;
				
				textField.css({
					fontSize : newFontSize
				})
				
				controlTextField();
				
				if(window.uniboard){
					window.uniboard.setPreference("fontSize", newFontSize);
				};
			});
			
		minimize.click(
			function(){
				$('.ubw-container').animate({height:"26px"},100);
				minimizedHeight = $('.ubw-container').height() ; 
				minimize.hide();
				maximize.show();
				$('#headtitle').show();
				window.resizeTo($('.ubw-container').width(),0);
				
				if(window.uniboard){
					window.uniboard.setPreference("minimized", "true");
				};
			});
			
		maximize.click(
			function(){
				var lastHeight = String(minimizedHeight)+'px';
				$('.ubw-container').animate({height: lastHeight},100);

				maximize.hide();
				minimize.show();
				$('#headtitle').hide();

				window.resizeTo($('.ubw-container').width()+15,minimizedHeight+20);
				
				if(window.uniboard){
					window.uniboard.setPreference("minimized", "false");
				};
			});			
				
		header
			.append(title);
		title
			.addClass('menu');
		header
			.addClass('head');
		textField
			.addClass("textField")
			.css("fontSize",currentFontSize)
			.attr('contentEditable','true')
			.keyup(function(){
				if(window.uniboard){
					window.uniboard.setPreference("text", textField.html());
				}
				controlTextField();
				$('#headtitle').hide();
			});
		if(textField.html().length === 0){
			textField.focus();
		}
		titletext
			.attr('rows','1');
			
	function controlTextField(){	
		if(textField.text().length < 25){
			var titleStr =  textField.text() + '...';
			$('#headtitle').val(titleStr);
		}	
		
		textField.css({
			height : $('.ubw-container').height()-28,
		})
		
		if(textField.text().length == 0){
			textField.css({	
				fontSize: defaultFontSize,
			});
		}
		
		currentFontSize = textField.css('fontSize').replace('px','');
	
	}	
	
	textField.bind('paste', function(e) {	
		controlTextField();			
    });	
	$('#headtitle').hide();
	if(window.uniboard){
		text = window.uniboard.preference('text', text);
		currentFontSize = window.uniboard.preference('fontSize', defaultFontSize);
		
		$('.ubw-container').css({
			width:parseInt(window.uniboard.preference('width', "300")),
			height:parseInt(window.uniboard.preference('height', "240"))
		});

		if(window.uniboard.preference('minimized', "false") == "true"){
			minimize.trigger("click");
		};

		textField.css({
			fontSize : parseInt(currentFontSize)
		})
		textField.html(text);
	}		
				
	  window.onresize = function(){
		  winwidth = window.innerWidth;
		  winheight = window.innerHeight;
		  	
		  	if(winwidth <= 290)
		  	{
		  		window.resizeTo(290,winheight);
		  	}
		  	if(winheight <= 100)
		  	{
		  		window.resizeTo(winwidth,100);
		  	}
		  	if(winheight > 600)
		  	{
		  		window.resizeTo(winwidth,600);
		  	}
      			
      		$('.ubw-container').width(winwidth-2) ;
      		$('.ubw-container').height(winheight-20) ; 

			if(window.uniboard){
				window.uniboard.setPreference("width", winwidth-2);
				window.uniboard.setPreference("height", winheight-20);
			}

      		controlTextField();  
      }
}