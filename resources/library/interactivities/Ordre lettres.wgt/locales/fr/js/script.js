var sankoreLang = {
    display: "Afficher", 
    edit: "Modifier", 
    short_desc: "Ecoutez le son et replacez les lettres dans l'ordre.", 
    add: "Nouveau bloc",
    enter: "Saisir la consigne ici ...",
    example: "exemple",
    wgt_name: "Ordonner des lettres",
    reload: "Recharger",
    slate: "ardoise",
    pad: "tablette",
    none: "aucun",
    help: "Aide",
    help_content: "<p><h2>Ordonner des lettres</h2></p>"+
    "<p><h3>Ordonner des lettres pour reconstituer un mot.</h3></p>"+

    "<p>L’intégration d’un fichier son (facultatif) permet d’écouter le mot à reconstituer. Lorsque les lettres sont dans le bon ordre, l’interactivité se colore en vert.</p>"+

    "<p>L’activité s’effectue par un glisser-déposer des étiquettes pour remettre les lettres en ordre.<b> NB : vous pouvez également utiliser cette activité avec des chiffres.</b></p>"+

    "<p>Le bouton “Recharger” réinitialise les exercices.</p>"+

    "<p>Le bouton “Modifier” vous permet :</p>"+
    "<ul><li>de choisir le thème de l’interactivité : tablette, ardoise ou aucun (par défaut aucun),</li>"+
    "<li>de déterminer le mot à reconstituer et modifier l'exercice.</li></ul>"+
 
    "<p>Dans le mode édition :</p>"+
    "<ul><li>modifiez la consigne en cliquant sur le champ de texte,</li>"+
    "<li>insérez un son dans la zone à gauche de la consigne par glisser-déposer d’un son à partir de votre bibliothèque (facultatif),</li>"+
    "<li>écrivez le mot souhaité à la place de “exemple”,</li>"+
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
        if(sankore.preference("ord_let","")){
            var data = jQuery.parseJSON(sankore.preference("ord_let",""));
            importData(data);
        }
        else 
            showExample();
        if(sankore.preference("ord_let_style","")){
            changeStyle(sankore.preference("ord_let_style",""));
            $("#style_select").val(sankore.preference("ord_let_style",""));
        } else
            changeStyle("3")
    } 
    else 
        showExample();
    //events
    if (window.widget) {
        window.widget.onleave = function(){
            exportData();
            sankore.setPreference("ord_let_style", $("#style_select").find("option:selected").val());
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
            setTimeout('$("#wgt_display").trigger("click")', 10)
            $("#wgt_edit").trigger("click");            
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
                $("#parameters").css("display","none");
//                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_array = [];
                    var ans_container = container.find(".audio_answer");
                    
                    container.find(".text_cont .audio_desc").removeAttr("contenteditable");
                    container.find(".audio_block").removeAttr("ondragenter")
                    .removeAttr("ondragleave")
                    .removeAttr("ondragover")
                    .removeAttr("ondrop")
//                    container.find(".close_cont").remove();
                    var answer = ans_container.text();
                    ans_container.prev().val(answer)
                    ans_container.remove();
                    var ul_cont = $("<ul id='sortable' class='imgs_answers_gray'>").insertAfter(container.find(".sub_cont"));
                    for(var j in answer){
                        var tmp_letter = $("<li class='ui-state-default'>" + answer[j] + "</li>");
                        tmp_array.push(tmp_letter);
                    }                        
                    tmp_array = shuffle(tmp_array);
                    for(var i = 0; i<tmp_array.length;i++)
                        tmp_array[i].appendTo(ul_cont);
                    ul_cont.sortable({
                        revert: true, 
                        placeholder: "highlight", 
                        update: checkResult
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
                $("#parameters").css("display","block");
                $(".cont").each(function(){
                    var container = $(this);
//                    $("<div class='close_cont'>").appendTo(container);
                    container.find("#sortable").removeClass("imgs_answers_red")
                    .removeClass("imgs_answers_green")
                    .addClass("imgs_answers_gray")
                    .sortable("destroy");
                    container.find(".text_cont .audio_desc").attr("contenteditable","true");
                    container.find(".audio_block").attr("ondragenter", "return false;")
                    .attr("ondragleave", "$(this).removeClass('audio_gray'); return false;")
                    .attr("ondragover", "$(this).addClass('audio_gray'); return false;")
                    .attr("ondrop", "$(this).removeClass('audio_gray'); return onDropAudio(this,event);");                    
                    $("<div class='audio_answer' contenteditable>" + container.find("ul").next().val() + "</div>").appendTo(container);
                    container.find("ul").remove();
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
            cont_obj.audio = $(this).find("source").attr("src");
            cont_obj.answer = $(this).find(".audio_answer").text();
            cont_obj.cur_answer = "";            
            array_to_export.push(cont_obj);
        });
    } else {
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.text = $(this).find(".audio_desc").text();
            cont_obj.audio = $(this).find("source").attr("src");
            cont_obj.answer = $(this).find("ul").next().val(); 
            cont_obj.cur_answer = getAnswer($(this).find("ul"));
            array_to_export.push(cont_obj);
        });
    }
    sankore.setPreference("ord_let", JSON.stringify(array_to_export));
    if($("#wgt_display").hasClass("selected"))
        sankore.setPreference("ord_let_state", "display");
    else
        sankore.setPreference("ord_let_state", "edit");
}

//import
function importData(data){
   
    for(var i in data){        
        var tmp_array = [];
        var container = $("<div class='cont'>").appendTo("#data");
        var sub_container = $("<div class='sub_cont'>").appendTo(container);
        var imgs_container = $("<ul id='sortable' class='imgs_answers_gray'>").appendTo(container);   
        
//        $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
        var text = $("<div class='text_cont'>").appendTo(sub_container);
        var audio_block = $("<div class='audio_block'>").appendTo(text);
        $("<div class='play'>").appendTo(audio_block);
        $("<div class='replay'>").appendTo(audio_block);
        var source = $("<source/>").attr("src", data[i].audio);
        var audio = $("<audio>").appendTo(audio_block);
        audio.append(source);
        $("<input type='hidden'/>").appendTo(audio_block);
        $("<div class='audio_desc'>" + data[i].text + "</div>").appendTo(text);
        $("<input type='hidden' value='" + data[i].answer + "'/>").appendTo(container);
        if(data[i].cur_answer)
            for(var j in data[i].cur_answer){
                var tmp_letter = $("<li class='ui-state-default'>" + data[i].cur_answer[j] + "</li>");
                tmp_array.push(tmp_letter);
            } 
        else
            for(j in data[i].answer){
                tmp_letter = $("<li class='ui-state-default'>" + data[i].answer[j] + "</li>");                
                tmp_array.push(tmp_letter);
            }
        
        if(sankore.preference("ord_let_state","")){
            if(sankore.preference("ord_let_state","") == "edit")
                tmp_array = shuffle(tmp_array);
        } else 
            tmp_array = shuffle(tmp_array);
        
        for(j = 0; j<tmp_array.length;j++)
            tmp_array[j].appendTo(imgs_container);
        
        imgs_container.sortable({
            revert: true, 
            placeholder: "highlight"
        }).bind('sortupdate', function(event, ui) {
            checkResult(event);
        }); 
        if(data[i].cur_answer)
            imgs_container.trigger("sortupdate")
    }
}

//example
function showExample(){
    
    var tmp_array = [];    
    var container = $("<div class='cont'>").appendTo("#data");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container = $("<ul id='sortable' class='imgs_answers_gray'>").appendTo(container);

//    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>").appendTo(sub_container);
    var audio_block = $("<div class='audio_block'>").appendTo(text);
    $("<div class='play'>").appendTo(audio_block);
    $("<div class='replay'>").appendTo(audio_block);
    var source = $("<source/>").attr("src", "../../objects/example_fr.mp3");
    var audio = $("<audio>").appendTo(audio_block);
    audio.append(source);
    $("<input type='hidden'/>").appendTo(audio_block);
    var audio_desc = $("<div class='audio_desc'>" + sankoreLang.short_desc + "</div>").appendTo(text);
    
    $("<input type='hidden' value='" + sankoreLang.example + "'/>").appendTo(container);
    
    for(var j in sankoreLang.example){
        var tmp_letter = $("<li class='ui-state-default'>" + sankoreLang.example[j] + "</li>");
        tmp_array.push(tmp_letter);
    } 
    
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++)
        tmp_array[i].appendTo(imgs_container);
    imgs_container.sortable({
        revert: true, 
        placeholder: "highlight"
    }).bind('sortupdate', function(event, ui) {
        checkResult(event);
    });
}

//add new container
//function addContainer(){
//    var container = $("<div class='cont'>");
//    var sub_container = $("<div class='sub_cont'>").appendTo(container);
//   
//    $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
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
//    $("<div class='audio_desc' contenteditable>" + sankoreLang.enter + "</div>").appendTo(text);
//    
//    var tmp_input = $("<input type='hidden' value=''/>").insertAfter(sub_container);
//    var close = $("<div class='close_cont'>").insertAfter(tmp_input);
//    $("<div class='audio_answer' contenteditable>" + sankoreLang.example + "</div>").insertAfter(close);
//    container.insertBefore($(".add_block"));
//}

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

//check result
function checkResult(event)
{
    var str = "";
    var right_str = $(event.target).next().val();
    $(event.target).find("li.ui-state-default").each(function(){
        str += $(this).text();
    });
    if(str == right_str)
        $(event.target).removeClass("imgs_answers_gray")
        .removeClass("imgs_answers_red")
        .addClass("imgs_answers_green");
    else
        $(event.target).removeClass("imgs_answers_gray")
        .removeClass("imgs_answers_green");
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
            $(obj).find("audio").remove();
            $(obj).find(":first-child").removeClass("stop").addClass("play");
            var source = $("<source/>").attr("src", "../../" + tmp);
            var audio = $("<audio>").appendTo($(obj));
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

//get text
function getAnswer(obj){
    var answer = "";
    obj.find("li.ui-state-default").each(function(){
        answer += $(this).text();
    });
    return answer;
}

if (window.widget) {
    window.widget.onremove = function(){
        $("audio").each(function(){
            this.pause();
            $(this).parent().find(":first-child").removeClass("stop").addClass("play");
        });
    }
}