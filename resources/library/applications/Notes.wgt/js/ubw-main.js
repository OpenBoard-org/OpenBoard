/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

function init(){
	
    var ubwidget = $("#ubwidget").ubwidget({
//        width:360,
//        height:240
    });
	
    var checkMinimize = false;
    var text = "";
    var winwidth;
    var minimizedHeight;
    var winheight;
    var scroll = false;
    var defaultFontSize = 35;
    var currentFontSize = defaultFontSize;
    var header = $('<div id="head"><input id="headtitle" class="stickytitle" readonly="readonly"/></div>');
    var fontUp = $('<img src="images/stick-but-aplus.png"/>');
    var fontDown = $('<img src="images/stick-but-aminus.png"/>');
    var minimize =  $('<img src="images/stick-but-minimize.png"/>');
    var maximize = $('<img src="images/stick-but-maximize.png"/>');
    var title = $('<div id="menu"></div>');
    var textField = $('<div id="txt"></div>');
    var shadow = $("<table width='100%' height='18' border='0' cellspacing='0' cellpadding='0'><tr><td id='sl' width='139'></td><td id='sc' style='background-image:url(images/shadowcenter.png)'>&nbsp;</td><td id='sr' width='139'></td></tr></table>")
    .appendTo("body");
    var shadowleft = $("<img src='images/shadowleft.png'/>")
    .appendTo($("#sl"));
    var shadowright = $("<img src='images/shadowright.png'/>")
    .appendTo($("#sr"));
		
    $("#ubwidget")
    .append(header)
    .append(textField);
    
    fontUp.addClass('menuElement');
    fontDown.addClass('menuElement');
    minimize.addClass('menuElement');
    maximize.addClass('menuElement')
    .hide();
		
    header.append(maximize)
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
				
            if(window.sankore){
                window.sankore.setPreference("fontSize", newFontSize);
            }
				
            if(!checkMinimize)
                textField.focus();
            
            var sel = window.getSelection();
            sel.removeAllRanges();
        });
			
    fontUp.click(
        function(){
            var newFontSize = parseInt(currentFontSize) + 3;
				
            textField.css({
                fontSize : newFontSize
            })
				
            controlTextField();
				
            if(window.sankore){
                window.sankore.setPreference("fontSize", newFontSize);
            }
				
            if(!checkMinimize)
                textField.focus();
            var sel = window.getSelection();
            sel.removeAllRanges();
        });
			
    minimize.click(
        function(){
            $('.ubw-container').css("min-height", "26px")
            $('.ubw-container').animate({
                height:"26px"
            },500);
            minimizedHeight = $('.ubw-container').height(); 
            minimize.hide();
            maximize.show();
            controlTextField();
            $('#headtitle').show();
            $('#headtitle').focus();
            //window.resizeTo($('.ubw-container').width(),0);				
            checkMinimize = true;
				
        });
			
    maximize.click(
        function(){            
            var lastHeight = String(minimizedHeight)+'px';
            $('.ubw-container').animate({
                height: lastHeight
            },500, function(){$('.ubw-container').css("min-height", "200px").css("height","")});

            maximize.hide();
            minimize.show();
            $('#headtitle').hide();
            //textField.focus();
            //window.resizeTo($('.ubw-container').width()+15,minimizedHeight+20);
            checkMinimize = false;
				
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
        if(window.sankore){
            window.sankore.setPreference("noteText", textField.html());
        }
        controlTextField();
    //$('#headtitle').hide();
    });
    if(textField.html().length === 0){
        textField.focus();
    }
    titletext.attr('rows','1');
			
    function controlTextField(){	
        if(textField.text().length < 25)
            $('#headtitle').val(textField.text());
        else 
            $('#headtitle').val(textField.text().substr(0,25) + "...");
		
        textField.css({
            height : $('.ubw-container').height()-28
        })
		
        if(textField.text().length == 0){
            textField.css({	
                fontSize: defaultFontSize
            });
        }
		
        currentFontSize = textField.css('fontSize').replace('px','');
	
    }	
	
    textField.bind('paste', function(e) {	
        controlTextField();			
    });
	
    $('#headtitle').hide();
	
    if(window.sankore){
        text = window.sankore.preference('noteText', text);
        currentFontSize = window.sankore.preference('fontSize', defaultFontSize);
        $('.ubw-container').css({
//            width:window.innerWidth - 2,
//            height:window.innerHeight - 20
        });

        if(checkMinimize){
            minimize.trigger("click");
        };

        textField.css({
            fontSize : parseInt(currentFontSize)
        })
        if(text) 
            textField.html(text);            
        else 
            textField.html("Saisir votre texte ici ...");       
        textField.focus();
    }		
				
    window.onresize = function(){
        winwidth = window.innerWidth;
        winheight = window.innerHeight;
		  	
//        if(winwidth <= 290)
//        {
//            window.resizeTo(290,winheight);
//        }
//        if(winheight <= 100)
//        {
//            window.resizeTo(winwidth,100);
//        }
//        if(winheight > 600)
//        {
//            window.resizeTo(winwidth,600);
//        }      		

//        $('.ubw-container').width(winwidth-2);
//			
        if(checkMinimize)					
            minimizedHeight = winheight-40; 


        controlTextField();  
    }
    
    if (window.widget) {
        window.widget.onleave = function(){
            window.sankore.setPreference("noteText", textField.html());
            window.sankore.setPreference("fontSize", newFontSize);
        }
    }
}
