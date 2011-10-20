
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



var sentence = "";
var curSentence = "";

if(window.sankore){
    sentence = (sankore.preference("rightOrdPhrases", ""))?sankore.preference("rightOrdPhrases", ""):"this is\nan example\nsentence";
    curSentence = (sankore.preference("currentOrdPhrases", ""))?sankore.preference("currentOrdPhrases", ""):"";
} else {
    sentence = "this is\nan example\nsentence";
}

var doCheck = true;

// array of dom elements
var phrases = [];

var editMode = false; // just a flag

// if use the "edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

// hardcoded parameters, not very good
var input_width = 606;
var widget_padding = 100;


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
	
    // if no sankore api, insert edit button
    if( !isSankore ){
        $( "#mp_setup" ).empty().append( '<input type="button" value="Edit">' );
        $( "#mp_setup input:button" ).click( function(){
            modeEdit();
        });
    }
	
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

    // adjustHeight
    var aHeight = $( phrases[0] ).outerHeight( true );
	
    // apply new width
    adjust( aHeight * phrases.length );
	
}

/*
============
adjust width or height
============
*/
function adjust( height )
{
    $( "#mp_word" ).animate( {
        height: height
    } );
    // if viewed as a widget, resize the window
    if( !isBrowser ){
        window.resizeTo( widget.width, height + widget_padding );
    }
}

/*
================
modeEdit
================
*/
function modeEdit()
{
    editMode = true;
    // if no sankore api, insert ok button
    if( !isSankore )
    {
        $( "#mp_setup" ).empty().append( '<input type="button" value="OK">' );
        $( "#mp_setup input:button" ).click( function(){
            modeView();
        });
    }
    $( "#mp_word").css( "margin-left", 0 ).empty()
    .append('<textarea cols="50" rows="5">'+sentence+'</textarea>');
    adjust( $( "#mp_word textarea" ).outerHeight() );
}

$(window).mouseout(function(){
    if(window.sankore){
        var ph = [];
        $( "#mp_word .phrase" ).each( function()
        {
            ph.push( $( this ).text() );
        });
		
        sankore.setPreference("currentOrdPhrases", ph.join( "\n" ));
        sankore.setPreference("rightOrdPhrases", sentence);

    }
});

$(document).ready(function()
{
    $("#ub-widget").append( '\
		<div id="mp_setup"></div>\
		<div id="mp_word"></div>\
	');
    modeView();
});