
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
    edit: "Upraviť",
    view: "Zobraziť",
    example: "toto\nje\nveta",
    wgt_name: "Zoradenie slov. spojení",
    reload: "Obnoviť",
    slate: "bridlica",
    pad: "tablet",
    none: "žiadny",
    help: "Pomoc",
    help_content: 
"<p><h2> Zoradenie slovných spojení</h2></p>" +
"<p><h3> Úprava textu zmenou poradia slov.</h3> </p>" +
"<p>Interaktívna aktivita zobrazí slovné spojenia v náhodnom poradí. Treba ich správne zoradiť. Ak sa to žiakom podarí, slová sa zafarbia dozelena.</p> "+

"<p>Tlačidlom „Obnoviť“ vrátite cvičenie do pôvodného stavu.</p>" +

"<p>Po stlačení tlačidla „Upraviť“ môžete v&nbsp;režime úprav:</p>" +
"<ul> <li> zmeniť farebný motív na tablet, bridlicu alebo na žiadny (predvolený je žiadny), </li>" +
"<li>oddeliť slovné spojenia vo vete. </li></ul>" +

"<p>Ak chcete vytvoriť nové cvičenie: </p>" +
"<ul><li>text zadávajte do textového poľa,</li>" +
"<li>jednotlivé slová a slovné spojenia, ktoré majú žiaci zoradiť, píšte do nového riadka.</li></ul> "+
"<p>Tlačidlom „Zobraziť“ sa z režimu úprav vrátite na aktivitu.</p>",
    theme: "Farebný motív"
}


var sentence = "";
var curSentence = "";

if(window.sankore){
    sentence = (sankore.preference("rightOrdPhrases", ""))?sankore.preference("rightOrdPhrases", ""):sankoreLang.example;
    curSentence = (sankore.preference("currentOrdPhrases", ""))?sankore.preference("currentOrdPhrases", ""):"";
} else {
    sentence = sankoreLang.example;
}

var doCheck = true;

// array of dom elements
var phrases = [];

var editMode = false; // just a flag

var wgtState = false; // just another flag

// if use the "edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

// hardcoded parameters, not very good
var input_width = 606;
var widget_padding = 0;

$(document).ready(function(){
    if(window.sankore)
        if(sankore.preference("ord_phrases_style","")){
            changeStyle(sankore.preference("ord_phrases_style",""));
            $("#style_select").val(sankore.preference("ord_phrases_style",""));
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
	
            var phrases;
            phrases = shuffle( createElements( sentence ) );
    
            for( i in phrases ){
                $("#mp_word").append( phrases[i] );
            }
	
            // in sankore api there would be a function to check 
            // the answer, so no update parameter would be needed
            $( "#mp_word" ).sortable();
            if( !isSankore ){
                $( "#mp_word" ).sortable( {
                    update: checkSentence
                } );
            } else 
                $( "#mp_word" ).sortable();
        }
    });
    
    
})

function str_replace( w, b, s ){
    while( s.indexOf( w ) != -1 ){
        s = s.replace( w, b );
    }
    return s;
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
=================
createWordLetters
=================
returns array of dom elements
*/
function createElements( sentence )
{
    var el;
    var elements = [];
    var phrases = sentence.split( "\n" );
	
    for( i in phrases )
    {
        el = document.createElement( "div" );
        $( el ).addClass( "phrase" ).text( phrases[i] );
        elements.push( el );
    }
    return elements;
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

function checkSentence()
{
    if( !doCheck )
        return;
		
    var ph = [];
    $( "#mp_word .phrase" ).each( function()
    {
        ph.push( $( this ).text() );
    });
    if( ph.join( "\n" ) == str_replace( "\r", "", sentence ) ){
        $( "#mp_word .phrase" ).addClass( "right" );
    } else {
        $( "#mp_word .phrase" ).removeClass( "right" );
    }
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
        sentence = str_replace( "\r", "", $( "#mp_word textarea" ).attr( "value" ) );
        var p = sentence.split( "\n" );
        var p2 = [];
        var t;
        for( var i in p )
        {
            t = jQuery.trim( p[i] );
            if( t ) p2.push( t );
        }
        sentence = p2.join( "\n" );
    }
	
    wgtState = false;
    
    // clean the previous word
    $( "#mp_word" ).empty();
	
    var phrases;
    // create new set of elements
    if(window.sankore && curSentence && !editMode)
        phrases = createElements( curSentence );
    else
        phrases = shuffle( createElements( sentence ) );
    
    for( i in phrases ){
        $("#mp_word").append( phrases[i] );
    }
	
    // in sankore api there would be a function to check 
    // the answer, so no update parameter would be needed
    $( "#mp_word" ).sortable();
    if( !isSankore ){
        $( "#mp_word" ).sortable( {
            update: checkSentence
        } );
    } else 
        $( "#mp_word" ).sortable();
    
    checkSentence();
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

    $( "#mp_word").css( "margin-left", 0 ).empty()
    .append('<textarea cols="50" rows="5">'+sentence+'</textarea>');
}

if (window.widget) {
    window.widget.onleave = function(){
        sankore.setPreference("ord_phrases_style", $("#style_select").find("option:selected").val());
        if($( "#mp_word textarea" ).val())
        {
            modeView();
            var ph = [];
            $( "#mp_word .phrase" ).each( function()
            {
                ph.push( $( this ).text() );
            });	
            sankore.setPreference("currentOrdPhrases", ph.join( "\n" ));            
            modeEdit();
        }
        else{
            ph = [];
            $( "#mp_word .phrase" ).each( function()
            {
                ph.push( $( this ).text() );
            });		
            sankore.setPreference("currentOrdPhrases", ph.join( "\n" ));
        }
        sankore.setPreference("rightOrdPhrases", sentence);
    }
}
