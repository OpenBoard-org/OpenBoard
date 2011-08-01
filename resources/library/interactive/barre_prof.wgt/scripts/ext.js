Array.prototype.shuffle = function( b )
{
	var i = this.length, j, t;
	while( i )
	{
		j = Math.floor( ( i-- ) * Math.random() );
		t = b && typeof this[i].shuffle!=='undefined' ? this[i].shuffle() : this[i];
		this[i] = this[j];
		this[j] = t;
	}
	return this;
};

Array.prototype.copy = function()
{
	var copy = new Array();
	for( var i = 0; i < this.length; i++ ){
		copy[i] = this[i];
	}
	return copy;
}


String.prototype.copy = function()
{
	return this.substring( 0, this.length );
};

String.prototype.trim = function( ext )
{
	var chars = [
		" ", "\t", "\n", "\r"
	];
	
	var s = this.copy();
	
	if( arguments.length > 0 ){
		for( var i in ext ){
			chars.push( ext[i] );
		}
	}
	
	while( chars.indexOf( s.charAt( 0 ) ) != -1 ){
		s = s.substring( 1, s.length );
	}
	while( chars.indexOf( s.charAt( s.length-1 ) ) != -1 ){
		s = s.substring( 0, s.length-1 );
	}
	return s;
};

String.prototype.replaceAll = function( search, replace )
{
	var s = this.copy();
	while( s.indexOf( search ) != -1 ){
		s = s.replace( search, replace );
	}
	return s;
};