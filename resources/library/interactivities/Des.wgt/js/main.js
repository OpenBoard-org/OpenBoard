function onTemplateLoadedCallback(app) {
	$("#launch button").text(fr.njin.i18n.de.actions.label.launch);
	$("#launch button").click(function(){
		draw(parseInt(app.parameters.value("count")));
	});
}

function reloadApp(app) {
	draw(parseInt(app.parameters.value("count")));
}


function reloadCallback(parameter) {
	reloadApp(this);
}

$(document).ready(function(){

	var callbacks = {
		onTemplatesLoaded: onTemplateLoadedCallback,
		onEdit: reloadApp,
		onView: reloadApp
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);
});


/**
* Copyright :: http://www.ceriously.com/projects/dice/
*/
var c = "";
var debug = false;
var kill = false; //saftey feature to kill script

function drawDice() {
    if(kill) return;
    drawSquare(); //replaced old way so stroke is enabled

    //draws glare
    c.fillStyle = "rgba(200,200,200,0.4)";
    c.globalAlpha = 1.0;
    c.beginPath();
    //c.moveTo(-25,-25);
    //c.lineTo(-25,25);
    c.moveTo(-25,25);
    c.lineTo(25,25);
    c.bezierCurveTo(0, 15, -10, 15, -25, -25); //gives curve to glare
    c.closePath();
    c.fill();

    //draw dots
    var n = Math.floor(Math.random()*6)+1; //1-6 dots
    c.save();
    c.fillStyle = "black";
    if (n == 1) {
        //c.fillRect(-4,-4,8,8);
        drawDot(0,0);
    } else if (n == 2) {
        //c.fillRect(12, 12, 8, 8);
        //c.fillRect(-12, -12, -8, -8);
        drawDot(16,16);
        drawDot(-16,-16);
    } else if (n == 3) {
        //c.fillRect(12, 12, 8, 8);
        //c.fillRect(-12, -12, -8, -8);
        //c.fillRect(-4,-4,8,8);
        drawDot(0,0);
        drawDot(16,16);
        drawDot(-16,-16);
    } else if (n == 4) {
        //c.fillRect(12, 12, 8, 8);
        //c.fillRect(-12, -12, -8, -8);
        //c.fillRect(12,-12,8,-8);
        //c.fillRect(-12,12,-8,8);
        drawDot(16,16);
        drawDot(-16,-16);
        drawDot(-16,16);
        drawDot(16,-16);
    } else if (n == 5) {
        //c.fillRect(12, 12, 8, 8);
        //c.fillRect(-12, -12, -8, -8);
        //c.fillRect(12,-12,8,-8);
        //c.fillRect(-12,12,-8,8);
        //c.fillRect(-4,-4,8,8);
        drawDot(0,0);
        drawDot(16,16);
        drawDot(-16,-16);
        drawDot(-16,16);
        drawDot(16,-16);
    } else { //6
        //c.fillRect(12, 12, 8, 8);
        //c.fillRect(-12, -12, -8, -8);
        //c.fillRect(12,-12,8,-8);
        //c.fillRect(-12,12,-8,8);
        //c.fillRect(12,-3,8,8);
        //c.fillRect(-12,3,-8,-8);
        drawDot(16,16);
        drawDot(-16,-16);
        drawDot(-16,16);
        drawDot(16,-16);
        drawDot(16,0);
        drawDot(-16,0);
    }
    c.restore();

}

function drawDot(x,y) {
    c.beginPath();
    c.arc(x,y,5,0,Math.PI*2,true);
    c.closePath();
    c.fill();
}

function drawSquare() {
    c.strokeStyle = "#000000";
    c.fillStyle = "rgba(250,250,250,1.0)"; //white
    c.beginPath();
	var w = 30;
    c.moveTo(-w, -w);
    c.lineTo(w,-w);
    c.lineTo(w,w);
    c.lineTo(-w,w);
    c.lineTo(-w,-w);
    c.closePath();
    c.stroke();
    c.fill();
}

function exists(pos1,pos2, positions) {
    if(kill) return false;
    for (var j=0; j<positions.length; j++) {
        if (pos1+80 >= (positions[j])[0] && pos1-80 <= positions[j][0]) {
            if (pos2+80 >= positions[j][1] && pos2-80 <= positions[j][1]) {
				log("Oh No "+pos1+" | "+pos2+" is already used!");
                return true;
            }
        }
    }
    return false;
}

function draw(num) {
    kill = false;
    var canvas = $("#dices").get(0);
    c = canvas.getContext("2d");
    c.clearRect(0,0,canvas.width,canvas.height); //clears previous dice

    // all other translates are relative to this one
    c.translate(canvas.width / 2, canvas.height / 2);

    //declare variables
    var pos1 = 0;
    var pos2 = 0;
    var positions = new Array(num);

    //this loop creates 2dim array with num rows and 2 columns
    for (var i=0; i<num; i++) {
        positions[i] = new Array(2);
    }

    //this loop draws the num of dice
    for (var i=0; i<num; i++) {

        log("rolling: " + pos1 + " | " + pos2); //always starts at 0,0
        var counter = 0;

        while(exists(pos1,pos2,positions)) {
            pos1 = ( Math.floor(Math.random()*3) - 1 ) * ( Math.floor(Math.random()*(canvas.height / 4) -30) + 60 );
            pos2 = ( Math.floor(Math.random()*3) - 1 ) * ( Math.floor(Math.random()*(canvas.height / 4) -30) + 60 );
			log("rolling: " + pos1 + " | " + pos2);
            counter++;
            if (counter >= 500000) { //kills script if it takes too long
                alert("Are you trying to crash your browser!? Ceriously...");
                kill = true;
                break;
            }
        }
        if(kill) break;

		var r = (Math.random()*175) * Math.PI / 180;
		
        positions[i][0] = pos1;
        positions[i][1] = pos2;

        c.save();
        c.translate(pos1, pos2);
        c.rotate(r);
        drawDice();
        c.restore();
        //get new coordinates
        pos1 = ( Math.floor(Math.random()*3) - 1 ) * ( Math.floor(Math.random()*(canvas.height / 4) -30) + 60 );
        pos2 = ( Math.floor(Math.random()*3) - 1 ) * ( Math.floor(Math.random()*(canvas.height / 4) -30) + 60 );
    }
    c.translate(-canvas.width / 2, -canvas.height / 2); //moves translation back
} // end draw()