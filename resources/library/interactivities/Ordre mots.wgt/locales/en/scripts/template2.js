
/*
	Sankore API
*/

function startEditing()
{
    modeEdit();
}

function stopEditing()
{
    modeView();
}

function initialize()
{
	
}

function checkResponse()
{
    checkWord();
}
/*
	/ END sankore api
*/

var sankoreLang = {
    view: "Display", 
    edit: "Edit",
    example: "a*long*,*long*time*ago*...",
    wgt_name: "Order words",
    reload: "Reload",
    slate: "slate",
    pad: "pad",
    none: "none",
    help: "Help",
    help_content: 
"<p><h2>Order words </h2> </p>" +
"<p><h3>Order words to reconstruct a correct sentence</h3></p>" +

"<p>The interactivity displays labels in a random order. Drag and drop words in correct order. If the result is correct, the area turns in green.</p> "+
"<p>“Reload” button resets the exercises. </p>" +

"<p> Enter the “Edit” mode to : </p>" +
"<ul> <li> choose the theme of the App : pad, slate, or none (by default : none), </li>" +
"<li> determine the sentence and place separation between words. </li> </ul>" +

 "<p>To create a new exercise : </p>" +
"<ul><li>insert the desired sentence,</li>" +
"<li> add spaces in the sentence, inserting “*” between each word. Do not place a star at the beginning or the end of your sentence.</li></ul>" +
"<p>“Display” button comes back to the activity.</p>",
    theme: "Theme"
};

var word = "";
var curWord = "";

if(window.sankore){
    word = (sankore.preference("rightOrdWords", ""))?sankore.preference("rightOrdWords", ""):sankoreLang.example;
    curWord = (sankore.preference("currentOrdWords", ""))?sankore.preference("currentOrdWords", ""):"";
} else {
    word = sankoreLang.example;
}

// array of dom elements
var letters = [];

var editMode = false; // just a flag

var wgtState = false; // just another flag

// if use the "edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

// hardcoded parameters, not very good
var input_width = 606;
var widget_padding = 0;
var min_view_width = 400;


$(document).ready(function(){
    if(window.sankore){
        if(sankore.preference("ord_words_style","")){
            changeStyle(sankore.preference("ord_words_style",""));
        } else
            changeStyle("3")
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
                modeView();
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $("#parameters").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
                modeEdit();
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
        if(wgtState)
            $("#wgt_display").trigger("click");
        else
        {
            $( "#mp_word" ).empty();
	
            // create new set of letters
            var letters;
            letters = shuffle( createWordLetters( word ) );
    
            for( i in letters ){
                $("#mp_word").append( letters[i] );
            }
	
            // in sankore api there would be a function to check 
            // the answer, so no update parameter would be needed
            if( !isSankore ){
                $( "#mp_word" ).sortable( {
                    update: checkWord
                } );
            } else $( "#mp_word" ).sortable();

            // adjustWidth
            var totalLettersWidth = 0;
            for( i in letters ){
                var currentWidth = $( letters[i] ).outerWidth( true );
                totalLettersWidth += currentWidth;
            }
            totalLettersWidth += 1;

            var width = Math.max(
                totalLettersWidth,
                min_view_width
                );
	
            // shift the words to the right to center them
            if( width > totalLettersWidth ){
                $( "#mp_word" ).css( "margin-left", Math.round( (width - totalLettersWidth)/2 ) );
            }
            else{
                $( "#mp_word" ).css( "margin-left", 0 );
            }
        }
    });    
    
})

/*
=================
createWordLetters
=================
returns array of dom elements
*/
function createWordLetters( word )
{
    var ch, el;
    var letters = [];
	
    if( word.indexOf( '*' ) != -1 )
    {
        var tmp = word.split( '*' );
        for( i in tmp )
        {
            ch = tmp[i];
            el = document.createElement( "div" );
            $(el).addClass( "letter" ).text( ch );
            letters.push( el );
        }
    }
    else
    {
        for( var i = 0; i < word.length; i++ )
        {
            ch = word.charAt( i );
            el = document.createElement( "div" );
            $(el).addClass( "letter" ).text( ch );
            letters.push( el );
        }
    }
    return letters;
}

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

/*
=====================
checkWord
=====================
scans the letters and checks 
if they are in the right order
*/
function checkWord()
{	
    var str = "";
    $( "#mp_word .letter" ).each( function(){
        str += $(this).text();
    });
    var w = word;
    while( w.indexOf( '*' ) != -1 )
    {
        w = w.replace( '*', '' );
    }
    //alert(str + " | " + w)
    if( str == w ){
        $( "#mp_word .letter" ).addClass( "right" );
    //message( "Right!" );
    } else {
        $( "#mp_word .letter" ).removeClass( "right" );
    }
}

/*
==============
shuffle
==============
shuffles an array
*/
function shuffle( arr )
{
    var pos, tmp;
	
    for( var i = 0; i < arr.length; i++ )
    {
        pos = Math.round( Math.random() * ( arr.length - 1 ) );
        tmp = arr[pos];
        arr[pos] = arr[i];
        arr[i] = tmp;
    }
    return arr;
}



/*
==================
modeView()
==================
turns the widget into the view mode
*/
function modeView()
{
    if( editMode ){
        word = $( "#mp_word .wgt_cont" ).val();
    }
    
    wgtState = false;
	
    // clean the previous word
    $( "#mp_word" ).empty();
	
    // create new set of letters
    var letters;
    if(window.sankore && curWord && !editMode)
        letters = createWordLetters( curWord );
    else
        letters = shuffle( createWordLetters( word ) );
    
    for( i in letters ){
        $("#mp_word").append( letters[i] );
    }
	
    // in sankore api there would be a function to check 
    // the answer, so no update parameter would be needed
    if( !isSankore ){
        $( "#mp_word" ).sortable( {
            update: checkWord
        } );
    } else $( "#mp_word" ).sortable();

    // adjustWidth
    var totalLettersWidth = 0;
    for( i in letters ){
        var currentWidth = $( letters[i] ).outerWidth( true );
        totalLettersWidth += currentWidth;
    }
    totalLettersWidth += 1;

    var width = Math.max(
        totalLettersWidth,
        min_view_width
        );
	
    // shift the words to the right to center them
    if( width > totalLettersWidth ){
        $( "#mp_word" ).css( "margin-left", Math.round( (width - totalLettersWidth)/2 ) );
    }
    else{
        $( "#mp_word" ).css( "margin-left", 0 );
    }
	
    checkWord();
}

/*
================
modeEdit
================
*/
function modeEdit()
{
    editMode = true;
    wgtState = true;
    $( "#mp_word").sortable( "destroy" );
    $( "#mp_word").css( "margin-left", 0 ).empty().append('<textarea class="wgt_cont">'+word+'</textarea>');

}

if (window.widget) {
    window.widget.onleave = function(){
        sankore.setPreference("ord_words_style", $("#style_select").find("option:selected").val());
        if($( "#mp_word .wgt_cont" ).val())
        {
            modeView();
            var str = "";
            $( "#mp_word .letter" ).each( function(){
                str += $(this).text() + "*";
            });        
            str = str.substr(0, str.length - 1);        
            sankore.setPreference("currentOrdWords", str);           
            modeEdit();
        }
        else{
            str = "";
            $( "#mp_word .letter" ).each( function(){
                str += $(this).text() + "*";
            });        
            str = str.substr(0, str.length - 1);        
            sankore.setPreference("currentOrdWords", str);
        }
        sankore.setPreference("rightOrdWords", word);
    }
}
