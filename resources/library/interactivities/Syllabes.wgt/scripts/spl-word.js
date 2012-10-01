//just a flag
var flag = true;

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

    if (window.widget) {
        window.widget.onleave = function(){
            sankore.setPreference("spl_word_style", $("#style_select").find("option:selected").val());
            if(w.editMode){
                sankore.setPreference("ordSplWords", w.elements.container.find( "input" ).val().trim( ['*'] ));
                sankore.setPreference("ordSplWordsState", "0");
            }
            else{
                sankore.setPreference("ordSplWordsCode", $(".viewmode").html());
                sankore.setPreference("ordSplWordsState", "1");
                sankore.setPreference("ordSplWordsAnswer", ($("#ub-widget").hasClass("answerRight"))?"answerRight":"");
                sankore.setPreference("ordSplWords", w.getData( "word" ));              
            }
        }
    }

    if(sankore.preference("spl_word_style","")){
        changeStyle(sankore.preference("spl_word_style",""));
        $("#style_select").val(sankore.preference("spl_word_style",""));
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

    // onViewMode
    w.onViewMode = function()
    {
        var word = w.elements.container.find( "input" ).val().trim( ['*'] );
        w.setData( "word", word );
        word = word.replace( /\*/g, '' );
        
        if(sankore.preference("ordSplWordsState", "") == "1" && flag){
            $(".viewmode").html(sankore.preference("ordSplWordsCode", ""));
            if(sankore.preference("ordSplWordsAnswer", ""))
                $("#ub-widget").addClass(sankore.preference("ordSplWordAnswer", ""));
            flag = false;
        } 
        else
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