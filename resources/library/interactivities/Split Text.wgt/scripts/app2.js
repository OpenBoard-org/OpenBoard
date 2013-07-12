//just a flag
var flag = true;

function createElements( text )
{
    var s = '';
	
    var words = text.split( " " );
	
    for( var i = 0; i < words.length; i++ )
    {
        if( i ){
            s += '<div>&nbsp;</div>';
        }
        s += '<div class="letter">' + words[i] + '</div>' + 
        '<div class="dash">.</div>';
    }
	
    return s;
}

$(document).ready(function()
{
    var w = new wcontainer( "#ub-widget" );
    
    var sentences = "";
    
    if(window.sankore)
        sentences = (sankore.preference("ordSplText", ""))?sankore.preference("ordSplText", ""):sankoreLang.example;
    else
        sentences = sankoreLang.example;
    
    if (window.widget) {
        window.widget.onleave = function(){
            sankore.setPreference("spl_text_style", $("#style_select").find("option:selected").val());
            if(w.editMode){
                sankore.setPreference("ordSplText", w.elements.container.find( "textarea" ).val());
                sankore.setPreference("ordSplTextState", "0");
            }
            else{
                sankore.setPreference("ordSplTextCode", $("#mp_view").html());
                sankore.setPreference("ordSplTextState", "1");
                sankore.setPreference("ordSplTextAnswer", ($("#mp_view").hasClass("answerRight"))?"answerRight":"");
                sankore.setPreference("ordSplText", w.getData( "text" ));              
            }
        }
    }
    
    if(sankore.preference("spl_text_style","")){
        changeStyle(sankore.preference("spl_text_style",""));
        $("#style_select").val(sankore.preference("spl_text_style",""));
    } else
        changeStyle("3")
    
    $("#wgt_display").text(sankoreLang.view);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_help").text(sankoreLang.help);
    $("#help").html(sankoreLang.help_content);
    $("#style_select option[value='1']").text(sankoreLang.slate);
    $("#style_select option[value='2']").text(sankoreLang.pad);
    $("#style_select option[value='3']").text(sankoreLang.none);
    
    var tmpl = $("div.inline label").html();
    $("div.inline label").html(sankoreLang.theme + tmpl)
    
    $("#style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })  
    
    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){                
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $("#parameters").css("display","none");                
                $(this).css("display", "none");
                $("#wgt_edit").css("display", "block");
                w.modeView();
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $("#parameters").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
                w.modeEdit();
            }
        }
    });
    
    $("#wgt_name").text(sankoreLang.wgt_name);
    
    $("#wgt_help").click(function(){
        var tmp = $(this);
        if($(this).hasClass("open")){
            $(this).removeClass("help_pad").removeClass("help_wood")
            $("#help").slideUp("100", function(){
                tmp.removeClass("open");
                $("#ub-widget").show();
            });
        } else {            
            ($("#style_select").val() == 1)?$(this).removeClass("help_pad").addClass("help_wood"):$(this).removeClass("help_wood").addClass("help_pad");
            $("#ub-widget").hide();
            $("#help").slideDown("100", function(){
                tmp.addClass("open");
            });
        }
    });
    
    $("#wgt_reload").text(sankoreLang.reload).click(function(){
        if($("#wgt_display").hasClass("selected")){
            $("#wgt_edit").trigger("click");
            $("#wgt_display").trigger("click");
        } else {
            $("#wgt_display").trigger("click");
        }
    });
    
    w.maxWidth = 600;
	
    w.setEditContent( '<div class="inputwrap"><textarea class="percent">' + sentences + '</textarea></div>' );
    w.setViewContent( '<div class="upper"><div class="dash fixed">. </div></div>' );
    w.setData( "dashWidth", w.elements.container.find( ".dash" ).outerWidth() );
    w.setViewContent( '<div class="upper"><div class="dash fixed">M</div></div>' );
    w.setData( "lineHeight", w.elements.container.find( ".dash" ).outerHeight() );
    w.setViewContent( "" );
	
	
    // onViewMode
    w.onViewMode = function()
    {
        // clean up the text
        var text = w.elements.container.find( "textarea" ).val()
        .replace( /\r/g, '' ).replace( /\n/g, ' ' ).replace( /  /g, ' ' );
		
        // store the text
        w.setData( "text", text );
		
        // remove all dots (they are to be set during the exercise)
        text = text.replace( /\. /g, ' ' ).trim( ["."] );
		
        // create the html
        if(sankore.preference("ordSplTextState", "") == "1" && flag){
            $("#mp_view").html(sankore.preference("ordSplTextCode", ""));
            if(sankore.preference("ordSplTextAnswer", ""))
                $("#mp_view").addClass(sankore.preference("ordSplTextAnswer", ""));
            flag = false;
        } 
        else
            w.setViewContent( createElements( text ) );
        
		
        // the behaviour
        w.elements.containerView.find( ".letter" )
        .mouseover( function()
        {
            var el = $( this ).next();
			
            // determine new hover class
            var is_fixed = ( el.get( 0 ).className.indexOf( "fixed" ) != -1 );
            var hover_class = is_fixed?
            "dash_hover_fixed" : "dash_hover";
			
            // assign new hover class
            el.addClass( hover_class )
            .data( "hc", hover_class );
        })
        .mouseout( function()
        {
            var el = $( this ).next();
            // remove current hover class
            var hc = el.data( "hc" );
            el.removeClass( hc );
        })
        .click( function()
        {
            var el = $( this ).next();
			
            // remove current hover class
            $( this ).trigger( "mouseout" );
			
            // toggle fixed class
            el.toggleClass( "fixed" );
			
            // determine new hover class
            // assign new hover class
            $( this ).trigger( "mouseover" );
			
            w.checkAnswer();
        });
		
        w.checkAnswer();
    };
	
    // viewSize
    w.viewSize = function()
    {
        var w = 0;
        var h = 0;
		
        var dh = winstance.getData( "lineHeight" );
        var dw = winstance.getData( "dashWidth" );
		
        winstance.elements.containerView.find( "div:visible" ).each( function()
        {
            w += $( this ).outerWidth();
            h = Math.max( h, $( this ).outerHeight( true ) );
        });
		
        var square = w*h;
        h = Math.max( h, $( winstance.elements.containerView ).height() );
		
        if( winstance.maxWidth )
        {
            w = Math.min( w, winstance.maxWidth );
            h = parseInt( square / w );
        }
		
        return {
            w: w, 
            h: h+dh
        };
    };
	
    // editSize
    w.editSize = function()
    {
        return {
            w: winstance.elements.containerEdit.find( "textarea" ).parent().outerWidth( true ),
            h: winstance.elements.containerEdit.find( "textarea" ).parent().outerHeight( true )
        };
    };
	
	
    w.checkAnswer = function()
    {
        var text = "";
        var ch = "";
        this.elements.containerView.find( "div:visible" ).each( function()
        {
            if( this.className.indexOf( "fixed" ) != -1 ){
                text += '.';
            }
            else if( this.className.indexOf( "dash" ) != -1 ){
                return;
            }
            else{
                ch = $( this ).html();
                if( ch == "&nbsp;" ){
                    ch = " ";
                }
                text += ch;
            }
        });
		
        if( text == this.getData( "text" ) ){
            this.elements.containerView.addClass( "answerRight" );
        }
        else{
            this.elements.containerView.removeClass( "answerRight" );
        }
    };

	
    window.w = w;
    window.winstance = w;
	
    w.modeView();
    
    //changing the style
    function changeStyle(val){
        switch(val){
            case "1":
                $(".b_top_left").removeClass("btl_pad").removeClass("without_back");
                $(".b_top_center").removeClass("btc_pad").removeClass("without_back");
                $(".b_top_right").removeClass("btr_pad").removeClass("without_back");
                $(".b_center_left").removeClass("bcl_pad").removeClass("without_back");
                $(".b_center_right").removeClass("bcr_pad").removeClass("without_back");
                $(".b_bottom_right").removeClass("bbr_pad").removeClass("without_back");
                $(".b_bottom_left").removeClass("bbl_pad").removeClass("without_back");
                $(".b_bottom_center").removeClass("bbc_pad").removeClass("without_back");
                $("#wgt_reload").removeClass("pad_color").removeClass("pad_reload");
                $("#wgt_help").removeClass("pad_color").removeClass("pad_help");
                $("#wgt_edit").removeClass("pad_color").removeClass("pad_edit");
                $("#wgt_name").removeClass("pad_color");
                $("#wgt_display").addClass("display_wood");
                $("#style_select option:first").attr('selected',true);
                $("body, html").removeClass("without_radius").addClass("radius_ft");
                break;
            case "2":
                $(".b_top_left").addClass("btl_pad").removeClass("without_back");
                $(".b_top_center").addClass("btc_pad").removeClass("without_back");
                $(".b_top_right").addClass("btr_pad").removeClass("without_back");
                $(".b_center_left").addClass("bcl_pad").removeClass("without_back");
                $(".b_center_right").addClass("bcr_pad").removeClass("without_back");
                $(".b_bottom_right").addClass("bbr_pad").removeClass("without_back");
                $(".b_bottom_left").addClass("bbl_pad").removeClass("without_back");
                $(".b_bottom_center").addClass("bbc_pad").removeClass("without_back");
                $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
                $("#wgt_help").addClass("pad_color").addClass("pad_help");
                $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
                $("#wgt_name").addClass("pad_color");
                $("#wgt_display").removeClass("display_wood");
                $("#style_select option:first").next().attr('selected',true);
                $("body, html").removeClass("without_radius").removeClass("radius_ft");
                break;
            case "3":
                $(".b_top_left").addClass("without_back").removeClass("btl_pad");
                $(".b_top_center").addClass("without_back").removeClass("btc_pad");
                $(".b_top_right").addClass("without_back").removeClass("btr_pad");
                $(".b_center_left").addClass("without_back").removeClass("bcl_pad");
                $(".b_center_right").addClass("without_back").removeClass("bcr_pad");
                $(".b_bottom_right").addClass("without_back").removeClass("bbr_pad");
                $(".b_bottom_left").addClass("without_back").removeClass("bbl_pad");
                $(".b_bottom_center").addClass("without_back").removeClass("bbc_pad");
                $("#wgt_help").addClass("pad_color").addClass("pad_help");
                $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
                $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
                $("#wgt_name").addClass("pad_color");
                $("#wgt_display").removeClass("display_wood");
                $("#style_select option:last").attr('selected',true);
                $("body, html").addClass("without_radius").removeClass("radius_ft");
                break;
        }
    }
    
});