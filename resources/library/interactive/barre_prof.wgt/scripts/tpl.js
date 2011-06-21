function Tpl( obj )
{
	var templates = {};
	this.load = function( obj )
	{
		templates = obj;
	};
	
	this.get = function( name )
	{
		return templates[name];
	};
	
	this.getParsed = function( name, obj )
	{
		var s = templates[name];
		for( var i in obj ){
			s = s.replaceAll( "{"+i+"}", obj[i] );
		}
		return s;
	};
	
	this.getParsedMulti = function( nao ) // nao = names and objects array
	{
		var html = "";
		for( var i = 0; i < nao.length; i++ )
		{
			if( nao[i].length > 1 ){
				html += this.getParsed( nao[i][0], nao[i][1] );
			}
			else{
				html += this.get( nao[i][0] );
			}
		}
		return html;
	};
	
	if( arguments.length > 0 ){
		this.load( obj );
	}
}