// ----------------------------------------------------------------------------
// markItUp!
// ----------------------------------------------------------------------------
// Copyright (C) 2008 Jay Salvat
// http://markitup.jaysalvat.com/
// ----------------------------------------------------------------------------
// Html tags
// http://en.wikipedia.org/wiki/html
// ----------------------------------------------------------------------------
// Basic set. Feel free to add more tags
// ----------------------------------------------------------------------------

var lang = "";

if(window.sankore){
    try{
        lang = sankore.locale().substr(0,2);
        sankoreLang[lang].welcome;
    } catch(e){
        lang = "en";
    }
} else 
    lang = "en";

mySettings = {
	onShiftEnter:	{keepDefault:false, replaceWith:'<br />\n'},
	onCtrlEnter:	{keepDefault:false, openWith:'\n<p>', closeWith:'</p>\n'},
	onTab:			{keepDefault:false, openWith:'	 '},
	markupSet: [
		{name:'Heading 1', key:'1', openWith:'<h1(!( class="[![Class]!]")!)>', closeWith:'</h1>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Heading 2', key:'2', openWith:'<h2(!( class="[![Class]!]")!)>', closeWith:'</h2>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Heading 3', key:'3', openWith:'<h3(!( class="[![Class]!]")!)>', closeWith:'</h3>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Heading 4', key:'4', openWith:'<h4(!( class="[![Class]!]")!)>', closeWith:'</h4>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Heading 5', key:'5', openWith:'<h5(!( class="[![Class]!]")!)>', closeWith:'</h5>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Heading 6', key:'6', openWith:'<h6(!( class="[![Class]!]")!)>', closeWith:'</h6>', placeHolder:sankoreLang[lang].enter_title},
		{name:'Paragraph', openWith:'<p(!( class="[![Class]!]")!)>', closeWith:'</p>'},
		{separator:'---------------'},
		{name:'Bold', key:'B', openWith:'(!(<strong>|!|<b>)!)', closeWith:'(!(</strong>|!|</b>)!)'},
		{name:'Italic', key:'I', openWith:'(!(<em>|!|<i>)!)', closeWith:'(!(</em>|!|</i>)!)'},
		{name:'Stroke through', key:'S', openWith:'<del>', closeWith:'</del>'},
		{separator:'---------------'},
		{name:'Ul', openWith:'<ul>\n', closeWith:'</ul>\n'},
		{name:'Ol', openWith:'<ol>\n', closeWith:'</ol>\n'},
		{name:'Li', openWith:'<li>', closeWith:'</li>'},
		{separator:'---------------'},
		{name:'Picture', key:'P', replaceWith:'<img src="[![' + sankoreLang[lang].source + ':!:http://]!]" alt="[![' + sankoreLang[lang].alt_text + ']!]" />'},
		{name:'Link', key:'L', openWith:'<a target="_blank" href="[![' + sankoreLang[lang].link + ':!:http://]!]"(!( title="[![Title]!]")!)>', closeWith:'</a>', placeHolder:sankoreLang[lang].text_to_link},
		{separator:'---------------'},
		//{name:'Editor', className:'clean', replaceWith:function() { $('textarea').show(); $('iframe').remove();} },
		{name:'', className:'preview', call:'preview'}
	]
}