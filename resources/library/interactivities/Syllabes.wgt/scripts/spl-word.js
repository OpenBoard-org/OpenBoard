
function createWord( word )
{
    var s = '';
    for( var i = 0; i < word.length; i++ )
    {
        ch = word.charAt( i );
        s += 
        '<span class="upper">' +
        '<span>' + ch + '</span>' +
        (( i < word.length - 1 ) ?
            '<span class="dash">&ndash;</span>' : '' ) +
        '</span>';
    }
    return s;
}

var w;

$(document).ready(function()
{
    if(sankore.preference("spl_word_style","")){
        changeStyle(sankore.preference("spl_word_style",""));
        $(".style_select").val(sankore.preference("spl_word_style",""));
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
                w.modeView();
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $(".style_select").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
                w.modeEdit();
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
    var words = "";
        
    if(window.sankore)
        words = (sankore.preference("ordSplWords", ""))?sankore.preference("ordSplWords", ""):sankoreLang.example;
    else
        words = sankoreLang.example;
    w.setEditContent( '<div class="inputwrap"><input class="percent" value="' + words + '"></div>' );
    w.setViewContent( '<span class="upper"><span class="dash fixed">&mdash;</span></span>' );
    w.setData( "dashWidth", w.elements.container.find( "span.dash" ).outerWidth() );
    w.setViewContent( "" );

    w.elements.container.find( "input" ).live("change", function(){
        sankore.setPreference("spl_word_style", $(".style_select").find("option:selected").val());
        if(w.editMode)
            sankore.setPreference("ordSplWords", w.elements.container.find( "input" ).val().trim( ['*'] ));
        else
            sankore.setPreference("ordSplWords", w.getData( "word" ));
    })
	
    // onViewMode
    w.onViewMode = function()
    {
        var word = w.elements.container.find( "input" ).val().trim( ['*'] );
        w.setData( "word", word );
        word = word.replace( /\*/g, '' );
        w.setViewContent( createWord( word ) );
        w.elements.container.find( "span.upper" ).click( function(){
            $( this ).find( "span.dash" ).toggleClass( "fixed" );
            w.checkAnswer();
        });
        w.checkAnswer();
    };
	
    // viewSize
    w.viewSize = function()
    {
        var w = 0;
        var h = 0;
		
        winstance.elements.containerView.find( "span.upper span:visible" ).each( function()
        {
            w += $( this ).outerWidth();
            h = Math.max( h, $( this ).outerHeight() );
        });
        var dw = winstance.getData( "dashWidth" );
		
        return {
            w: w+dw, 
            h: h
        };
    };
	
    // editSize
    w.editSize = function()
    {
        return {
            w: winstance.elements.containerEdit.find( "input" ).outerWidth( true ),
            h: winstance.elements.containerEdit.find( "input" ).outerHeight( true )
        };
    };
	
	
    w.checkAnswer = function()
    {
        var word = "";
        this.elements.container.find( "span.upper span:visible" ).each( function()
        {
            if( this.className.indexOf( "fixed" ) != -1 ){
                word += '*';
            }
            else if( this.className.indexOf( "dash" ) != -1 ){
                return;
            }
            else{
                word += $( this ).text();
            }
        });
		
        if( word == this.getData( "word" ) ){
            this.elements.container.addClass( "answerRight" );
        }
        else{
            this.elements.container.removeClass( "answerRight" );
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