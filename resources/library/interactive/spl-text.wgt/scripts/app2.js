


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
	
	w.maxWidth = 600;
	
	w.setEditContent( '<div class="inputwrap"><textarea class="percent">hello, this is the first sentence. hi, this is the second sentence. hello again, this is the third sentence. good morning, this is the fifth sentence. hi, sorry, i\'m late, i\'m the fourth sentence.</textarea></div>' );
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
		
		return { w: w, h: h+dh };
	};
	
	// editSize
	w.editSize = function()
	{
		return {
			w: winstance.elements.containerEdit.find( "textarea" ).parent().outerWidth( true ),
			h: winstance.elements.containerEdit.find( "textarea" ).parent().outerHeight( true ),
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
		
		console.log( text );
		
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
	
});