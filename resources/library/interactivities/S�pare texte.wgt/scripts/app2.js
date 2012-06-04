


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

if (window.widget) {
    window.widget.onleave = function(){
        sankore.setPreference("spl_text_style", $(".style_select").find("option:selected").val());
        if(w.editMode)
            sankore.setPreference("ordSplText", w.elements.container.find( "textarea" ).val());
        else
            sankore.setPreference("ordSplText", w.getData( "text" ));
    }
}

$(document).ready(function()
{
    if(sankore.preference("spl_text_style","")){
        changeStyle(sankore.preference("spl_text_style",""));
        $(".style_select").val(sankore.preference("spl_text_style",""));
    } else
        changeStyle(1)
    $("#wgt_display").text(sankoreLang.view);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){                
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $(".style_select").css("display","none");                
                $(this).css("display", "none");
                $("#wgt_edit").css("display", "block");
                modeView();
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $(".style_select").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
                modeEdit();
            }
        }
    });
    $("#wgt_name").text(sankoreLang.wgt_name);
    $("#wgt_reload").text(sankoreLang.reload).click(function(){
        window.location.reload();
    });
    
    $(".style_select option[value='1']").text(sankoreLang.slate);
    $(".style_select option[value='2']").text(sankoreLang.pad);
    
    $(".style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
    
    var w = new wcontainer( "#ub-widget" );
    
    var sentences = "";
    
    if(window.sankore)
        sentences = (sankore.preference("ordSplText", ""))?sankore.preference("ordSplText", ""):sankoreLang.example;
    else
        sentences = sankoreLang.example;
    
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
        if(val == 1){
            $(".b_top_left").removeClass("btl_pad");
            $(".b_top_center").removeClass("btc_pad");
            $(".b_top_right").removeClass("btr_pad");
            $(".b_center_left").removeClass("bcl_pad");
            $(".b_center_right").removeClass("bcr_pad");
            $(".b_bottom_right").removeClass("bbr_pad");
            $(".b_bottom_left").removeClass("bbl_pad");
            $(".b_bottom_center").removeClass("bbc_pad");
            $("#wgt_reload").removeClass("pad_color").removeClass("pad_reload");
            $("#wgt_edit").removeClass("pad_color").removeClass("pad_edit");
            $("#wgt_display").removeClass("pad_color").removeClass("pad_edit");
            $("#wgt_name").removeClass("pad_color");
            $(".style_select").removeClass("pad_select");
        } else {
            $(".b_top_left").addClass("btl_pad");
            $(".b_top_center").addClass("btc_pad");
            $(".b_top_right").addClass("btr_pad");
            $(".b_center_left").addClass("bcl_pad");
            $(".b_center_right").addClass("bcr_pad");
            $(".b_bottom_right").addClass("bbr_pad");
            $(".b_bottom_left").addClass("bbl_pad");
            $(".b_bottom_center").addClass("bbc_pad");
            $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_display").addClass("pad_color").addClass("pad_edit");
            $("#wgt_name").addClass("pad_color");
            $(".style_select").addClass("pad_select");
        }
    }
    
});