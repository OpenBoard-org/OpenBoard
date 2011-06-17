// if use the "view/edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

function wcontainer( containerID )
{
	// some protecred variables
	var thisInstance = this;
	var editMode = false;
	var data = {}; // see setData and getData
	this.allowResize = false;
	this.maxWidth = 1000; // set to 0 for no max width restriction
	this.minWidth = 50;
	this.maxHeight = 800; // set to 0 for no max width restriction
	this.minHeight = 50;
	
	// links to the elements of the widget
	this.elements = {};
	
	/*
	============
	create
	============
	- creates html base, inits this.elements, assings events
	*/
	this.create = function( containerID )
	{
		var html = 
			'<div id="mp_setup">' +
				'<div class="viewmode">' +
					'<button>Edit</button>' +
				'</div>' +
				'<div class="editmode">' +
					'<button>View</button>' +
				'</div>' +
			'</div>' +
			'<div id="mp_content">' +
				'<div class="viewmode" id="mp_view">' +
				'</div>' +
				'<div class="editmode" id="mp_edit">' +
				'</div>' +
			'</div>';
		
		var container = $( containerID );
		
		container.append( html );
		this.elements.edit = container.find( ".editmode" );
		this.elements.view = container.find( ".viewmode" );
		
		this.elements.container = container;
		this.elements.subcontainer = container.find( "#mp_content" );
		this.elements.setup = container.find( "#mp_setup" );
		this.elements.splash = $( "<div>" );
		
		this.elements.containerView = this.elements.subcontainer.find( ".viewmode" );
		this.elements.containerEdit = this.elements.subcontainer.find( ".editmode" );
		
		
		
		
		container.find( ".viewmode button" ).click( function(){
			thisInstance.modeEdit();
		} );
		
		container.find( ".editmode button" ).click( function(){
			thisInstance.modeView();
		} );
		
	};
	
	this.setSplashContent = function( html )
	{
		this.elements.container.find( "#mp_setup .viewmode" ).append(
			'<button class="hideAll">Hide</button>'
		);
		this.elements.container.find( "#mp_setup .editmode" ).append(
			'<button class="hideAll">Hide</button>'
		);
		this.elements.container.after( 
			'<div id="mp_splash">'+ html +'</div>'
		);
		
		this.elements.splash = $( "#mp_splash" );
		this.elements.splash.hide();
		this.elements.splash.click( function()
		{
			winstance.modeSplash( false );
		});
		
		this.elements.setup.find( "button.hideAll" ).click( function()
		{
			winstance.modeSplash();
		});
	};
	
	
	/*
	===============
	setViewContent
	===============
	- assigns custom html to the viewmode container
	*/
	this.setViewContent = function( html )
	{
		this.elements.container.find( "#mp_content .viewmode" ).html( html );
	};
	this.getViewContent = function()
	{
		return this.elements.container.find( "#mp_content .viewmode" ).html();
	};
	
	/*
	===============
	setEditContent
	===============
	- assigns custom html to the editmode container
	*/
	this.setEditContent = function( html )
	{
		this.elements.container.find( "#mp_content .editmode" ).html( html );
	};
	this.getEditContent = function( html )
	{
		return this.elements.container.find( "#mp_content .editmode" ).html();
	};
	
	
	
	/*
	=========================
	modeEdit and modeView
	=========================
	- switch the widget betweed modes
	* for customization extend onEditMode and onViewMode
	*/
	this.modeEdit = function()
	{
		this.onEditMode();
		editMode = true;
		this.elements.edit.removeClass( "hide" );
		this.elements.view.addClass( "hide" );
		
		this.adjustSize();
	};
	this.modeView = function()
	{
		this.onViewMode();
		editMode = false;
		this.elements.edit.addClass( "hide" );
		this.elements.view.removeClass( "hide" );
		
		this.adjustSize();
	};
	this.modeSplash = function( enable )
	{
		if( arguments.length < 1 ){
			var enable = true;
		}
		if( enable ){
			this.allowResize = false;
			this.elements.container.hide();
			this.elements.splash.show();
			// hack, need to work out
			this.adjustSize( 190, 110, true );
		}
		else{
			this.elements.container.show();
			this.elements.splash.hide();
			this.adjustSize();
			this.allowResize = true;
		}
	};
	
	
	
	/*
	================
	adjustSize
	================
	- changes the widget size (window and container)
	*/
	this.adjustSize = function( width, height, override )
	{
		if( arguments.length < 3 ){
			var override = false;
		}
		
		// retrieve the arguments
		if( arguments.length < 2 )
		{
			var s = ( editMode )? this.editSize() : this.viewSize();
			var width = s.w;
			var height = s.h;
		}
		
		// check for validity
		if( width + height == 0 )
			return;
		
		// add view/edit bar height
		if( !isSankore && !override ){
			height += $( this.elements.container ).find( "#mp_setup" ).outerHeight();
		}
		
		if( !override )
		{
			// apply min and max restrictions
			width = Math.max( this.minWidth, width );
			height = Math.max( this.minHeight, height );
			if( this.maxWidth ){
				width = Math.min( width, this.maxWidth );
			}
		}
		
		// if viewed as a widget, resize the window
		if( !isBrowser )
		{
			// dw and dh are paddings+margins+borders
			var dw = this.getData( "dw" );
			var dh = this.getData( "dh" );
			
			if( override ) dw = dh = 0;
			
			if( width == 0 ){
				width = widget.width;
			}
			if( height == 0 ){
				height = widget.height;
			}
			window.resizeTo( width + dw, height + dh );
		}
		
		// resize the container
		var params = {};
		if( width != 0 ){
			params.width = width;
		}
		if( height != 0 ){
			params.height = height;
		}
		console.log( params );
		this.elements.container.animate( params );
		
	};
	
	/*
	======================
	setData and getData
	======================
	- store some data inside
	*/
	this.setData = function( name, value ){
		data[name] = value;
	};
	this.getData = function( name ){
		if( typeof( data[name] ) == "undefined" ){
			return null;
		} else return data[name];
	};
	
	this.viewSizeDefault = function(){
		return {
			w: this.elements.containerView.outerWidth(),
			h: this.elements.containerView.outerHeight()
		};
	};
	this.editSizeDefault = function(){
		return {
			w: this.elements.containerEdit.outerWidth(),
			h: this.elements.containerEdit.outerHeight()
		};
	};
	
	
	// redefinable methods
	
	/*
	==========================
	onEditMode and onViewMode
	==========================
	- these are called when the mode is being changed
	*/
	this.onEditMode = function(){
		//
	};
	this.onViewMode = function(){
		//
	};
	
	/*
	======================
	viewSize and editSize
	======================
	- calculate container size for the adjustSize method
	* they are likely to be redefined for each particular widget
	*/
	this.viewSize = function(){
		return this.viewSizeDefault();
	};
	this.editSize = function(){
		return this.editSizeDefault();
	};
	
	// constructor end
	
	// if the constructor was called with a parameter,
	// call create() automatically
	if( arguments.length > 0 ){
		this.create( containerID );
	}
	this.setData( "dw", this.elements.container.outerWidth( true ) - this.elements.container.width() );
	this.setData( "dh", this.elements.container.outerHeight( true ) - this.elements.container.height() );
	window.winstance = thisInstance;
}
