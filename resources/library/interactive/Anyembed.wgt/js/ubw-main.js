
function init(){
	var embed;
	var inputBox = $("<textarea type='text'></textarea>")
	.css({
		width: "350px",
		height: "58px",
		marginRight: "5px"
	});

	var submit = $("<div>Embed</div>")
	.css({	
		float: "right",
		width: "auto",
		height:"auto",
		padding:"4px",
		marginTop:"13px",
		cursor:"pointer",
		backgroundColor:"rgb(45,65,250)",
		border:"1px solid",
		borderColor:"rgb(45,65,250) rgb(25,45,230) rgb(25,45,230) rgb(45,65,250)",
		textTransform:"capitalize",
		fontFamily:"Arial",
		fontSize:"18px",
		fontWeight:"regular",
		color:"white"
	});
	
	var loadWindow = $("<div></div>");
	
	var searchWrap = $("<div></div>")
	.css({
		position:"absolute",
		padding: "10px",
		backgroundColor: "rgb(230,230,233)",
		width: "425px",
		height: "65px"
	});
	
	//FIT WIDGET FRAME ON LAUNCH...
	if(window.uniboard){
		window.uniboard.resize($("#ubwidget").width()-15,85);
	}
	
	searchWrap.append(inputBox)
			  .append(submit);
			
	$("#ubwidget").append(searchWrap);	

	submit.click(function(){
		$("#ubwidget").append(loadWindow)
		embed = inputBox.val();
		searchWrap.hide();
		loadWindow.append(embed);
		
				if(window.uniboard){
					window.uniboard.resize($(document).width(),$(document).height());
					window.uniboard.setPreference("embed", escape(embed));
				}
		
	});
	
	if(window.uniboard){

		if(window.uniboard.preferenceKeys().length != 0){

				var loadEmbed = unescape(window.uniboard.preference("embed"));
				inputBox.val(loadEmbed);
				submit.trigger("click");	
		}	
	
	}

}