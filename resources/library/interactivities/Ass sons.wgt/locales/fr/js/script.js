var sankoreLang = {
    display: "Afficher", 
    edit: "Modifier", 
    short_desc: "Combien de signaux entendez-vous?", 
    add: "Nouveau bloc",
    enter: "Saisir votre description ici ...",
    wgt_name: "Associer aux sons",
    reload: "Recharger",
    slate: "ardoise",
    pad: "tablette",
    none: "aucun",
    help: "Aide",
    help_content: "<p><h2>Associer aux sons</h2></p>"+
    "<p><h3>Faire correspondre une image à un son.</h3></p>"+

    "<p>L’activité s’effectue par un glisser-déposer de l’image dans la zone délimitée. Si le résultat est incorrect, la zone se colore en rouge. Si le résultat est correct, la zone se colore en vert.</p>"+

    "<p>Le bouton “Recharger” réinitialise les exercices.</p>"+

    "<p>Le bouton “Modifier” vous permet : </p>"+
    "<ul><li>de choisir le thème de l’interactivité : tablette, ardoise ou aucun (par défaut aucun),</li>"+
    "<li>de modifier l'exercice.</li></ul>"+

"<p>En mode édition :</p>"+
"<ul><li>modifiez la consigne en cliquant sur le champ de texte,"+
"<li>remplacez le son dans la zone à gauche de la consigne par glisser-déposer d’un son à partir de votre bibliothèque,</li>"+
"<li>ajoutez des zones image(s) en cliquant sur le gros signe + en dessous,</li>"+
"<li>insérez des images par glisser-déposer des images à partir de votre bibliothèque,</li>"+
"<li>définissez l’image correcte de l’interactivité en cliquant sur le bouton valider “v” situé en bas à droite de l’image concernée,</li>"+
"<li>supprimez une zone image, en cliquant sur la croix située dans son coin supérieur droit,</li>"+ 
"<li>le bouton “Afficher” vous permet d’utiliser l’activité.</li></ul>",
    theme: "Thème"

};

//main function
function start(){

    $("#wgt_display").text(sankoreLang.display);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_name").text(sankoreLang.wgt_name);
    $("#wgt_reload").text(sankoreLang.reload);
    $("#wgt_help").text(sankoreLang.help);
    $("#help").html(sankoreLang.help_content);
    $("#style_select option[value='1']").text(sankoreLang.slate);
    $("#style_select option[value='2']").text(sankoreLang.pad);
    $("#style_select option[value='3']").text(sankoreLang.none);
    var tmpl = $("div.inline label").html();
    $("div.inline label").html(sankoreLang.theme + tmpl)
    
    if(window.sankore){
        if(sankore.preference("associer_sound","")){
            var data = jQuery.parseJSON(sankore.preference("associer_sound",""));
            importData(data);
        }
        else 
            showExample();
        if(sankore.preference("as_snd_style","")){
            changeStyle(sankore.preference("as_snd_style",""));
            $("#style_select").val(sankore.preference("as_snd_style",""));
        } else
            changeStyle("3")
    } 
    else 
        showExample();
    
    //events
    if (window.widget) {
        window.widget.onleave = function(){
            exportData();
            sankore.setPreference("as_snd_style", $("#style_select").find("option:selected").val());
        }
    }
    
    $("#wgt_help").click(function(){
        var tmp = $(this);
        if($(this).hasClass("open")){
            $(this).removeClass("help_pad").removeClass("help_wood")
            $("#help").slideUp("100", function(){
                tmp.removeClass("open");
                $("#data").show();
            });
        } else {
            ($("#style_select").val() == 1)?$(this).removeClass("help_pad").addClass("help_wood"):$(this).removeClass("help_wood").addClass("help_pad");
            $("#data").hide();
            $("#help").slideDown("100", function(){
                tmp.addClass("open");
            });
        }
    });
    
    $("#wgt_reload").click(function(){
        if($("#wgt_display").hasClass("selected")){
            $("#wgt_edit").trigger("click");
            $("#wgt_display").trigger("click");
        } else {
            $("#wgt_display").trigger("click");
        }
    });
    
    $("#wgt_reload, #wgt_display, #wgt_edit").mouseover(function(){
        exportData();
    });
    
    $("#style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
    
    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){
                if(window.sankore)
                    sankore.enableDropOnWidget(false);
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $("#parameters").css("display", "none");
//                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_i = 0;
                    var tmp_array = [];
                    
                    container.find(".text_cont .audio_desc").removeAttr("contenteditable");
                    container.find(".audio_block").removeAttr("ondragenter")
                    .removeAttr("ondragleave")
                    .removeAttr("ondragover")
                    .removeAttr("ondrop")
                    container.find(".add_img").remove();
//                    container.find(".close_cont").remove();
                    container.find(".img_block").each(function(){
                        if($(this).find("img").attr("src") != "img/drop_img.png"){
                            $(this).find(".close_img").remove();
                            $(this).find(".true_img").remove();
                            $(this).find(".false_img").remove();
                            $(this).removeAttr("ondragenter")
                            .removeAttr("ondragleave")
                            .removeAttr("ondragover")
                            .removeAttr("ondrop")
                            .addClass("img_gray");
                        } else 
                            $(this).remove();
                    });
                    var img_answers = $("<div class='imgs_answers imgs_answers_gray'><img src='img/drop_img.png' style='margin-top: 11px;'/></div>").insertAfter(container.find(".sub_cont"));
                    container.find(".img_block").each(function(){
                        $(this).css("float","");
                        tmp_array.push($(this));
                    });                    
                    tmp_array = shuffle(tmp_array);
                    for(var i = 0; i<tmp_array.length;i++){
                        tmp_array[i].draggable({
                            helper:'clone',
                            zIndex:100,
                            appendTo: '#data'
                        });
                        tmp_array[i].appendTo(container.find(".imgs_cont"));
                    }
                    
                    img_answers.droppable({
                        hoverClass: 'dropHere',
                        drop: function(event, ui) {
                            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                if($(this).children()[0].tagName == "IMG")
                                    $(this).children().remove();
                                else
                                    $(ui.draggable).parent().append($(this).children());
                                $(this).append($(ui.draggable));  
                                if($(this).children().length == 1){                
                                    if($(this).children().find("input").val() == "1")
                                        $(this).removeClass("imgs_answers_gray")
                                        .removeClass("imgs_answers_red")
                                        .addClass("imgs_answers_green");
                                    else
                                        $(this).removeClass("imgs_answers_gray")
                                        .removeClass("imgs_answers_green")
                                        .addClass("imgs_answers_red");
                                }                    
                            }
                        }
                    });
                    
                    container.find(".imgs_cont").droppable({
                        hoverClass: 'dropBack',
                        drop: function(event, ui) {
                            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                if(this != $(ui.draggable).parent()[0]){
                                    var tmp_cont = $(ui.draggable).parent();
                                    $(this).append($(ui.draggable));
                                    tmp_cont.append("<img src='img/drop_img.png' style='margin-top: 11px;'/>");
                                    var answers = "";
                                    $(this).parent().find(".imgs_answers .img_block").each(function(){
                                        answers += $(this).find("input").val();
                                    });
                                    if(tmp_cont.children()[0].tagName == "IMG")
                                        tmp_cont.removeClass("imgs_answers_green")
                                        .removeClass("imgs_answers_red")
                                        .addClass("imgs_answers_gray");
                                    else
                                    if(tmp_cont.children().length == 1){                
                                        if(tmp_cont.children().find("input").val() == "1")
                                            tmp_cont.removeClass("imgs_answers_gray")
                                            .removeClass("imgs_answers_red")
                                            .addClass("imgs_answers_green");
                                        else
                                            tmp_cont.removeClass("imgs_answers_gray")
                                            .removeClass("imgs_answers_green")
                                            .addClass("imgs_answers_red");
                                    }    
                                }
                            }
                        }
                    });
                });
                $(this).css("display", "none");
                $("#wgt_edit").css("display", "block");
            }
        } else {            
            if(!$(this).hasClass("selected")){
                if(window.sankore)
                    sankore.enableDropOnWidget(true);
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $("#parameters").css("display", "block");
                
                $(".cont").each(function(){
                    var container = $(this);
                    container.find(".imgs_answers").find(".img_block").each(function(){
                        $(this).appendTo(container.find(".imgs_cont"))
                    });
                    container.find(".imgs_answers").remove();
//                    $("<div class='close_cont'>").appendTo(container);
                    container.find(".imgs_cont").css("background-color", "");
                    container.find(".text_cont .audio_desc").attr("contenteditable","true");
                    container.find(".audio_block").attr("ondragenter", "return false;")
                    .attr("ondragleave", "$(this).removeClass('audio_gray'); return false;")
                    .attr("ondragover", "$(this).addClass('audio_gray'); return false;")
                    .attr("ondrop", "$(this).removeClass('audio_gray'); return onDropAudio(this,event);");
                    var add_img = $("<div class='add_img'>");
                    container.find(".img_block").each(function(){
                        $(this).draggable("destroy");
                        $(this).attr("ondragenter", "return false;")
                        .attr("ondragleave", "$(this).css(\"background-color\",\"\"); return false;")
                        .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
                        .attr("ondrop", "$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);")
                        .css("float","left");
                        $("<div class='close_img'>").appendTo($(this));
                        if($(this).find("input").val() == "1"){
                            $("<div class='false_img'>").appendTo($(this));
                        }
                        else{
                            $("<div class='true_img'>").appendTo($(this));
                        }
                    });
                    container.find(".imgs_cont").append(add_img)
                });                
                
//                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("#data");
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
            }
        }
        
        $("audio").each(function(){
            this.pause();
            $(this).parent().find(":first-child").removeClass("stop").addClass("play");
        });
        
    });
    
    //add new block
//    $(".add_block").live("click", function(){
//        addContainer();
//    });
    
    //adding new img
    $(".add_img").live("click", function(){
        addImgBlock($(this));
    });
    
    //deleting a block
//    $(".close_cont").live("click",function(){
//        $(this).parent().remove();
//        refreshBlockNumbers();
//    });
    
    //deleting the img block
    $(".close_img").live("click", function(){
        var i = 0;
        var tmp_obj = $(this).parent().parent();        
        $(this).parent().remove();        
    });
    
    //correct image
    $(".true_img").live("click",function(){        
        $(this).parent().parent().find(".img_block").each(function(){
            $(this).find("input").val(0);
            var tmp_div = $(this).find(".false_img");
            if(tmp_div)
                tmp_div.removeClass("false_img").addClass("true_img");
        })
        $(this).parent().find("input").val(1);
        $(this).removeClass("true_img").addClass("false_img");
    });
    
    //wrong image
    $(".false_img").live("click",function(){
        $(this).parent().find("input").val(0);
        $(this).removeClass("false_img").addClass("true_img");
    });
    
    //play/pause event
    $(".play, .stop").live("click", function(){
        var tmp_audio = $(this);
        var audio = tmp_audio.parent().find("audio").get(0);
        if($(this).hasClass("play")){            
            if(tmp_audio.parent().find("source").attr("src")){
                tmp_audio.removeClass("play").addClass("stop");
                var id = setInterval(function(){
                    if(audio.currentTime == audio.duration){
                        clearInterval(id);
                        tmp_audio.removeClass("stop").addClass("play");
                    }
                }, 10);
                tmp_audio.parent().find("input").val(id);
                audio.play();
            }
        } else {
            $(this).removeClass("stop").addClass("play");
            clearInterval( tmp_audio.parent().find("input").val())
            audio.pause();
        }
    });
    
    $(".replay").live("click", function(){
        var tmp_audio = $(this).prev();
        var audio = $(this).parent().find("audio").get(0); 
        if(tmp_audio.parent().find("source").attr("src")){
            $(this).prev().removeClass("play").addClass("stop");
            clearInterval($(this).parent().find("input").val());
            var id = setInterval(function(){
                if(audio.currentTime == audio.duration){
                    clearInterval(id);
                    tmp_audio.removeClass("stop").addClass("play");
                }
            }, 10);
            tmp_audio.parent().find("input").val(id);
            audio.currentTime = 0;
            audio.play();
        }
    });
}

//export
function exportData(){
    var array_to_export = [];
    if($("#wgt_edit").hasClass("selected")){
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.text = $(this).find(".audio_desc").text();
            cont_obj.audio = $(this).find("source").attr("src").replace("../../","");;
            cont_obj.mode = "edit";
            cont_obj.imgs = [];
            $(this).find(".img_block").each(function(){
                var img_obj = new Object();
                img_obj.value = $(this).find("input").val();
                img_obj.link = $(this).find("img").attr("src").replace("../../","");
                img_obj.ht = $(this).find("img").height();
                img_obj.wd = $(this).find("img").width();
                cont_obj.imgs.push(img_obj);
            });
            array_to_export.push(cont_obj);
        });
    } else {
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.text = $(this).find(".audio_desc").text();
            cont_obj.audio = $(this).find("source").attr("src").replace("../../","");
            cont_obj.mode = "display";
            cont_obj.imgs = [];
            if($(this).find(".imgs_answers").children()[0].tagName == "DIV"){
                var tmp_cont = $(this).find(".imgs_answers").find(".img_block");
                var img_obj = new Object();
                img_obj.value = tmp_cont.find("input").val();
                img_obj.link = tmp_cont.find("img").attr("src").replace("../../","");
                img_obj.ht = tmp_cont.find("img").height();
                img_obj.wd = tmp_cont.find("img").width();
                img_obj.cont = "answers";
                cont_obj.imgs.push(img_obj);
            }
            $(this).find(".imgs_cont .img_block").each(function(){
                var img_obj = new Object();
                img_obj.value = $(this).find("input").val();
                img_obj.link = $(this).find("img").attr("src").replace("../../","");
                img_obj.ht = $(this).find("img").height();
                img_obj.wd = $(this).find("img").width();
                img_obj.cont = "cont";
                cont_obj.imgs.push(img_obj);
            });
            array_to_export.push(cont_obj);
        });
    }
    if(window.sankore)
        sankore.setPreference("associer_sound", JSON.stringify(array_to_export));
}

//import
function importData(data){
       
    for(var i in data){
        if(data[i].mode == "edit"){          
            var tmp_array = [];
            var container = $("<div class='cont'>");
            var sub_container = $("<div class='sub_cont'>").appendTo(container);
            var imgs_answers = $("<div class='imgs_answers imgs_answers_gray'><img src='img/drop_img.png' style='margin-top: 11px;'/></div>").appendTo(container);
            var imgs_container = $("<div class='imgs_cont'>").appendTo(container);    
        
//            var number = $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
            var text = $("<div class='text_cont'>").appendTo(sub_container);
            var audio_block = $("<div class='audio_block'>").appendTo(text);
            $("<div class='play'>").appendTo(audio_block);
            $("<div class='replay'>").appendTo(audio_block);
            var source = $("<source/>").attr("src", "../../" + data[i].audio);
            var audio = $("<audio>").appendTo(audio_block);
            audio.append(source);
            $("<input type='hidden'/>").appendTo(audio_block);
            $("<div class='audio_desc'>" + data[i].text + "</div>").appendTo(text);
        
            for(var j in data[i].imgs){
                var img_block = $("<div class='img_block img_gray' style='text-align: center;'>");
                var img = $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;'>");
                img.height(data[i].imgs[j].ht);
                if((120 - data[i].imgs[j].ht) > 0)
                    img.css("margin",(120 - data[i].imgs[j].ht)/2 + "px 0");
                var hidden_input = $("<input type='hidden'>").val(data[i].imgs[j].value);
                img_block.append(hidden_input).append(img);
                tmp_array.push(img_block);
            }
            tmp_array = shuffle(tmp_array);
            for(j in tmp_array){
                tmp_array[j].draggable({
                    helper:'clone',
                    zIndex:100,
                    appendTo: '#data'
                });
                tmp_array[j].appendTo(imgs_container);
            }
            imgs_answers.droppable({
                hoverClass: 'dropHere',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if($(this).children()[0].tagName == "IMG")
                            $(this).children().remove();
                        else
                            $(ui.draggable).parent().append($(this).children());
                        $(this).append($(ui.draggable));  
                        if($(this).children().length == 1){                
                            if($(this).children().find("input").val() == "1")
                                $(this).removeClass("imgs_answers_gray")
                                .removeClass("imgs_answers_red")
                                .addClass("imgs_answers_green");
                            else
                                $(this).removeClass("imgs_answers_gray")
                                .removeClass("imgs_answers_green")
                                .addClass("imgs_answers_red");
                        }                    
                    }
                }
            });
            imgs_container.droppable({
                hoverClass: 'dropBack',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if(this != $(ui.draggable).parent()[0]){
                            var tmp_cont = $(ui.draggable).parent();
                            $(this).append($(ui.draggable));
                            tmp_cont.append("<img src='img/drop_img.png' style='margin-top: 11px;'/>");
                            var answers = "";
                            $(this).parent().find(".imgs_answers .img_block").each(function(){
                                answers += $(this).find("input").val();
                            });
                            if(tmp_cont.children()[0].tagName == "IMG")
                                tmp_cont.removeClass("imgs_answers_green")
                                .removeClass("imgs_answers_red")
                                .addClass("imgs_answers_gray");
                            else
                            if(tmp_cont.children().length == 1){                
                                if(tmp_cont.children().find("input").val() == "1")
                                    tmp_cont.removeClass("imgs_answers_gray")
                                    .removeClass("imgs_answers_red")
                                    .addClass("imgs_answers_green");
                                else
                                    tmp_cont.removeClass("imgs_answers_gray")
                                    .removeClass("imgs_answers_green")
                                    .addClass("imgs_answers_red");
                            }    
                        }
                    }
                }
            }); 
            container.appendTo("#data");
        } else {
            container = $("<div class='cont'>");
            sub_container = $("<div class='sub_cont'>").appendTo(container);
            imgs_answers = $("<div class='imgs_answers'></div>").appendTo(container);
            $("<img src='img/drop_img.png' style='margin-top: 11px;'/>").appendTo(imgs_answers);
            imgs_container = $("<div class='imgs_cont'>").appendTo(container);    
        
//            number = $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
            text = $("<div class='text_cont'>").appendTo(sub_container);
            audio_block = $("<div class='audio_block'>").appendTo(text);
            $("<div class='play'>").appendTo(audio_block);
            $("<div class='replay'>").appendTo(audio_block);
            source = $("<source/>").attr("src", "../../" + data[i].audio);
            audio = $("<audio>").appendTo(audio_block);
            audio.append(source);
            $("<input type='hidden'/>").appendTo(audio_block);
            $("<div class='audio_desc'>" + data[i].text + "</div>").appendTo(text);
        
            for(j in data[i].imgs){
                img_block = $("<div class='img_block img_gray' style='text-align: center;'>");
                img = $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;'>");
                img.height(data[i].imgs[j].ht);
                if((120 - data[i].imgs[j].ht) > 0)
                    img.css("margin",(120 - data[i].imgs[j].ht)/2 + "px 0");
                hidden_input = $("<input type='hidden'>").val(data[i].imgs[j].value);
                img_block.append(hidden_input).append(img);
                img_block.draggable({
                    helper:'clone',
                    zIndex:100,
                    appendTo: '#data'
                });
                if(data[i].imgs[j].cont == "cont")
                    img_block.appendTo(imgs_container);
                else{
                    imgs_answers.empty();
                    img_block.appendTo(imgs_answers);
                    if(data[i].imgs[j].value == "1")
                        imgs_answers.addClass("imgs_answers_green");
                    else
                        imgs_answers.addClass("imgs_answers_red");
                }
            }
            
            imgs_answers.droppable({
                hoverClass: 'dropHere',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if($(this).children()[0].tagName == "IMG")
                            $(this).children().remove();
                        else
                            $(ui.draggable).parent().append($(this).children());
                        $(this).append($(ui.draggable));  
                        if($(this).children().length == 1){                
                            if($(this).children().find("input").val() == "1")
                                $(this).removeClass("imgs_answers_gray")
                                .removeClass("imgs_answers_red")
                                .addClass("imgs_answers_green");
                            else
                                $(this).removeClass("imgs_answers_gray")
                                .removeClass("imgs_answers_green")
                                .addClass("imgs_answers_red");
                        }                    
                    }
                }
            });
            imgs_container.droppable({
                hoverClass: 'dropBack',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if(this != $(ui.draggable).parent()[0]){
                            var tmp_cont = $(ui.draggable).parent();
                            $(this).append($(ui.draggable));
                            tmp_cont.append("<img src='img/drop_img.png' style='margin-top: 11px;'/>");
                            var answers = "";
                            $(this).parent().find(".imgs_answers .img_block").each(function(){
                                answers += $(this).find("input").val();
                            });
                            if(tmp_cont.children()[0].tagName == "IMG")
                                tmp_cont.removeClass("imgs_answers_green")
                                .removeClass("imgs_answers_red")
                                .addClass("imgs_answers_gray");
                            else
                            if(tmp_cont.children().length == 1){                
                                if(tmp_cont.children().find("input").val() == "1")
                                    tmp_cont.removeClass("imgs_answers_gray")
                                    .removeClass("imgs_answers_red")
                                    .addClass("imgs_answers_green");
                                else
                                    tmp_cont.removeClass("imgs_answers_gray")
                                    .removeClass("imgs_answers_green")
                                    .addClass("imgs_answers_red");
                            }    
                        }
                    }
                }
            }); 
            container.appendTo("#data");
        }
    }
}

//example
function showExample(){
    
    var tmp_array = [];
    
    var container = $("<div class='cont'>").appendTo("#data");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_answers = $("<div class='imgs_answers imgs_answers_gray'><img src='img/drop_img.png' style='margin-top: 11px;'/></div>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);

//    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>").appendTo(sub_container);
    var audio_block = $("<div class='audio_block'>").appendTo(text);
    $("<div class='play'>").appendTo(audio_block);
    $("<div class='replay'>").appendTo(audio_block);
    var source = $("<source/>").attr("src", "../../objects/beep.wav");
    var audio = $("<audio>").appendTo(audio_block);
    audio.append(source);
    $("<input type='hidden'/>").appendTo(audio_block);
    var audio_desc = $("<div class='audio_desc'>" + sankoreLang.short_desc + "</div>").appendTo(text);
    
    $("<input type='hidden' value='1'/>").appendTo(imgs_container);
    
    var img1 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img1);
    $("<img src=\"../../objects/0.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img1);
    var img2 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(img2);
    $("<img src=\"../../objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img2);
    var img3 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img3);
    $("<img src=\"../../objects/2.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img3);
    var img4 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img4);
    $("<img src=\"../../objects/3.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img4);
    var img5 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img5);
    $("<img src=\"../../objects/4.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img5);  
    
    tmp_array.push(img1, img2, img3, img4, img5);
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++){
        tmp_array[i].draggable({
            helper:'clone',
            zIndex:100,
            appendTo: '#data'
        });
        tmp_array[i].appendTo(imgs_container);
    }
    imgs_answers.droppable({
        hoverClass: 'dropHere',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                if($(this).children()[0].tagName == "IMG")
                    $(this).children().remove();
                else
                    $(ui.draggable).parent().append($(this).children());
                $(this).append($(ui.draggable));  
                if($(this).children().length == 1){                
                    if($(this).children().find("input").val() == "1")
                        $(this).removeClass("imgs_answers_gray")
                        .removeClass("imgs_answers_red")
                        .addClass("imgs_answers_green");
                    else
                        $(this).removeClass("imgs_answers_gray")
                        .removeClass("imgs_answers_green")
                        .addClass("imgs_answers_red");
                }                    
            }
        }
    });
    imgs_container.droppable({
        hoverClass: 'dropBack',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                if(this != $(ui.draggable).parent()[0]){
                    var tmp_cont = $(ui.draggable).parent();
                    $(this).append($(ui.draggable));
                    tmp_cont.append("<img src='img/drop_img.png' style='margin-top: 11px;'/>");
                    var answers = "";
                    $(this).parent().find(".imgs_answers .img_block").each(function(){
                        answers += $(this).find("input").val();
                    });
                    if(tmp_cont.children()[0].tagName == "IMG")
                        tmp_cont.removeClass("imgs_answers_green")
                        .removeClass("imgs_answers_red")
                        .addClass("imgs_answers_gray");
                    else
                    if(tmp_cont.children().length == 1){                
                        if(tmp_cont.children().find("input").val() == "1")
                            tmp_cont.removeClass("imgs_answers_gray")
                            .removeClass("imgs_answers_red")
                            .addClass("imgs_answers_green");
                        else
                            tmp_cont.removeClass("imgs_answers_gray")
                            .removeClass("imgs_answers_green")
                            .addClass("imgs_answers_red");
                    }    
                }
            }
        }
    });
}

//check result
function checkResult(event)
{
    var str = "";
    var right_str = $(event.target).find("input").val();
    $(event.target).find(".img_block").each(function(){
        str += $(this).find("input").val() + "*";
    });
    if(str == right_str)
        $(event.target).css("background-color","#9f9");
}

//add new container
//function addContainer(){
//    var container = $("<div class='cont'>");
//    var sub_container = $("<div class='sub_cont'>").appendTo(container);
//    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);
//    
//    var close = $("<div class='close_cont'>").appendTo(container);
//    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
//    var text = $("<div class='text_cont'>").appendTo(sub_container);
//    var audio_block = $("<div class='audio_block'>").appendTo(text);
//    audio_block.attr("ondragenter", "return false;")
//    .attr("ondragleave", "$(this).removeClass('audio_gray'); return false;")
//    .attr("ondragover", "$(this).addClass('audio_gray'); return false;")
//    .attr("ondrop", "$(this).removeClass('audio_gray'); return onDropAudio(this,event);");    
//    $("<div class='play'>").appendTo(audio_block);
//    $("<div class='replay'>").appendTo(audio_block);
//    var source = $("<source/>").attr("src", "");
//    var audio = $("<audio>").appendTo(audio_block);
//    audio.append(source);
//    $("<input type='hidden'/>").appendTo(audio_block);
//    var audio_desc = $("<div class='audio_desc' contenteditable>" + sankoreLang.enter + "</div>").appendTo(text);
//    
//    $("<input type='hidden' value='1'/>").appendTo(imgs_container);
//    var add_img = $("<div class='add_img'>").appendTo(imgs_container);
//    container.insertBefore($(".add_block"));
//}

//add new img block
function addImgBlock(dest){
    var img_block = $("<div class='img_block img_gray' ondragenter='return false;' ondragleave='$(this).css(\"background-color\",\"\"); return false;' ondragover='$(this).css(\"background-color\",\"#ccc\"); return false;' ondrop='$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);' style='text-align: center; float: left;'></div>").insertBefore(dest);
    $("<div class='close_img'>").appendTo(img_block);
    $("<div class='true_img'>").appendTo(img_block);
    $("<input type='hidden' value='0'/>").appendTo(img_block);
    $("<img src='img/drop_img.png' height='120'/>").appendTo(img_block);
}

//function refreshBlockNumbers(){
//    var i = 0;
//    $(".cont").each(function(){
//        $(this).find(".number_cont").text(++i);
//    })
//}

//shuffles an array
function shuffle( arr )
{
    var pos, tmp;
	
    for( var i = 0; i < arr.length; i++ )
    {
        pos = Math.round( Math.random() * ( arr.length - 1 ) );
        tmp = arr[pos];
        arr[pos] = arr[i];
        arr[i] = tmp;
    }
    return arr;
}


function stringToXML(text){
    if (window.ActiveXObject){
        var doc=new ActiveXObject('Microsoft.XMLDOM');
        doc.async='false';
        doc.loadXML(text);
    } else {
        var parser=new DOMParser();
        doc=parser.parseFromString(text,'text/xml');
    }
    return doc;
}

//changing the style
function changeStyle(val){
    switch(val){
        case "1":
            $(".b_top_left").removeClass("btl_pad").removeClass("without_back");
            $(".b_top_center").removeClass("btc_pad").removeClass("without_back");
            $(".b_top_right").removeClass("btr_pad").removeClass("without_back");
            $(".b_center_left").removeClass("bcl_pad").removeClass("without_back");
            $(".b_center_right").removeClass("bcr_pad").removeClass("without_back");
            $(".b_bottom_right").removeClass("bbr_pad").removeClass("without_back");
            $(".b_bottom_left").removeClass("bbl_pad").removeClass("without_back");
            $(".b_bottom_center").removeClass("bbc_pad").removeClass("without_back");
            $("#wgt_reload").removeClass("pad_color").removeClass("pad_reload");
            $("#wgt_help").removeClass("pad_color").removeClass("pad_help");
            $("#wgt_edit").removeClass("pad_color").removeClass("pad_edit");
            $("#wgt_name").removeClass("pad_color");
            $("#wgt_display").addClass("display_wood");
            $("#style_select").val(val);
            $("body, html").removeClass("without_radius").addClass("radius_ft");
            break;
        case "2":
            $(".b_top_left").addClass("btl_pad").removeClass("without_back");
            $(".b_top_center").addClass("btc_pad").removeClass("without_back");
            $(".b_top_right").addClass("btr_pad").removeClass("without_back");
            $(".b_center_left").addClass("bcl_pad").removeClass("without_back");
            $(".b_center_right").addClass("bcr_pad").removeClass("without_back");
            $(".b_bottom_right").addClass("bbr_pad").removeClass("without_back");
            $(".b_bottom_left").addClass("bbl_pad").removeClass("without_back");
            $(".b_bottom_center").addClass("bbc_pad").removeClass("without_back");
            $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
            $("#wgt_help").addClass("pad_color").addClass("pad_help");
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_name").addClass("pad_color");
            $("#wgt_display").removeClass("display_wood");
            $("#style_select").val(val);
            $("body, html").removeClass("without_radius").removeClass("radius_ft");
            break;
        case "3":
            $(".b_top_left").addClass("without_back").removeClass("btl_pad");
            $(".b_top_center").addClass("without_back").removeClass("btc_pad");
            $(".b_top_right").addClass("without_back").removeClass("btr_pad");
            $(".b_center_left").addClass("without_back").removeClass("bcl_pad");
            $(".b_center_right").addClass("without_back").removeClass("bcr_pad");
            $(".b_bottom_right").addClass("without_back").removeClass("bbr_pad");
            $(".b_bottom_left").addClass("without_back").removeClass("bbl_pad");
            $(".b_bottom_center").addClass("without_back").removeClass("bbc_pad");
            $("#wgt_help").addClass("pad_color").addClass("pad_help");
            $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_name").addClass("pad_color");
            $("#wgt_display").removeClass("display_wood");
            $("#style_select").val(val);
            $("body, html").addClass("without_radius").removeClass("radius_ft");
            break;
    }
}

function onDropTarget(obj, event) {
    $(obj).find("img").remove();
    if (event.dataTransfer) {
        var format = "text/plain";
        var textData = event.dataTransfer.getData(format);
        if (!textData) {
            alert(":(");
        }
        textData = stringToXML(textData);
        var tmp = textData.getElementsByTagName("path")[0].firstChild.textContent;
        var tmp_img = $("<img/>").attr("src", "../../" + tmp);
        $(obj).append(tmp_img);
        setTimeout(function(){
            if(tmp_img.height() >= tmp_img.width())
                tmp_img.attr("height", "120");
            else{
                tmp_img.attr("width","120");
                var h = tmp_img.height();
                tmp_img.attr("height",h);
                tmp_img.css("margin",(120 - tmp_img.height())/2 + "px 0");
            }
        }, 6)
    }
    else {
        alert ("Your browser does not support the dataTransfer object.");
    }

    if (event.stopPropagation) {
        event.stopPropagation ();
    }
    else {
        event.cancelBubble = true;
    }
    return false;
}

function onDropAudio(obj, event) {
    if (event.dataTransfer) {
        var format = "text/plain";
        var textData = event.dataTransfer.getData(format);
        if (!textData) {
            alert(":(");
        }
        textData = stringToXML(textData);
        var tmp = textData.getElementsByTagName("path")[0].firstChild.textContent;
        var tmp_type = textData.getElementsByTagName("type")[0].firstChild.textContent;
        if(tmp_type.substr(0, 5) == "audio"){       
            var audio_block = $(obj);
            $(obj).find("audio").remove();
            audio_block.find(":first-child").removeClass("stop").addClass("play");
            var source = $("<source/>").attr("src", "../../" + tmp);
            var audio = $("<audio>").appendTo(audio_block);
            audio.append(source);
        }
    }
    else {
        alert ("Your browser does not support the dataTransfer object.");
    }

    if (event.stopPropagation) {
        event.stopPropagation ();
    }
    else {
        event.cancelBubble = true;
    }
    return false;
}

if (window.widget) {
    window.widget.onremove = function(){
        $("audio").each(function(){
            this.pause();
            $(this).parent().find(":first-child").removeClass("stop").addClass("play");
        });
    }
}
