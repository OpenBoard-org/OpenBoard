(function($) {

    var s = 0;
    var w = 0;
    var h = 0;
    var t = 0;
    var ts = 0;
        
    $.fn.setSize = function(width, height){
        w = width;
        h = height;        
        $(this).find("ul").css('width',s*w);	
        $("ul", $(this)).animate(
        {
            marginLeft: (t*w*-1)
        }, 
        0
        );	
    }
    
    $.fn.removeSlide = function(){
        s = $("li", $(this)).length;     
        $(this).find("ul").css('width',s*w);
        ts = s-1;		
    }
    
    $.fn.addSlide = function(place){
        s = $("li", $(this)).length;          
        ts = s-1;	
        $(this).find("ul").css('width',s*w);
        if(place == "after")
            $("a","#nextBtn").trigger("click");   
    }

    $.fn.goToSlide = function(dest){
        t = dest;
        p = (t*w*-1);
        $("ul",$(this)).animate(
        {
            marginLeft: p
        }, 
        0
        );
    }
    
    $.fn.getPage = function(){
        return t;
    }

    $.fn.easySlider = function(options){
	  
        // default configuration properties
        var defaults = {			
            prevId: 		'prevBtn',
            prevText: 		'Previous',
            nextId: 		'nextBtn',	
            nextText: 		'Next',
            controlsShow:           true,
            controlsBefore:         '',
            controlsAfter:          '',	
            controlsFade:           true,
            firstId: 		'firstBtn',
            firstText: 		'First',
            firstShow:		false,
            lastId: 		'lastBtn',	
            lastText: 		'Last',
            lastShow:		false,				
            vertical:		false,
            speed: 			800,
            auto:			false,
            pause:			2000,
            continuous:		false
        }; 
		
        options = $.extend(defaults, options);  
				
        this.each(function() {  
            var obj = $(this); 				
            s = $("li", obj).length;
            w = $("li", obj).width(); 
            h = $("li", obj).height(); 
            obj.width(w); 
            obj.height(h); 
            obj.css("overflow","hidden");
            ts = s-1;
            t = 0;
            $("ul", obj).css('width',s*w);			
            if(!options.vertical) $("li", obj).css('float','left');
			
            if(options.controlsShow){
                var html = options.controlsBefore;
                if(options.firstShow) html += '<span id="'+ options.firstId +'"><a href=\"javascript:void(0);\">'+ options.firstText +'</a></span>';
                html += ' <span id="'+ options.prevId +'"><a href=\"javascript:void(0);\">'+ options.prevText +'</a></span>';
                html += ' <span id="'+ options.nextId +'"><a href=\"javascript:void(0);\">'+ options.nextText +'</a></span>';
                if(options.lastShow) html += ' <span id="'+ options.lastId +'"><a href=\"javascript:void(0);\">'+ options.lastText +'</a></span>';
                html += options.controlsAfter;						
                $(obj).after(html);										
            };
	
            $("a","#"+options.nextId).click(function(){	
                animate("next",true);
            });
            $("a","#"+options.prevId).click(function(){		
                animate("prev",true);				
            });	
            $("a","#"+options.firstId).click(function(){		
                animate("first",true);
            });				
            $("a","#"+options.lastId).click(function(){		
                animate("last",true);				
            });		
			
            function animate(dir,clicked){
                var ot = t;
                switch(dir){
                    case "next":
                        t = (ot>=ts) ? (options.continuous ? 0 : ts) : ++t;
                        break; 
                    case "prev":
                        t = (t<=0) ? (options.continuous ? ts : 0) : --t;
                        break; 
                    case "first":
                        t = 0;
                        break; 
                    case "last":
                        t = ts;
                        break; 
                    default:
                        break; 
                };	

                var diff = Math.abs(ot-t);
                var speed = diff*options.speed;						
                if(!options.vertical) {
                    p = (t*w*-1);
                    $("ul",obj).animate(
                    {
                        marginLeft: p
                    }, 
                    speed
                    );
                } else {
                    p = (t*h*-1);
                    $("ul",obj).animate(
                    {
                        marginTop: p
                    }, 
                    speed
                    );					
                };
				
                if(!options.continuous && options.controlsFade){					
                    if(t==ts){
                        $("a","#"+options.nextId).hide();
                        $("a","#"+options.lastId).hide();
                    } else {
                        $("a","#"+options.nextId).show();
                        $("a","#"+options.lastId).show();					
                    };
                    if(t==0){
                        $("a","#"+options.prevId).hide();
                        $("a","#"+options.firstId).hide();
                    } else {
                        $("a","#"+options.prevId).show();
                        $("a","#"+options.firstId).show();
                    };					
                };				
				
                if(clicked) clearTimeout(timeout);
                if(options.auto && dir=="next" && !clicked){
                    timeout = setTimeout(function(){
                        animate("next",false);
                    },diff*options.speed+options.pause);
                };
				
            };
            // init
            var timeout;
            if(options.auto){
                timeout = setTimeout(function(){
                    animate("next",false);
                },options.pause);
            };		
		
            if(!options.continuous && options.controlsFade){					
                $("a","#"+options.prevId).hide();
                $("a","#"+options.firstId).hide();				
            };				
			
        });
	  
    };

})(jQuery);



