
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
    view: "Afficher", 
    edit: "Modifier",
    example: "Il*était*une*fois*..."
};

var word = "";
var curWord = "";

if(window.sankore){
    word = (sankore.preference("rightOrdWords", ""))?sankore.preference("rightOrdWords", ""):sankoreLang.example;
    curWord = (sankore.preference("currentOrdWords", ""))?sankore.preference("currentOrdWords", ""):"";
} else {
    word = sankoreLang.example;
}

var doCheckWord = true;

// array of dom elements
var letters = [];

var editMode = false; // just a flag

// if use the "edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

// hardcoded parameters, not very good
var input_width = 606;
var widget_padding = 65;
var min_view_width = 400;

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


/*
=====================
checkWord
=====================
scans the letters and checks 
if they are in the right order
*/
function checkWord()
{
    if( !doCheckWord )
        return;
		
    var str = "";
    $( "#mp_word .letter" ).each( function(){
        str += $(this).text();
    });
    var w = word;
    while( w.indexOf( '*' ) != -1 )
    {
        w = w.replace( '*', '' );
    }
    if( str == w ){
        $( "#mp_word .letter" ).addClass( "right" );
    //message( "Right!" );
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
        word = $( "#mp_word input:text" ).attr( "value" );
    }
	
    // if no sankore api, insert edit button
    if( !isSankore ){
        $( "#mp_setup" ).empty().append( '<input type="button" value="' + sankoreLang.edit + '">' );
        $( "#mp_setup input:button" ).click( function(){
            modeEdit();
        });
    }
	
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
	
    // apply new width
    adjustWidth( width );
	
}

/*
============
adjustWidth
============
*/
function adjustWidth( width )
{
    $( "#ub-widget" ).animate( {
        width: width
    } );
    // if viewed as a widget, resize the window
    if( !isBrowser ){
        window.resizeTo( width + widget_padding, widget.height );
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
        $( "#mp_setup" ).empty().append( '<input type="button" value="' + sankoreLang.view + '">' );
        $( "#mp_setup input:button" ).click( function(){
            modeView();
        });
    }
    $( "#mp_word").css( "margin-left", 0 ).empty().append('<input value="'+word+'">');
    adjustWidth( input_width );
}

if (window.widget) {
    window.widget.onleave = function(){
        if($( "#mp_word input:text" ).attr( "value" ))
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