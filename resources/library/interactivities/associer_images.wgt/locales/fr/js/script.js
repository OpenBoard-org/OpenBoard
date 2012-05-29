var sankoreLang = {
    display: "Affichage", 
    edit: "Modifier", 
    short_desc: "Sélectionner le numéro «trois».", 
    add: "Nouveau bloc",
    enter: "Saisissez la consigne ici ...",
    wgt_name: "Associer des images",
    reload: "Recharger",
    slate: "Bois",
    pad: "Pad"
};

//main function
function start(){

    $("#wgt_display").text(sankoreLang.display);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_name").text(sankoreLang.wgt_name);
    $("#wgt_reload").text(sankoreLang.reload);
    $(".style_select option[value='1']").text(sankoreLang.slate);
    $(".style_select option[value='2']").text(sankoreLang.pad);
    
    if(window.sankore){
        if(sankore.preference("associer","")){
            var data = jQuery.parseJSON(sankore.preference("associer",""));
            importData(data);
        } else {
            showExample();
        }
    } 
    else 
        showExample();
    //events
    if (window.widget) {
        window.widget.onleave = function(){
            exportData();
        }
    }
    
    $("#wgt_reload").click(function(){
        window.location.reload();
    });
    
    $(".style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
        
    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){
                sankore.enableDropOnWidget(false);
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $(".style_select").css("display","none");
                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_i = 0;
                    var tmp_array = [];
                    
                    container.find(".text_cont").removeAttr("contenteditable");
                    container.find(".add_img").remove();
                    container.find(".close_cont").remove();
                    container.find(".img_block").each(function(){
                        if($(this).find("img").attr("src") != "img/drop_img.png"){
                            $(this).find(".close_img").remove();
                            $(this).find(".clear_img").remove();
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
                sankore.enableDropOnWidget(true);
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $(".style_select").css("display","block");
                
                $(".cont").each(function(){
                    var container = $(this);
                    container.find(".imgs_answers").find(".img_block").each(function(){
                        $(this).appendTo(container.find(".imgs_cont"))
                    });
                    container.find(".imgs_answers").remove();
                    $("<div class='close_cont'>").appendTo(container);
                    container.find(".text_cont").attr("contenteditable","true");
                    //container.find(".imgs_cont").sortable("destroy");
                    //container.find(".imgs_cont").css("background-color", "white");
                    
                    var add_img = $("<div class='add_img'>");
                    container.find(".img_block").each(function(){
                        $(this).draggable("destroy");
                        $(this).attr("ondragenter", "return false;")
                        .attr("ondragleave", "$(this).css(\"background-color\",\"\"); return false;")
                        .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
                        .attr("ondrop", "$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);")
                        .css("float","left");
                        $("<div class='close_img'>").appendTo($(this));
                        $("<div class='clear_img'>").appendTo($(this));
                        if($(this).find("input").val() == "1"){
                            $("<div class='false_img'>").appendTo($(this));
                        }
                        else{
                            $("<div class='true_img'>").appendTo($(this));
                        }
                    });
                    container.find(".imgs_cont").append(add_img)
                });                
                
                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("#data");
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
            }
        }
    });
    
    //add new block
    $(".add_block").live("click", function(){
        addContainer();
    });
    
    //adding new img
    $(".add_img").live("click", function(){
        addImgBlock($(this));
    });
    
    //deleting a block
    $(".close_cont").live("click",function(){
        $(this).parent().remove();
        refreshBlockNumbers();
    });
    
    //deleting the img block
    $(".close_img").live("click", function(){
        var i = 0;
        var tmp_obj = $(this).parent().parent();        
        $(this).parent().remove();        
    });
    
    //cleaning an image
    $(".clear_img").live("click",function(){
        $(this).parent().find("img").attr("src","img/drop_img.png");
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
}

//export
function exportData(){
    var array_to_export = [];
    if($("#wgt_edit").hasClass("selected")){
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.style = $(".style_select").find("option:selected").val();
            cont_obj.text = $(this).find(".text_cont").text();
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
            cont_obj.style = $(".style_select").find("option:selected").val();
            cont_obj.text = $(this).find(".text_cont").text();
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
    
    if($(".cont").size() == 0){
        var cont_obj = new Object();
        cont_obj.style = $(".style_select").find("option:selected").val();
        cont_obj.tmp = "clear";
        array_to_export.push(cont_obj);
    }
    
    sankore.setPreference("associer", JSON.stringify(array_to_export));
}

//import
function importData(data){
    
    var tmp = 0;    
    for(var i in data){
        if(data[i].tmp){
            changeStyle(data[i].style);
            $(".style_select").val(data[i].style);
        } else {
            if(i == 0){
                changeStyle(data[i].style);
                $(".style_select").val(data[i].style);
            }
            if(data[i].mode == "edit"){          
                var tmp_array = [];
                var container = $("<div class='cont'>");
                var sub_container = $("<div class='sub_cont'>").appendTo(container);
                var imgs_answers = $("<div class='imgs_answers imgs_answers_gray'><img src='img/drop_img.png' style='margin-top: 11px;'/></div>").appendTo(container);
                var imgs_container = $("<div class='imgs_cont'>").appendTo(container);    
        
                var number = $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
                var text = $("<div class='text_cont'>" + data[i].text + "</div>").appendTo(sub_container);
        
                for(var j in data[i].imgs){
                    var img_block = $("<div class='img_block img_gray' style='text-align: center;'>");
                    var img = $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;'>");
                    img.height(data[i].imgs[j].ht).width(data[i].imgs[j].wd);
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
        
                number = $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
                text = $("<div class='text_cont'>" + data[i].text + "</div>").appendTo(sub_container);
        
                for(j in data[i].imgs){
                    img_block = $("<div class='img_block img_gray' style='text-align: center;'>");
                    img = $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;'>");
                    img.height(data[i].imgs[j].ht).width(data[i].imgs[j].wd);
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
}

//example
function showExample(){
    
    changeStyle(1);
    var tmp_array = [];
    
    var container = $("<div class='cont'>").appendTo("#data");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_answers = $("<div class='imgs_answers imgs_answers_gray'><img src='img/drop_img.png' style='margin-top: 11px;'/></div>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);

    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>" + sankoreLang.short_desc + "</div>").appendTo(sub_container);
    
    $("<input type='hidden' value='1'/>").appendTo(imgs_container);
    
    var img1 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img1);
    $("<img src=\"../../objects/0.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img1);
    var img2 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img2);
    $("<img src=\"../../objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img2);
    var img3 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='0'/>").appendTo(img3);
    $("<img src=\"../../objects/2.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img3);
    var img4 = $("<div class='img_block img_gray' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(img4);
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
//container.appendTo("body")
}

//changing the style
function changeStyle(val){
    if(val == 1){
        $(".b_top_left").removeClass("btl_pad");
        $(".b_top_center").removeClass("btc_pad");
        $(".b_top_right").removeClass("btr_pad");
        $(".b_center_left").removeClass("bcl_pad");
        $(".b_center_right").removeClass("bcr_pad");
        $(".b_bottom_right").removeClass("bbr_pad");
        $(".b_bottom_left").removeClass("bbl_pad");
        $(".b_bottom_center").removeClass("bbc_pad");
        $("#wgt_reload").removeClass("pad_color").removeClass("pad_reload");
        $("#wgt_edit").removeClass("pad_color").removeClass("pad_edit");
        $("#wgt_display").removeClass("pad_color").removeClass("pad_edit");
        $("#wgt_name").removeClass("pad_color");
        $(".style_select").removeClass("pad_select");
    } else {
        $(".b_top_left").addClass("btl_pad");
        $(".b_top_center").addClass("btc_pad");
        $(".b_top_right").addClass("btr_pad");
        $(".b_center_left").addClass("bcl_pad");
        $(".b_center_right").addClass("bcr_pad");
        $(".b_bottom_right").addClass("bbr_pad");
        $(".b_bottom_left").addClass("bbl_pad");
        $(".b_bottom_center").addClass("bbc_pad");
        $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
        $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
        $("#wgt_display").addClass("pad_color").addClass("pad_edit");
        $("#wgt_name").addClass("pad_color");
        $(".style_select").addClass("pad_select");
    }
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
function addContainer(){
    var container = $("<div class='cont'>");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);
    
    var close = $("<div class='close_cont'>").appendTo(container);
    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
    var text = $("<div class='text_cont' contenteditable>" + sankoreLang.enter + "</div>").appendTo(sub_container);
    
    $("<input type='hidden' value='1*2*3*4*5*'/>").appendTo(imgs_container);
    var add_img = $("<div class='add_img'>").appendTo(imgs_container);
    container.insertBefore($(".add_block"));
}

//add new img block
function addImgBlock(dest){
    var img_block = $("<div class='img_block img_gray' ondragenter='return false;' ondragleave='$(this).css(\"background-color\",\"\"); return false;' ondragover='$(this).css(\"background-color\",\"#ccc\"); return false;' ondrop='$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);' style='text-align: center; float: left;'></div>").insertBefore(dest);
    $("<div class='close_img'>").appendTo(img_block);
    $("<div class='clear_img'>").appendTo(img_block);
    $("<div class='true_img'>").appendTo(img_block);
    $("<input type='hidden' value='0'/>").appendTo(img_block);
    $("<img src='img/drop_img.png' height='120'/>").appendTo(img_block);
}

function refreshBlockNumbers(){
    var i = 0;
    $(".cont").each(function(){
        $(this).find(".number_cont").text(++i);
    })
}

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

