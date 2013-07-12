var sankoreLang = {
    view: "Display", 
    edit: "Edit",
    example: "this is a bunch of words which should be split apart",
    wgt_name: "Split a sentence",
    reload: "Reload",
    slate: "slate",
    pad: "pad",
    none: "none",
    help: "Help",
    help_content: 
"<p><h2>Split a sentence</h2></p>" +
"<p><h3> Separate words in a sentence</h3></p>" +
"<p>The sentence is written without separations between words. The goal is to insert spaces in right places. If the result is correct, the area turns in green.</p> "+

"<p>To add separations between words, move the cursor and click between two characters, a separation is then added.</p>" +
"<p>“Reload” button resets the exercise.</p>" +


"<p>Enter the “Edit” mode to : </p>" +
"<ul><li> choose the theme of the App : pad, slate, or none (by default : none),</li>" +
"<li>modify the sentence (select the text field).</li></ul>" +
"<p>“Display” button comes back to the activity.</p>",
    theme: "Theme"
};

// if use the "view/edit" button or rely on the api instead
var isSankore = false;
// whether to do window.resize or not (window = widget area)
var isBrowser = ( typeof( widget ) == "undefined" );

function wcontainer( containerID )
{
	// some protecred variables
	var thisInstance = this;
	this.editMode = false;
	var data = {}; // see setData and getData
	
	// widget size parameters
	this.minHeight = 100;
	this.minWidth = 400;
	
	// set to 0 for no max width restriction
	this.maxWidth = 0;
	
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
		this.elements.containerView = this.elements.subcontainer.find( ".viewmode" );
		this.elements.containerEdit = this.elements.subcontainer.find( ".editmode" );
		
		$("#wgt_edit").live("click", function(){
			thisInstance.modeEdit();
		} );
		
		$("#wgt_display").live("click", function(){
			thisInstance.modeView();
		} );
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
		this.editMode = true;
		this.elements.edit.removeClass( "hide" );
		this.elements.view.addClass( "hide" );
		
		//this.adjustSize();
	};
	this.modeView = function()
	{
		this.onViewMode();
		this.editMode = false;
		this.elements.edit.addClass( "hide" );
		this.elements.view.removeClass( "hide" );
		
		//this.adjustSize();
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
		return {
			w: this.elements.containerView.outerWidth(),
			h: this.elements.containerView.outerHeight()
		};
	};
	this.editSize = function(){
		return {
			w: this.elements.containerEdit.outerWidth(),
			h: this.elements.containerEdit.outerHeight()
		};
	};
	
	/*
	=====================
	checkAnswer
	=====================
	- check if the exercise in the view mode was done right
	* redefine it for each particular widget
	*/
	this.checkAnswer = function()
	{
		//
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
