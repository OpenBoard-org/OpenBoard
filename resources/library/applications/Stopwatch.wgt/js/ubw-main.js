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
	
    var tempHours;
    var tempMinutes;
    var tempSeconds;
	
    if(window.sankore){
        tempHours = window.sankore.preference("hours","00");
        tempMinutes = window.sankore.preference("minutes","00");
        tempSeconds = window.sankore.preference("seconds","00");
    } else {
        tempHours = "00";
        tempMinutes = "00";
        tempSeconds = "00";
    }
	
    var ubwidget = $("#ubwidget").ubwidget({
        width:252,
        height:89
    });
		
    var space = $("<div style='font-size:15px'>h</div>").ubwbutton({
        w:17, 
        h:68
    });
    space.find(".ubw-button-body").css({
        borderLeft:"none",
        borderRight:"none"
    });
    var spaceb = $("<div style='font-size:15px'>m</div>").ubwbutton({
        w:17, 
        h:68
    });
    spaceb.find(".ubw-button-body").css({
        borderLeft:"none",
        borderRight:"none"
    });
    spaceb.find(".ubw-button-canvas").unbind("mouseenter");
    var spacec = $("<div style='font-size:15px'>s</div>").ubwbutton({
        w:17, 
        h:68
    });
    spacec.find(".ubw-button-body").css({
        borderLeft:"none",
        borderRight:"none"
    });
    spacec.find(".ubw-button-canvas").unbind("mouseenter");
	
    var hours = $("<div class='scroll' id='hours'>" + tempHours + "</div>").ubwbutton({
        w:52, 
        h:68
    }, {
        bottom:1, 
        top:1, 
        right:0, 
        left:0
    });
    hours.find(".ubw-button-body").css({
        borderRight:"none"
    });
    var minutes = $("<div class='scroll' id='minutes'>" + tempMinutes + "</div>").ubwbutton({
        w:52, 
        h:68
    }, {
        bottom:1, 
        top:1, 
        right:0, 
        left:0
    });
    minutes.find(".ubw-button-body").css({
        borderLeft:"none",
        borderRight:"none"
    });
    var seconds = $("<div class='scroll' id='seconds'>" + tempSeconds + "</div>").ubwbutton({
        w:52, 
        h:68
    }, {
        bottom:1, 
        top:1, 
        right:0, 
        left:0
    });
	
    seconds.find(".ubw-button-body").css({
        borderLeft:"none",
        borderRight:"none"
    });
    var pause = $("<div id='pausebutton'><img src='images/button_pause_invert.png'></div>")
    pause.css({
        marginLeft:8, 
        marginTop:3,
        marginBottom:-2,
        height:47
    });
				
    var reset = $("<div id='resetbutton'><img src='images/button_reset_invert.png'></div>")
    .css({
        marginLeft:8, 
        marginTop:1
    })
    .click(function(){
        $("#hours").find(".ubw-button-content").text("00");
        $("#minutes").find(".ubw-button-content").text("00");
        $("#seconds").find(".ubw-button-content").text("00");
        clearTimeout(currentTimer);
        if(play){
            $("#pausebutton").trigger("click");
        }
    });
			
    if (window.widget) {
        window.widget.onremove = function(){
            if(play){
                $("#pausebutton").trigger("click");
            }
        };
    }
					
    pause.toggle(
        function(){
            play = false;
            clearTimeout(currentTimer);
            $(this).find("img").attr("src", "images/button_play_invert.png");
        },
        function(){
            if($("#hours").find(".ubw-button-content").text() != "00" || 
                $("#minutes").find(".ubw-button-content").text() != "00" ||
                $("#seconds").find(".ubw-button-content").text() != "00") {
                
                play = true;
                var timeInSeconds = parseInt($("#seconds").find(".ubw-button-content").text(), 10) +
                parseInt($("#minutes").find(".ubw-button-content").text(), 10)*60 + 
                parseInt($("#hours").find(".ubw-button-content").text(), 10)*3600;
		
                updateChronometerReverse(timeInSeconds);
					
                $(this).find("img").attr("src", "images/button_pause_invert.png");
            }
        }
        );/*.mouseenter(function(){
				var o = $(this).find("img").attr("src");
				$(this).find("img").attr("src", o.split(".")[0]+"xov.png");
			}).mouseout(function(){
				var o = $(this).find("img").attr("src");
				$(this).find("img").attr("src", o.split("x")[0]+".png");
			});*/
			
    setTimeout(function(){
        pause.trigger("click")
    }, 200);
			
    hours
    .bind("mousedown", {
        button:hours
    }, timeButtonDownHandler)
    .find(".ubw-button-arrowTop").bind("mousedown",{
        button:hours
    }, addbtn);
    hours
    .find(".ubw-button-arrowBottom").bind("mousedown",{
        button:hours
    }, rembtn);
    minutes
    .bind("mousedown", {
        button:minutes
    }, timeButtonDownHandler)
    .find(".ubw-button-arrowTop").bind("mousedown",{
        button:minutes
    }, addbtn);
    minutes
    .find(".ubw-button-arrowBottom").bind("mousedown",{
        button:minutes
    }, rembtn);
    seconds
    .bind("mousedown", {
        button:seconds
    }, timeButtonDownHandler)
    .find(".ubw-button-arrowTop").bind("mousedown",{
        button:seconds
    }, addbtn);
    seconds
    .find(".ubw-button-arrowBottom").bind("mousedown",{
        button:seconds
    }, rembtn);
		
    $(document).mouseup(function(){
        /*if(isScrolling){
					$().unbind("mousemove");
					activeTimeScroll.find(".ubw-button-canvas")
						.bind("mouseleave", {button:activeTimeScroll}, buttonOutHandler)
						.bind("mouseenter", {button:activeTimeScroll}, buttonOverHandler);
				};*/
        clearTimeout(incDecTime);
    });
			
    var btnsWrapper = $("<div></div>")
    .css({
        float:"left",
        marginLeft:5,
        marginTop:6
    })
    .append(pause)
    .append(reset);
			
    ubwidget
    .append(hours)
    .append(space.clone())
    .append(minutes)
    .append(spaceb)
    .append(seconds)
    .append(spacec)
    .append(btnsWrapper);
			
    clearTimeout(currentTimer);

    var currentTimer = null;
    var incDecTime = null;
    var play = true;
    var isScrolling = false;	
    var reverse = true;
    var activeTimeScroll = null;
				
    function addbtn(m){
	
        var button = m.data.button;
        var content = button.find(".ubw-button-content");
		
        if(content.text().substr(0, 1) == "0"){
            content.text(content.text().substr(1, content.text().length))
        }

        content.text(formatTime(parseInt(content.text())+1));	
		
        incDecTime = setTimeout(function(){
            addbtn(m)
        },150);
    }
	
    function rembtn(m){
	
        var button = m.data.button;
        var content = button.find(".ubw-button-content");
		
        if(content.text().substr(0, 1) == "0"){
            content.text(content.text().substr(1, content.text().length))
        }
		
        content.text(formatTime(parseInt(content.text())-1));
		
        incDecTime = setTimeout(function(){
            rembtn(m);
        },150);
    }	
				
    function timeButtonDownHandler(m){
        var button = m.data.button;
        var content = button.find(".ubw-button-content");
        if(window.sankore){
            var temp = button.attr("id");
            switch(temp){
                case "seconds":
                    window.sankore.setPreference("seconds", content.text());
                    break;
                case "minutes":
                    window.sankore.setPreference("minutes", content.text());
                    break;
                case "hours":
                    window.sankore.setPreference("hours", content.text());
                    break;
            }
        }
        /*var mouseStart = {
            pageX:m.pageX, 
            pageY:m.pageY
        };*/        
        var val = content.text();
        isScrolling = true;
        reverse = true;
        activeTimeScroll = button;
        if(play){
            $("#pausebutton").trigger("click");
        }
		
    /*button.find(".ubw-button-canvas")
			.unbind("mouseenter")
			.unbind("mouseleave");
			
		$().bind("mousemove", function(e){
			var value = {
				x:mouseStart.pageX-e.pageX, 
				y:mouseStart.pageY-e.pageY
			};
			hvalue = Math.floor(value.y/10);
			content.text(parseInt(val)+Math.floor(hvalue));
			
			content.text(content.text()%60);
			if(content.text() < 0)content.text(0);
		});*/
    }

    function buttonOverHandler(e) {
        var button = e.data.button;
        var buttonbody = button.find(".ubw-button-body");
								
        button.css({
            zIndex:1
        })
        buttonbody.removeClass("ubw-button-out")
        .addClass("ubw-button-over")
        .css({
            fontSize:"125%"
        });
			
        button.find(".ubw-button-canvas").find(".ubw-button-arrowTop").children("img").attr("src", "images/arrows_over/top.png");
        button.find(".ubw-button-canvas").find(".ubw-button-arrowBottom").children("img").attr("src", "images/arrows_over/bottom.png");	
    }

    function buttonOutHandler(e){
        var button = e.data.button;
        var buttonbody = button.find(".ubw-button-body");
				
        button.css({
            zIndex:0
        });
        buttonbody.removeClass("ubw-button-over")
        .addClass("ubw-button-out")					
        .css({
            fontSize:"100%"
        });
			
        button.find(".ubw-button-canvas").find(".ubw-button-arrowTop").children("img").attr("src", "images/arrows_out/top.png");
        button.find(".ubw-button-canvas").find(".ubw-button-arrowBottom").children("img").attr("src", "images/arrows_out/top.png");			
    }

    function updateChronometerReverse(seconds){
		
        currentTimer = setTimeout(function(){
            updateChronometerReverse(seconds-1)
        }, 1000);
        if (seconds < 6 && seconds > 0){
            DHTMLSound(1);
        }else if(seconds === 0){
            DHTMLSound(2);
        }
		
        if(seconds < 1){
            seconds = 0;
            $("#pausebutton").trigger("click");
        }
	
        var hoursValue = $("#hours").find(".ubw-button-content");
        var minutesValue = $("#minutes").find(".ubw-button-content");
        var secondsValue = $("#seconds").find(".ubw-button-content");
	
        var currentSecond = String(seconds%60);
        var currentMinute = String(Math.floor(seconds/60)%60);
        var currentHour = String(Math.floor(seconds/3600));
		
        hoursValue.text(formatTime(currentHour));
        minutesValue.text(formatTime(currentMinute));
        secondsValue.text(formatTime(currentSecond));
		
        if(window.sankore){
            window.sankore.setPreference("hours", hoursValue.text());
            window.sankore.setPreference("minutes", minutesValue.text());
            window.sankore.setPreference("seconds", secondsValue.text());
        }
		
    }	
	
    function formatTime(time){
		
        document.title = time +", " + String(time).length;
		
        if(time<0){
            time = 59;
        }
		
        time = time%60;
	
        String(time).length < 2 ? time = "0"+time : time = time;
	
        return time;		
    }
	
    function DHTMLSound(type) {
        var ad = document.getElementById("audio" + type);
        ad.play();
    }
}