/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

function init(){
    var embed;
    var lang = "";
    
    if(window.sankore){
        try{
            lang = sankore.locale().substr(0,2);
            sankoreLang[lang].embed;
        } catch(e){
            lang = "en";
        }
    } else 
        lang = "en";
    
    var inputBox = $("<textarea type='text'></textarea>")
    .css({
        width: "350px",
        height: "58px",
        marginRight: "5px"
    });

    var submit = $("<div>" + sankoreLang[lang].embed + "</div>")
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
        width: "460px",
        height: "65px"
    });
	
    //FIT WIDGET FRAME ON LAUNCH...
    if(window.sankore){
        window.sankore.resize($("#ubwidget").width() + 20,85);
    }
	
    searchWrap.append(inputBox)
    .append(submit);
			
    $("#ubwidget").append(searchWrap);	

    submit.click(function(){
        embed = inputBox.val();
        if(embed){
            $("#ubwidget").append(loadWindow);        
            searchWrap.hide();

            if(/<object/.test(embed)){
                loadWindow.append(embed);
                loadWindow.find("embed").attr("wmode","opaque");
                var loadWindow_content = loadWindow.html();
                loadWindow.empty().html(loadWindow_content);
            }
            else {
                loadWindow.append(embed);				
            }

            if(window.sankore){
                window.sankore.resize($(document).width(),$(document).height());
                window.sankore.setPreference("embed", escape(embed));
            }
        } else{
            var sleepTime = 100;
            var counter = 0;
            var id = setInterval(function(){
                inputBox.toggleClass("bordered");
                counter ++;
                if(counter == 10)
                    clearInterval(id);
            }, sleepTime);
        }		
    });
	
    if(window.sankore){

        if(window.sankore.preferenceKeys().length != 0){

            var loadEmbed = unescape(window.sankore.preference("embed"));
            inputBox.val(loadEmbed);
            submit.trigger("click");	
        }	
    }
    
    if (window.widget) {
        window.widget.onremove = function(){
            loadWindow.empty();
        }
    }

}