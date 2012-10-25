var just_a_flag = false;

function onTemplateLoadedCallback() {

}

function revert($e) {
    var elmt = $e.css("left","auto").css("top", "auto").removeClass("inBox").get(0);
    if(elmt)
        elmt.className = elmt.className.replace(/(d(\d+))/, "");
}

function getN(range) {
    if(range > 1)
        return Math.floor(Math.random()*range);
    else if(range == 1)
        return Math.round(Math.random()*10)/10; 
    else if(range == 0.1)
        return Math.round(Math.random()*0.1*100)/100; 
}

function reloadApp(app) {
    var operator = app.parameters.value("operator");
    var range = parseFloat(app.parameters.value("range"));
    var count = parseInt(app.parameters.value("count"));
    var show = app.parameters.value("show");
    if(!just_a_flag){
        just_a_flag = true;
        show = "3";
    }
    switch(show){
        case "1":
            $("#labels div div").each(function(){
                $(this).css("top","auto").css("left","auto")
            })
            break;
        case "2":
            break;
        default:
            $scene = $("#scene");
            $scene.empty();

            $labels = $("<div id='labels'></div>");
            $scene.append($labels);

            $drops = $("<div id='drops'></div>");
            $scene.append($drops);

            var meeted = [];

            for (var i = 0; i < count; i++) {

                var n = getN(range);
		
                while($.inArray(n, meeted) != -1)
                    n = getN(range);

                meeted.push(n);
	
                $label = $("<div id='l"+i+"' class='label'>"+n+"</div>");
                $labels.append($label);
                $label.wrap("<div/>");

                $label.draggable({
                    containment: "#scene",
                    scroll: false,
                    snap: ".drop",
                    snapMode: "inner"
                });

                $drop = $("<div id='d"+i+"' class='drop'></div>");
                $drops.append($drop);
                $drop.droppable({
                    hoverClass: "ui-state-hover",
                    activeClass: "ui-state-active",  
                    greedy: true,
                    drop: function(event, ui) {
                        var id = $(this).attr("id");
                        var $current = $(".inBox."+id);
                        if($current.size() > 0 && $current.attr("id") != ui.draggable.attr("id"))
                            revert($current);
                        ui.draggable.get(0).className = ui.draggable.get(0).className.replace(/(d(\d+))/, "");
                        ui.draggable.addClass("inBox").addClass(id);
                    }
                });
                if(i != count-1) {
                    $("<div class='operator'>"+operator+"</div>").appendTo($drops);
                }
            }

            $scene.droppable({
                drop: function(event, ui) {
                    revert(ui.draggable);
                }
            });
            break;
    }
	
    
}

function reloadCallback(parameter) {
    if(parameter === undefined)
        reloadApp(this);
}

$(document).ready(function(){
    var callbacks = {
        onTemplatesLoaded: onTemplateLoadedCallback,
        onEdit: reloadApp,
        onView: reloadApp
    };
    init(reloadCallback, {
        toolbar: toolbarTemplate, 
        parameters: parametersTemplate
    }, callbacks);
});