
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

$(document).ready(function()
{
	var w = new wcontainer( "#ub-widget" );
	
	w.setEditContent( '<div class="inputwrap"><input class="percent" value="so*phis*ti*ca*ted"></div>' );
	w.setViewContent( '<span class="upper"><span class="dash fixed">&mdash;</span></span>' );
	w.setData( "dashWidth", w.elements.container.find( "span.dash" ).outerWidth() );
	w.setViewContent( "" );
	
	// onViewMode
	w.onViewMode = function()
	{
		var word = w.elements.container.find( "input" ).val().trim( ['*'] );
		
		w.setData( "word", word );
		word = word.replace( /\*/g, '' );
		w.setViewContent( createWord( word ) );
		w.elements.container.find( "span.upper" ).click( function(){
			$( this ).find( "span.dash" ).toggleClass( "fixed" );
			w.adjustSize();
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
		
		return { w: w+dw, h: h };
	};
	
	// editSize
	w.editSize = function()
	{
		return {
			w: winstance.elements.containerEdit.find( "input" ).outerWidth( true ),
			h: winstance.elements.containerEdit.find( "input" ).outerHeight( true ),
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
	
});