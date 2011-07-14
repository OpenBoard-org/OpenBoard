// sankore = {
	// preference : function(){},
	// setPreference : function(){}
// }

$(document).ready(function()
{
	var tinyContainer;
	var w = new wcontainer( "#ub-widget" );
	var _super_modeView = w.modeView;
	var _super_modeEdit = w.modeEdit;
	var _super_modeSplash = w.modeSplash;
	
	function _init()
	{
		w.setSplashContent( '<img src="custom_icon.png" alt="Click">' );
		w.setEditContent('<textarea style="width: 100%; height: 100%;">'+(window.sankore.preference("text") || 'Type a note here')+'</textarea>');
		w.elements.containerEdit.find( "textarea" ).tinymce(
		{
			script_url : 'tinymcejq/tiny_mce.js',
			width: w.getWidth(), 
			height: w.getHeight(),
			plugins : "table,advhr,advimage,inlinepopups,media,searchreplace,paste,advlist",

			theme : "advanced",
			theme_advanced_buttons1 : "formatselect,fontselect,fontsizeselect,|,forecolor,backcolor",
			theme_advanced_buttons2 : "bold,italic,underline,|,bullist,numlist,|,outdent,indent,|,justifyleft,justifycenter,justifyright,justifyfull,|,link,unlink",
			theme_advanced_buttons3 : "",
			theme_advanced_toolbar_location : "top",
			theme_advanced_toolbar_align : "left",
			theme_advanced_statusbar_location : "bottom",
			theme_advanced_resizing : false,

			// Drop lists for link/image/media/template dialogs
			template_external_list_url : "lists/template_list.js",
			external_link_list_url : "lists/link_list.js",
			external_image_list_url : "lists/image_list.js",
			media_external_list_url : "lists/media_list.js",
			
			setup : function(ed) 
			{
				ed.onKeyUp.add(function(ed, e) 
				{
					if (window.sankore)
					{
						window.sankore.setPreference("text", ed.getContent());
					}
				});
				ed.onExecCommand.add(function(ed, e)
				{
					if (window.sankore)
					{
						window.sankore.setPreference("text", ed.getContent());
					}
				});				
			},

			// Replace values for the template plugin
			template_replace_values : {
				username : "Some User",
				staffid : "991234"
			}
		});
		w.modeView(); // init view mode
		w.modeEdit(); // init edit mode
		if (window.sankore.preference("state") == 'view') // back to view mode if last state was it
			w.modeView();
		if (window.sankore.preference("is_splash") == '1')
			w.modeSplash(true);
		w.allowResize = true;
	};
	
	w.onViewMode = function()
	{
		var html = this.elements.containerEdit.find( "textarea" ).val();
		this.setViewContent( html );
	};
	
	w.getSize = function ()
	{
		return {w: this.getWidth(), h:this.getHeight()};
	};
		
	w.getWidth = function()
	{
		var res = 360;
		if (window.sankore && window.sankore.preference("width"))
			res = parseInt(window.sankore.preference("width"));
		return res;
	};
	w.getHeight = function()
	{
		var res = 230;
		if (window.sankore && window.sankore.preference("height"))
			res = parseInt(window.sankore.preference("height"));
		return res;
	};
	
	w.viewSize = function()
	{
		return this.getSize();
	};
	w.editSize = function()
	{
		return this.getSize();
	};
	
	window.onresize = function(){
		if (!w.allowResize)
			return;
		winwidth = window.innerWidth - 46;
		winheight = window.innerHeight - 45;
		if(winwidth <= w.minWidth)
			window.resizeTo(w.minWidth,winheight);
		if(winwidth > w.maxWidth)
			window.resizeTo(w.maxWidth,winheight);
		if(winheight <= w.minHeight)
		  	window.resizeTo(winwidth,w.minHeight);
		if(winheight > w.maxHeight)
			window.resizeTo(winwidth,w.maxHeight);
		w.elements.container.width(winwidth);
		w.elements.container.height(winheight);
		tinyMCE.activeEditor.theme.resizeTo(winwidth, winheight-98);
		if(window.sankore)
		{
			window.sankore.setPreference("width", winwidth);
			window.sankore.setPreference("height", winheight-33);
		}
	};
	
	w.modeView = function()
	{
		if (w.allowResize)
			window.sankore.setPreference("state", "view");
		return _super_modeView.call(this);
	}
	
	w.modeEdit = function()
	{
		if (w.allowResize)
			window.sankore.setPreference("state", "edit");
		return _super_modeEdit.call(this);
	}
	
	w.modeSplash = function (enable)
	{
		if (enable == undefined)
			enable = true;
		window.sankore.setPreference("is_splash", (w.allowResize && enable)?1:0);
		return _super_modeSplash.call(this, enable);
	}

	_init();
});