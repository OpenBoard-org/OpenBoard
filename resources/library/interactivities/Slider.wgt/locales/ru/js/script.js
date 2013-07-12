var sankoreLang = {
    display: "Закрыть", 
    edit: "Изменить",
    text_content: "Это - пример. Вместо этого текста вы можете ввести свой собственный контент.", 
    new_txt: "Новый текстовый блок.",
    new_slide: "Это - новый слайд.",
    wgt_name: "Слайдер",
    slate: "Узор",
    pad: "Планшет",
    none: "Нет",
    help: "Помощь",
    help_content: "Пример текста помощи ...",
    theme: "Тема"
};

//some flags
var mouse_state = false;

//object for resize
var resize_obj = {
    object: null,
    top: 0,
    left: 0,
    clicked: false,
    k: 0
}

//main function
function start(){

    $("#wgt_display").text(sankoreLang.display);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_name").text(sankoreLang.wgt_name);
    $("#wgt_help").text(sankoreLang.help);
    $("#help").html(sankoreLang.help_content);
    $("#style_select option[value='1']").text(sankoreLang.slate);
    $("#style_select option[value='2']").text(sankoreLang.pad);
    $("#style_select option[value='3']").text(sankoreLang.none);
    var tmpl = $("div.inline label").html();
    $("div.inline label").html(sankoreLang.theme + tmpl)
    
    if(window.sankore){
        if(sankore.preference("etudier","")){
            var data = jQuery.parseJSON(sankore.preference("etudier",""));
            importData(data);
        }
        else 
            showExample();
        if(sankore.preference("etudier_style","")){
            changeStyle(sankore.preference("etudier_style",""));
            $("#style_select").val(sankore.preference("etudier_style",""));
        } else
            changeStyle("3")
    } 
    else 
        showExample();
    
    //events
    if (window.widget) {
        window.widget.onleave = function(){
            if(!$("#wgt_help").hasClass("open")){
                exportData();
                sankore.setPreference("etudier_style", $("#style_select").find("option:selected").val());
                sankore.setPreference("etudier_cur_page", $("#slider").getPage());
                sankore.setPreference("etudier_left_nav", $("#prevBtn a").css("display"));
                sankore.setPreference("etudier_right_nav", $("#nextBtn a").css("display"));
            }
        }
    }
    
    $("#style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
    
    $("#wgt_help").click(function(){
        var tmp = $(this);
        if($(this).hasClass("open")){
            $(this).removeClass("help_pad").removeClass("help_wood")
            $("#help").hide();
            tmp.removeClass("open");
            $("#slider").show();
        } else {
            ($("#style_select").val() == 1)?$(this).removeClass("help_pad").addClass("help_wood"):$(this).removeClass("help_wood").addClass("help_pad");
            exportData();
            sankore.setPreference("etudier_style", $("#style_select").find("option:selected").val());
            sankore.setPreference("etudier_cur_page", $("#slider").getPage());
            sankore.setPreference("etudier_left_nav", $("#prevBtn a").css("display"));
            sankore.setPreference("etudier_right_nav", $("#nextBtn a").css("display"));            
            $("#slider").hide();
            $("#help").show();
            tmp.addClass("open");
        }
    });
    
    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){
                if(window.sankore)
                    sankore.enableDropOnWidget(false);
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $("#parameters").css("display","none");
                var tmpwh = $(window).height();
                var tmpww = $(window).width();
                window.resizeTo(tmpww, tmpwh - 44)
                
                $("#slider li>div").each(function(){
                    var container = $(this);
                    container.removeAttr("ondragenter")
                    .removeAttr("ondragleave")
                    .removeAttr("ondragover")
                    .removeAttr("ondrop");
                    
                    container.find(".text_block").each(function(){
                        $(this).draggable();
                        $(this).find(".move_block").remove();
                        $(this).find(".close_img").remove();
                        $(this).find(".size_up").remove();
                        $(this).find(".size_down").remove();
                        $(this).find(".resize_block").remove();
                        $(this).find(".real_text").removeAttr("contenteditable");
                        $(this).removeClass("block_border");
                        $(this).css("position","absolute");
                    });
                    
                    container.find(".img_block").each(function(){
                        $(this).draggable();
                        $(this).find(".close_img").remove();
                        $(this).find(".move_block").remove();
                        $(this).find(".resize_block").remove();
                        $(this).removeClass("block_border");
                        $(this).css("position","absolute");
                    });
                    
                    container.find(".audio_block").each(function(){
                        $(this).find(".close_img").remove();
                        $(this).removeClass("block_border");
                        $(this).css("position","absolute");
                    });
                    
                    container.find(".add_left").remove();
                    container.find(".add_right").remove();
                    container.find(".close_slide").remove();
                    container.find(".add_text").remove();
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
                tmpwh = $(window).height();
                tmpww = $(window).width();
                window.resizeTo(tmpww, tmpwh + 44)
                
                $("#slider li>div").each(function(){
                    var container = $(this);
                    container.attr("ondragenter", "return false;")
                    .attr("ondragleave", "$(this).css(\"background\",\"none\"); return false;")
                    .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
                    .attr("ondrop", "$(this).css(\"background\",\"none\"); return onDropTarget(this,event);");
                    
                    container.find(".text_block").each(function(){
                        $(this).draggable("destroy");
                        $("<div class='move_block' contenteditable='false'>").appendTo($(this));
                        $("<div class='close_img' contenteditable='false'>").appendTo($(this));
                        $("<div class='size_up' contenteditable='false'>").appendTo($(this));
                        $("<div class='size_down' contenteditable='false'>").appendTo($(this));
                        $("<div class='resize_block' contenteditable='false'>").appendTo($(this));
                        $(this).find(".real_text").attr("contenteditable", "true");
                        $(this).addClass("block_border");
                    });
                    
                    container.find(".img_block").each(function(){
                        $(this).draggable("destroy");
                        $("<div class='move_block' contenteditable='false'>").appendTo($(this));
                        $("<div class='close_img' contenteditable='false'>").appendTo($(this));
                        $("<div class='resize_block' contenteditable='false'>").appendTo($(this));
                        $(this).addClass("block_border");
                    });
                    
                    container.find(".audio_block").each(function(){
                        $("<div class='close_img' contenteditable='false'>").appendTo($(this));
                        $(this).addClass("block_border");
                    });
                    
                    $("<div class='add_left'>").appendTo(container);
                    $("<div class='add_right'>").appendTo(container);
                    $("<div class='close_slide'>").appendTo(container);
                    $("<div class='add_text'>").appendTo(container);
                    $(window).trigger("resize")
                });        
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
            }
        }
        
        $("audio").each(function(){
            this.pause();
            $(this).parent().find(":first-child").removeClass("stop").addClass("play");
        });
        
    });
     
    //deleting the img block
    $(".close_img").live("click", function(){    
        $(this).parent().remove();        
    });
    
    //increase a size of text
    $(".size_up").live("click", function(){
        $(this).parent().height("");
        var fz = parseInt($(this).parent().css("font-size").replace("px", ""));
        $(this).parent().css("font-size", fz+1 + "px");
    });
    
    //decrease a size of text
    $(".size_down").live("click", function(){    
        var fz = parseInt($(this).parent().css("font-size").replace("px", ""));
        fz = ((fz - 1) < 8)?8:fz-1;
        $(this).parent().css("font-size", fz + "px");       
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
    
    //moving objects
    $(".move_block").live("mouseover",function(){
        $(this).parent().draggable();
    });

    $(".move_block").live("mouseleave",function(){
        if(!mouse_state)
            $(this).parent().draggable("destroy");
    });
    
    $(".move_block").live("mousedown",function(){
        mouse_state = true;
    });
    
    $(".move_block").live("mouseup",function(){
        mouse_state = false;
    });
    
    //resize block
    $(".resize_block").live("mousedown", function(){
        resize_obj.object = $(this);
        resize_obj.top = event.clientY;
        resize_obj.left = event.clientX;
        resize_obj.clicked = true;
        if($(this).parent().hasClass("img_block"))
            resize_obj.k = $(this).parent().find("img").width() / $(this).parent().find("img").height();
    })
    
    $("li>div").live("mouseup", function(){
        resize_obj.object = null;
        resize_obj.top = 0;
        resize_obj.left = 0;
        resize_obj.clicked = false;
    })
    
    $("li>div").live("mousemove", function(){
        if(resize_obj.clicked){
            if(resize_obj.object.parent().hasClass("text_block")){                
                var width = resize_obj.object.parent().width() - resize_obj.left + event.clientX;
                //var height = resize_obj.object.parent().height() - resize_obj.top + event.clientY;
                resize_obj.left = event.clientX;
                resize_obj.top = event.clientY;
                resize_obj.object.parent().width(width);
            } else {
                var img_width = resize_obj.object.parent().find("img").width() - resize_obj.left + event.clientX;
                var img_height = img_width / resize_obj.k;
                resize_obj.left = event.clientX;
                resize_obj.top = event.clientY;
                resize_obj.object.parent().find("img").width(img_width).height(img_height);
            }
        }
    });
    
    //closing a slide
    $(".close_slide").live("click", function(){
        $(this).parent().parent().remove();
        $("#slider").removeSlide();
    });
    
    //adding new slides
    $(".add_left").live("click", function(){
        var cur_li = $(this).parent().parent();
        var new_li = $("<li>");
        new_li.width(cur_li.width()).height(cur_li.height()).css("float","left");
        var new_div = $("<div>").appendTo(new_li);
        new_div.attr("ondragenter", "return false;")
        .attr("ondragleave", "$(this).css(\"background-color\",\"\"); return false;")
        .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
        .attr("ondrop", "$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);");
        var example = $("<div class='text_block' style='position: absolute;'>").addClass("block_border");
        $("<div class='real_text' contenteditable='true'>" + sankoreLang.new_slide + "</div>").appendTo(example);
        $("<div class='move_block' contenteditable='false'>").appendTo(example);
        $("<div class='close_img' contenteditable='false'>").appendTo(example);
        $("<div class='size_up' contenteditable='false'>").appendTo(example);
        $("<div class='size_down' contenteditable='false'>").appendTo(example);
        $("<div class='resize_block' contenteditable='false'>").appendTo(example);
        example.css("top","40%").css("left","40%");
        new_div.append(example);
        $("<div class='add_left'>").appendTo(new_div);
        $("<div class='add_right'>").appendTo(new_div);
        $("<div class='close_slide'>").appendTo(new_div);
        $("<div class='add_text'>").appendTo(new_div);
        new_li.insertBefore(cur_li);
        $("#slider").addSlide("before");
    });
    
    $(".add_right").live("click", function(){
        var cur_li = $(this).parent().parent();
        var new_li = $("<li>");
        new_li.width(cur_li.width()).height(cur_li.height()).css("float","left");
        var new_div = $("<div>").appendTo(new_li);
        new_div.attr("ondragenter", "return false;")
        .attr("ondragleave", "$(this).css(\"background-color\",\"\"); return false;")
        .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
        .attr("ondrop", "$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);");
        var example = $("<div class='text_block' style='position: absolute;'>").addClass("block_border");
        $("<div class='real_text' contenteditable='true'>" + sankoreLang.new_slide + "</div>").appendTo(example);
        $("<div class='move_block' contenteditable='false'>").appendTo(example);
        $("<div class='close_img' contenteditable='false'>").appendTo(example);
        $("<div class='size_up' contenteditable='false'>").appendTo(example);
        $("<div class='size_down' contenteditable='false'>").appendTo(example);
        $("<div class='resize_block' contenteditable='false'>").appendTo(example);
        example.css("top","40%").css("left","40%");
        new_div.append(example);
        $("<div class='add_left'>").appendTo(new_div);
        $("<div class='add_right'>").appendTo(new_div);
        $("<div class='close_slide'>").appendTo(new_div);
        $("<div class='add_text'>").appendTo(new_div);
        new_li.insertAfter(cur_li);
        $("#slider").addSlide("after");
    });
    
    $(".add_text").live("click", function(){
        var container = $(this).parent();
        var text_block = $("<div class='text_block'><div class='real_text' contenteditable='true'>" + sankoreLang.new_txt + "</div></div>").appendTo(container);
        $("<div class='move_block' contenteditable='false'>").appendTo(text_block);
        $("<div class='close_img' contenteditable='false'>").appendTo(text_block);
        $("<div class='size_up' contenteditable='false'>").appendTo(text_block);
        $("<div class='size_down' contenteditable='false'>").appendTo(text_block);
        $("<div class='resize_block' contenteditable='false'>").appendTo(text_block);
        text_block.addClass("block_border");
    });
}

//export
function exportData(){
    var array_to_export = [];
    $("#slider li>div").each(function(){
        var cont_obj = new Object();
        cont_obj.w = $(this).parent().width();
        cont_obj.h = $(this).parent().height();
        cont_obj.text = [];
        $(this).find(".text_block").each(function(){
            var txt_block = new Object();
            txt_block.top = $(this).position().top;
            txt_block.left = $(this).position().left;
            txt_block.w = $(this).width();
            txt_block.fz = $(this).css("font-size");
            txt_block.val = $(this).find(".real_text").html();
            cont_obj.text.push(txt_block);
        });
        cont_obj.imgs = [];
        $(this).find(".img_block").each(function(){
            var img_obj = new Object();
            img_obj.link = $(this).find("img").attr("src").replace("../../","");
            img_obj.h = $(this).find("img").height();
            img_obj.w = $(this).find("img").width();
            img_obj.top = $(this).position().top;
            img_obj.left = $(this).position().left;
            cont_obj.imgs.push(img_obj);
        });
        cont_obj.audio = [];
        $(this).find(".audio_block").each(function(){
            var audio_block = new Object();
            audio_block.top = $(this).position().top;
            audio_block.left = $(this).position().left;
            audio_block.val = $(this).find("source").attr("src").replace("../../","");
            cont_obj.audio.push(audio_block);
        });
        array_to_export.push(cont_obj);
    });
    sankore.setPreference("etudier", JSON.stringify(array_to_export));
}

//import
function importData(data){
    
    var width = 0;
    var height = 0;
    
    for(var i in data){
        
        width = data[i].w;
        height = data[i].h;
        var li = $("<li style='float: left; width: " + data[i].w + "; height: " + data[i].h + ";'>");
        var div = $("<div>").appendTo(li);
        
        for(var j in data[i].text){
            var text_div = $("<div class='text_block'><div class='real_text'>" + data[i].text[j].val + "</div></div>");
            text_div.draggable().css("position","absolute")
            .width(data[i].text[j].w)
            .css("top", data[i].text[j].top)
            .css("left", data[i].text[j].left)
            .css("font-size", data[i].text[j].fz)
            .appendTo(div);
        }
        
        for(j in data[i].imgs){
            var img_div = $("<div class='img_block' style='text-align: center;'>");            
            img_div.draggable().css("position","absolute")
            .css("top", data[i].imgs[j].top)
            .css("left", data[i].imgs[j].left)
            .appendTo(div);
            $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;' width='" + data[i].imgs[j].w + "' height='" + data[i].imgs[j].h + "'/>").appendTo(img_div);
        }
        
        for(j in data[i].audio){
            var audio_div = $("<div class='audio_block'>");
            $("<div class='play'>").appendTo(audio_div);
            $("<div class='replay'>").appendTo(audio_div);
            var tmp_audio = $("<audio>").appendTo(audio_div);
            $("<source src='../../" + data[i].audio[j].val + "' />").appendTo(tmp_audio);
            audio_div.draggable().css("position","absolute")
            .css("top", data[i].audio[j].top)
            .css("left", data[i].audio[j].left)
            .appendTo(div);
        }
        
        $("#slider ul").append(li);
        
    }
    
    $(window).trigger("resize")
    
    $("#slider").width(width).height(height).easySlider({
        prevText: '',
        nextText: '',
        controlsShow: false
    });
    $("#slider").goToSlide(sankore.preference("etudier_cur_page",""));
    $("#prevBtn a").css("display", sankore.preference("etudier_left_nav",""));
    $("#nextBtn a").css("display", sankore.preference("etudier_right_nav",""));
}

//example
function showExample(){
    
    var li1 = $("<li>");
    var div1 = $("<div>").appendTo(li1);
    $("<div class='text_block'><div class='real_text'>" + sankoreLang.text_content + "</div></div>").draggable().appendTo(div1)
    li1.width($("#slider").width()).height($("#slider").height());
    $("#slider ul").append(li1);
    var li2 = $("<li>");
    var div2 = $("<div>").appendTo(li2);
    var img = $("<div class='img_block' style='text-align: center;'></div>").draggable().appendTo(div2);
    $("<img src=\"../../objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img);
    li2.width($("#slider").width()).height($("#slider").height());
    $("#slider ul").append(li2);
    var li3 = $("<li>");
    var div3 = $("<div>").appendTo(li3);
    li3.width($("#slider").width()).height($("#slider").height());
    var audio_block = $("<div class='audio_block'>").draggable().appendTo(div3);
    $("<div class='play'>").appendTo(audio_block);
    $("<div class='replay'>").appendTo(audio_block);
    var source = $("<source/>").attr("src", "../../objects/bateaux.mp3");
    var audio = $("<audio>").appendTo(audio_block);
    audio.append(source);
    $("#slider ul").append(li3);
    var li4 = $("<li>");
    var div4 = $("<div>").appendTo(li4);
    $("<div class='text_block'><div class='real_text'>" + sankoreLang.text_content + "</div></div>").draggable().appendTo(div4);
    var img2 = $("<div class='img_block' style='text-align: center;'></div>").draggable().appendTo(div4);
    $("<img src=\"../../objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img2);
    var audio_block2 = $("<div class='audio_block'>").draggable().appendTo(div4);
    $("<div class='play'>").appendTo(audio_block2);
    $("<div class='replay'>").appendTo(audio_block2);
    var source2 = $("<source/>").attr("src", "../../objects/bateaux.mp3");
    var audio2 = $("<audio>").appendTo(audio_block2);
    audio2.append(source2);
    li4.width($("#slider").width()).height($("#slider").height());
    $("#slider ul").append(li4);
    $(".text_block, .audio_block, .img_block").each(function(){
        $(this).css("position","absolute");
    });
    $("#slider").easySlider({
        prevText: '',
        nextText: '',
        controlsShow: false
    });
}

//string into xml-format
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
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_name").addClass("pad_color");
            $("#wgt_display").removeClass("display_wood");
            $("#style_select").val(val);
            $("body, html").addClass("without_radius").removeClass("radius_ft");
            break;
    }
}

//drop handler
function onDropTarget(obj, event) {
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
            var audio_block = $("<div class='audio_block'>").draggable().appendTo($(obj));
            audio_block.css("position","absolute").css("top",event.clientY - 54).css("left",event.clientX - 54);
            $("<div class='close_img' contenteditable='false'>").appendTo(audio_block);
            audio_block.addClass("block_border");
            $("<div class='play'>").appendTo(audio_block);
            $("<div class='replay'>").appendTo(audio_block);
            var source = $("<source/>").attr("src", "../../" + tmp);
            var audio = $("<audio>").appendTo(audio_block);
            audio.append(source);
        } else {
            var img_block = $("<div class='img_block' style='text-align: center;'></div>").appendTo($(obj));
            img_block.css("top",event.clientY - 54).css("left",event.clientX - 54);
            $("<div class='move_block' contenteditable='false'>").appendTo(img_block);
            $("<div class='close_img' contenteditable='false'>").appendTo(img_block);
            $("<div class='resize_block' contenteditable='false'>").appendTo(img_block);
            img_block.addClass("block_border");
            var tmp_img = $("<img src=\"../../" + tmp + "\" style=\"display: inline;\"/>").appendTo(img_block);
            setTimeout(function(){
                if(tmp_img.height() >= tmp_img.width())
                    tmp_img.attr("height", "120");
                else{
                    tmp_img.attr("width","120");
                    tmp_img.css("margin",(120 - tmp_img.height())/2 + "px 0");
                }
            }, 6)
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

$(window).resize(function(){
    var slider = $("#slider");
    slider.width($(this).width() - 108).height($(this).height() - 108);
    $("#slider li").each(function(){
        $(this).width(slider.width()).height(slider.height());
    });
    slider.setSize(slider.width(), slider.height());
})