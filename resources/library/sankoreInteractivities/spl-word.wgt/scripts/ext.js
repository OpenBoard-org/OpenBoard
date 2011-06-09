
// http://javascript.about.com/library/blshuffle.htm
Array.prototype.shuffle = function()
{
	var s = [];
	while (this.length) s.push(this.splice(Math.random() * this.length, 1));
	while (s.length) this.push(s.pop());
	return this;
};

Array.prototype.indexOf = function( element )
{
	for( var i in this )
	{
		if( this[i] == element )
			return i;
	}
	return -1;
};

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