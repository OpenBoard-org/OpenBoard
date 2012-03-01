


function createElements( phrase )
{
    var s = '';
    for( var i = 0; i < phrase.length; i++ )
    {
        ch = phrase.charAt( i );
        if( ch == " " ){
            ch = "&nbsp;";
        }
		
        s += '<div class="letter">' + ch + '</div>' +
        '<div class="dash">&nbsp;</div>';
    }
    return s;
}


$(document).ready(function()
{
    var w = new wcontainer( "#ub-widget" );
    var sentence = "";
        
    if(window.sankore)
        sentence = (sankore.preference("ordSplPhrases", ""))?sankore.preference("ordSplPhrases", ""):sankoreLang.example;
    else
        sentence = sankoreLang.example;
    
	
    w.maxWidth = 600;
	
    w.setEditContent( '<div class="inputwrap"><textarea class="percent">' + sentence + '</textarea></div>' );
    w.setViewContent( '<div class="upper"><div class="dash fixed">. </div></div>' );
    w.setData( "dashWidth", w.elements.container.find( ".dash" ).outerWidth() );
    w.setViewContent( '<div class="upper"><div class="dash fixed">M</div></div>' );
    w.setData( "lineHeight", w.elements.container.find( ".dash" ).outerHeight() );
    w.setViewContent( "" );
	
	
    // onViewMode
    w.onViewMode = function()
    {
        // clean up the text
        var phrase = w.elements.container.find( "textarea" ).val()
        .replace( /\r/g, '' ).replace( /\n/g, ' ' ).replace( /  /g, ' ' ).trim();
		
        // store the text
        w.setData( "phrase", phrase );
		
        // remove all dots (they are to be set during the exercise)
        phrase = phrase.replace( / /g, '' );
		
        // create the html
        w.setViewContent( createElements( phrase ) );
		
		
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
            w.adjustSize();
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
        var phrase = "";
        var ch = "";
        this.elements.containerView.find( "div:visible" ).each( function()
        {
            if( this.className.indexOf( "fixed" ) != -1 ){
                phrase += ' ';
            }
            else if( this.className.indexOf( "dash" ) != -1 ){
                return;
            }
            else{
                ch = $( this ).html();
                phrase += ch;
            }
        });
		
        if( phrase == this.getData( "phrase" ) ){
            this.elements.containerView.addClass( "answerRight" );
        }
        else{
            this.elements.containerView.removeClass( "answerRight" );
        }
    };

	
    window.w = w;
    window.winstance = w;
	
    w.modeView();
    
    if (window.widget) {
        window.widget.onleave = function(){
            if(w.editMode)
                sankore.setPreference("ordSplPhrases", w.elements.container.find( "textarea" ).val());
            else
                sankore.setPreference("ordSplPhrases", w.getData("phrase"));
        }
    }
	
});