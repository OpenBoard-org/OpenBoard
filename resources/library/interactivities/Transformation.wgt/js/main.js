function onTemplateLoadedCallback() {

}

function editRow(app, index) {
    var parameters = app.parameters;
    var row = $("<div/>");
    row.append("<div class='label'><div>"+fr.njin.i18n.transformation.row.label(index+1)+"</div></div>");
    var cards = $("<div class='cards'/>");
    row.append(cards);
	
    var beforecontent = parameters.value("#"+index+"before");
    var aftercontent = parameters.value("#"+index+"after");
	
    var before = $("<div rel='before'><div class='switch'><label>"+fr.njin.i18n.transformation.label.usePicture+"<input type='checkbox' name='switch'></label></div><div class='card'><div class='text'><div>"+( beforecontent !== undefined ? beforecontent : "" )+"</div></div><div class='picture'><div></div><div class='dropzone'><div>"+fr.njin.i18n.transformation.label.drop+"</div></div></div></div><div class='label'>"+fr.njin.i18n.transformation.label.before+"</div></div>");
    var after = $("<div rel='after'><div class='switch'><label>"+fr.njin.i18n.transformation.label.usePicture+"<input type='checkbox' name='switch'></label></div><div class='card'><div class='text'><div>"+( aftercontent !== undefined ? aftercontent : "" )+"</div></div><div class='picture'><div></div><div class='dropzone'><div>"+fr.njin.i18n.transformation.label.drop+"</div></div></div></div><div class='label'>"+fr.njin.i18n.transformation.label.after+"</div></div>");
	
    makeEditable(app, before, index);
    makeEditable(app, after, index);
	
    cards.append(before);
    cards.append(after);
	
    return row;
}

function cards(app, index) {
    var parameters = app.parameters;
    var beforecard = $("<div class='part before closed'/>");
    var aftercard = $("<div class='part after'/>");
	
    var beforecontent = parameters.value("#"+index+"before");
    var aftercontent = parameters.value("#"+index+"after");

    /*var sample1 = "", sample2 = "";
    
    if(!window.sankore || !window.sankore.preference("trans","")){
        if(index == 1){
            sample1 = "<img src='images/coq.png' class='uploadPic' title='coq.png' alt='coq.png'/>";
            sample2 = "<img src='images/poule.png' class='uploadPic' title='poule.png' alt='poule.png'/>";
        } else {
            sample1 = "<img src='images/boeuf.png' class='uploadPic' title='boeuf.png' alt='boeuf.png'/>";
            sample2 = "<img src='images/vache.png' class='uploadPic' title='vache.png' alt='vache.png'/>";
        }
    }*/
        
    var before = $("<div rel='before'><div class='card'><div class='text'><div>"+( beforecontent !== undefined ? beforecontent : "" )+"</div></div><div class='picture'><div></div></div></div></div>");
    var after = $("<div rel='after'><div class='card'><div class='text'><div>"+( aftercontent !== undefined ? aftercontent : "" )+"</div></div><div class='picture'><div></div></div></div></div>");
	
    var usePicture = parameters.value("#UsePicture"+index+"before") === "true"
    || parameters.value("#UsePicture"+index+"before") == true;
    
    //if(!window.sankore || !window.sankore.preference("trans",""))
        //usePicture = true;
    
    if(usePicture) {
        before.addClass("usePicture");	
        var f = $.parseJSON(parameters.value("#Picture"+index+"before"));
        if(f !== null) {
            var $img = $('<img src="" class="uploadPic" title="" alt="" />').attr(f);
            before.find(".picture>div:eq(0)").append($img);	
        }
    }
	
    usePicture = parameters.value("#UsePicture"+index+"after") === "true"
    || parameters.value("#UsePicture"+index+"after") == true;

    //if(!window.sankore || !window.sankore.preference("trans",""))
        //usePicture = true;
	
    if(usePicture) {
        after.addClass("usePicture");
        var f = $.parseJSON(parameters.value("#Picture"+index+"after"));
        if(f !== null) {
            var $img = $('<img src="" class="uploadPic" title="" alt="" />').attr(f);
            after.find(".picture>div:eq(0)").append($img);
        }
    }
	
    beforecard.append(before);
    aftercard.append(after);
	
    return [beforecard, aftercard];
}

function makeEditable(app, row, index) {
    var tmp_flag = false;
    var parameters = app.parameters;
    var editable = row.find(".text>div").eq(0);
    var key = index+editable.parent().parent().parent().attr('rel');	
    editable.get(0).contentEditable = true;
    editable.bind('blur keyup paste', function(){
        parameters.value("#"+key, $(this).html());
    });
	
    var checkbox = row.find("input[name='switch']");
	
    function setSwicth() {
        var val = parameters.value("#UsePicture"+key) === "true"
        || parameters.value("#UsePicture"+key) === true;        
        if(val) {
            row.addClass("usePicture");
        }else {
            row.removeClass("usePicture");
        }
        checkbox.attr("checked", val);
    }
	
    checkbox.change(function() {
        parameters.value("#UsePicture"+key, $(this).is(':checked'));
        setSwicth();
    });
    setSwicth();
	
    var dropzone = row.find(".dropzone");
    var pictureHolder = dropzone.parent().find(">div:eq(0)");
					
    function setPicture() {
        var f = $.parseJSON(parameters.value("#Picture"+key));
        if(f !== null) {
            var $img = $('<img src="" class="uploadPic" title="" alt="" />').attr(f);
            pictureHolder.empty();
            pictureHolder.append($img);
        }
    }
					
    app.utils.droppable(dropzone, function(f) {
        parameters.value("#Picture"+key, JSON.stringify(f));
        setPicture();
    });
    setPicture();
}

function reloadApp(app) {
    var number = app.parameters.value("number");
	
    var scene = $("#scene");
    scene.empty();
	
    if(app.onEdit) {
        var editcards = $("#edit-cards");
        editcards.empty();
        for(var i=0 ; i<number ; i++) {
            editcards.append(editRow(app, i));
        }
        if(window.sankore)
            window.sankore.enableDropOnWidget(app.onEdit);
        return;
    }
	
    var table = $("<div id='table'/>");
    scene.append(table);
	
    var before = $("<div class='deck before'><div></div></div>");
    var box = $("<div class='box'><div></div></div>");
    var after = $("<div class='deck after'><div></div></div>");
	
    if(number === 0)
        before.addClass("empty");
    after.addClass("empty");
	
    table.append(before);
    table.append(box);
    table.append(after);
	
    box.find(">div").css("z-index", number+1);				
	
    var beforeDeck = before.find(">div");
    var afterDeck = after.find(">div");
    var beforeLeftPosition = box.find(">div").position().left - beforeDeck.position().left + 10 + 40; //10px de padding dans la box + 40px d'ombre dans l'image
	
    function makeCard(i, cards) {
        beforeDeck.append(cards[0]);
        afterDeck.append(cards[1]);
        cards[1].css("left", -beforeLeftPosition+"px");				
        cards[1].css("z-index", i);				
		
        cards[0].click(function(e) {
            var c = $(this);
            c.removeClass("closed");
            c.unbind(e);
            setTimeout(function(){
                c.animate({
                    left : beforeLeftPosition
                }, 1000, function(){			
                    setTimeout(function(){
                        cards[1].animate({
                            left: 0
                        }, 1000, function(){
                            after.removeClass("empty");
                        });
                    }, 1000);
                    c.remove();	
                });
                if(i === 0) {
                    before.addClass("empty");
                }
            }, 3000);
			
        });
    }
		
    for(var i=number ; i>0 ; i--) {
        makeCard(i-1, cards(app, i-1));
    }
	
    if(window.sankore)
        window.sankore.enableDropOnWidget(app.onEdit);
}

function reloadCallback(parameter) {
    if(parameter === undefined || parameter.key === "number" )
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
    
    if (window.widget) {
        window.widget.onleave = function(){
            sankore.setPreference("trans", "true");
        }
    }
});