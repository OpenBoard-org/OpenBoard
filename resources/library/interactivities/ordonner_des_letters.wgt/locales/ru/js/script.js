var sankoreLang = {
    display: "Показать", 
    edit: "Изменить", 
    short_desc: "Прослушайте звук и составьте корректное слово.", 
    add: "Новый блок",
    enter: "Введите описание ...",
    example: "пример"
};

//main function
function start(){

    $("#display_text").text(sankoreLang.display);
    $("#edit_text").text(sankoreLang.edit);
    
    if(window.sankore){
        if(sankore.preference("associer_sound","")){
            var data = jQuery.parseJSON(sankore.preference("associer_sound",""));
            importData(data);
        }
        else {
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
    
    $("#display, #edit").click(function(event){
        if(this.id == "display"){
            if(!$(this).hasClass("selected")){
                if(window.sankore)
                    sankore.enableDropOnWidget(false);
                $(this).addClass("selected");
                $("#display_img").removeClass("red_point").addClass("green_point");
                $("#edit_img").removeClass("green_point").addClass("red_point");
                $("#edit").removeClass("selected");
                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_array = [];
                    var imgs_container = container.find(".imgs_cont");
                    
                    container.find(".text_cont .audio_desc").removeAttr("contenteditable");
                    container.find(".text_cont").removeAttr("ondragenter")
                    .removeAttr("ondragleave")
                    .removeAttr("ondragover")
                    .removeAttr("ondrop")
                    container.find(".close_cont").remove();
                    var answer = imgs_container.find(".audio_answer").text();
                    imgs_container.find(".audio_answer").remove();
                    imgs_container.find("input").val(answer);
                    for(var j in answer){
                        var tmp_letter = $("<div class='img_block' style='text-align: center;'>" + answer[j] + "</div>");
                        tmp_array.push(tmp_letter);
                    }                        
                    tmp_array = shuffle(tmp_array);
                    for(var i = 0; i<tmp_array.length;i++)
                        tmp_array[i].appendTo(imgs_container);
                    imgs_container.sortable( {
                        update: checkResult
                    } );
                });
            }
        } else {            
            if(!$(this).hasClass("selected")){
                if(window.sankore)
                    sankore.enableDropOnWidget(true);
                $(this).addClass("selected");
                $("#edit_img").removeClass("red_point").addClass("green_point");
                $("#display_img").removeClass("green_point").addClass("red_point");
                $("#display").removeClass("selected");
                
                $(".cont").each(function(){
                    var container = $(this);
                    $("<div class='close_cont'>").appendTo(container);
                    container.find(".imgs_cont").removeClass("imgs_answers_red")
                    .removeClass("imgs_answers_green")
                    .addClass("imgs_answers_gray")
                    .sortable("destroy");
                    container.find(".text_cont .audio_desc").attr("contenteditable","true");
                    container.find(".text_cont").attr("ondragenter", "return false;")
                    .attr("ondragleave", "$(this).removeClass('gray'); return false;")
                    .attr("ondragover", "$(this).addClass('gray'); return false;")
                    .attr("ondrop", "$(this).removeClass('gray'); return onDropAudio(this,event);");
                    container.find(".img_block").remove();
                    $("<div class='audio_answer' contenteditable>" + container.find(".imgs_cont input").val() + "</div>").appendTo(container.find(".imgs_cont"));
                });                
                
                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("body");
            }
        }
        
        $("audio").each(function(){
            this.pause();
            $(this).parent().find(":first-child").removeClass("stop").addClass("play");
        });
        
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
    if($("#edit").hasClass("selected")){
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.text = $(this).find(".audio_desc").text();
            cont_obj.audio = $(this).find("source").attr("src").replace("../../","");
            cont_obj.answer = $(this).find(".audio_answer").text();            
            array_to_export.push(cont_obj);
        });
    } else {
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.text = $(this).find(".audio_desc").text();
            cont_obj.audio = $(this).find("source").attr("src").replace("../../","");
            cont_obj.answer = $(this).find(".imgs_cont input").val(); 
            array_to_export.push(cont_obj);
        });
    }
    sankore.setPreference("associer_sound", JSON.stringify(array_to_export));
}

//import
function importData(data){
    
    var tmp = 0;    
    for(var i in data){        
        var tmp_array = [];
        var container = $("<div class='cont'>").appendTo("body");
        var sub_container = $("<div class='sub_cont'>").appendTo(container);
        var imgs_container = $("<div class='imgs_cont imgs_answers_gray'>").appendTo(container);    
        
        $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
        var text = $("<div class='text_cont'>").appendTo(sub_container);
        var audio_block = $("<div class='audio_block'>").appendTo(text);
        $("<div class='play'>").appendTo(audio_block);
        $("<div class='replay'>").appendTo(audio_block);
        var source = $("<source/>").attr("src", "../../" + data[i].audio);
        var audio = $("<audio>").appendTo(audio_block);
        audio.append(source);
        $("<input type='hidden'/>").appendTo(audio_block);
        $("<div class='audio_desc'>" + data[i].text + "</div>").appendTo(text);
        $("<input type='hidden' value='" + data[i].answer + "'/>").appendTo(imgs_container);
        for(var j in data[i].answer){
            var tmp_letter = $("<div class='img_block' style='text-align: center;'>" + data[i].answer[j] + "</div>");
            tmp_array.push(tmp_letter);
        }                        
        tmp_array = shuffle(tmp_array);
        for(j = 0; j<tmp_array.length;j++)
            tmp_array[j].appendTo(imgs_container);
        imgs_container.sortable( {
            update: checkResult
        } );
    }
}

//example
function showExample(){
    
    var tmp_array = [];
    
    var container = $("<div class='cont'>").appendTo("body");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont imgs_answers_gray'>").appendTo(container);

    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>").appendTo(sub_container);
    var audio_block = $("<div class='audio_block'>").appendTo(text);
    $("<div class='play'>").appendTo(audio_block);
    $("<div class='replay'>").appendTo(audio_block);
    var source = $("<source/>").attr("src", "../../objects/beep.wav");
    var audio = $("<audio>").appendTo(audio_block);
    audio.append(source);
    $("<input type='hidden'/>").appendTo(audio_block);
    var audio_desc = $("<div class='audio_desc'>" + sankoreLang.short_desc + "</div>").appendTo(text);
    
    $("<input type='hidden' value='" + sankoreLang.example + "'/>").appendTo(imgs_container);
    
    for(var j in sankoreLang.example){
        var tmp_letter = $("<div class='img_block' style='text-align: center;'>" + sankoreLang.example[j] + "</div>");
        tmp_array.push(tmp_letter);
    } 
    
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++)
        tmp_array[i].appendTo(imgs_container);
    imgs_container.sortable( {
        update: checkResult
    } );
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
    var imgs_container = $("<div class='imgs_cont imgs_answers_gray'>").appendTo(container);
    
    var close = $("<div class='close_cont'>").appendTo(container);
    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>").appendTo(sub_container);
    text.attr("ondragenter", "return false;")
    .attr("ondragleave", "$(this).removeClass('gray'); return false;")
    .attr("ondragover", "$(this).addClass('gray'); return false;")
    .attr("ondrop", "$(this).removeClass('gray'); return onDropAudio(this,event);");
    var audio_block = $("<div class='audio_block'>").appendTo(text);
    $("<div class='play'>").appendTo(audio_block);
    $("<div class='replay'>").appendTo(audio_block);
    var source = $("<source/>").attr("src", "");
    var audio = $("<audio>").appendTo(audio_block);
    audio.append(source);
    $("<input type='hidden'/>").appendTo(audio_block);
    var audio_desc = $("<div class='audio_desc' contenteditable>" + sankoreLang.enter + "</div>").appendTo(text);
    
    $("<input type='hidden' value=''/>").appendTo(imgs_container);
    $("<div class='audio_answer' contenteditable>" + sankoreLang.example + "</div>").appendTo(imgs_container);
    container.insertBefore($(".add_block"));
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

//check result
function checkResult(event)
{
    var str = "";
    var right_str = $(event.target).find("input").val();
    $(event.target).find(".img_block").each(function(){
        str += $(this).text();
    });
    if(str == right_str)
        $(event.target).removeClass("imgs_answers_gray")
        .removeClass("imgs_answers_red")
        .addClass("imgs_answers_green");
    else
        $(event.target).removeClass("imgs_answers_gray")
        .removeClass("imgs_answers_green")
        .addClass("imgs_answers_red");
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
            var audio_block = $(obj).find(".audio_block");
            tmp = tmp.substr(1, tmp.length);            
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