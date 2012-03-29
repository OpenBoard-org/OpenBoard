var sankoreLang = {
    display: "Display", 
    edit: "Edit", 
    first_desc: "Odd numbers", 
    second_desc: "Even numbers", 
    enter: "Enter your category name here ...",
    add: "Add new block"
};

//main function
function start(){
    
    $("#display_text").text(sankoreLang.display);
    $("#edit_text").text(sankoreLang.edit);
    
    if(window.sankore){
        if(sankore.preference("categoriser_images","")){
            var data = jQuery.parseJSON(sankore.preference("categoriser_images",""));
            importData(data);
        } else {
            showExample();
        }
    } 
    else 
        showExample();

    if (window.widget) {
        window.widget.onleave = function(){
            exportData();
        }
    }
    
    $("#display, #edit").click(function(event){
        if(this.id == "display"){
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#display_img").removeClass("red_point").addClass("green_point");
                $("#edit_img").removeClass("green_point").addClass("red_point");
                $("#edit").removeClass("selected");
                sankore.enableDropOnWidget(false);
                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_i = 0;
                    var tmp_right = "";
                    var tmp_array = [];
                    
                    container.find(".close_cont").remove();
                    container.find(".imgs_cont").each(function(){                        
                        $(this).find(".del_category").remove();
                        $(this).find(".add_category").remove();                        
                        $(this).removeAttr("ondragenter")
                        .removeAttr("ondragleave")
                        .removeAttr("ondragover")
                        .removeAttr("ondrop")                        
                        .find(".cat_desc").attr("disabled","disabled");
                        var tmp_count = $(this).find(".img_block").size();
                        $(this).find("input[name='count']").val(tmp_count);
                        $(this).find(".img_block").each(function(){
                            $(this).find(".close_img").remove();
                            tmp_array.push($(this));
                            $(this).remove();
                        });
                        $(this).droppable({
                            hoverClass: 'dropHere',
                            drop: function(event, ui) {
                                if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                    var tmp_ui = $(ui.draggable).parent();
                                    checkOnDrop($(this), $(ui.draggable));
                                    checkCorrectness(tmp_ui);
                                }
                            }
                        });
                        $(this).removeAttr("style");
                    });
                    
                    var all_imgs = $("<div class='all_imgs'>").appendTo(container);
                    
                    tmp_array = shuffle(tmp_array);
                    for(var i = 0; i<tmp_array.length;i++){
                        tmp_array[i].draggable({
                            helper:'clone',
                            zIndex:100,
                            appendTo: 'body'
                        });
                        tmp_array[i].appendTo(all_imgs);
                    }
                    
                    all_imgs.droppable({
                        hoverClass: 'dropBack',
                        drop: function(event, ui) {
                            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                if(this != $(ui.draggable).parent()[0]){
                                    var tmp_ui = $(ui.draggable).parent();                    
                                    $(this).append($(ui.draggable));
                                    checkCorrectness(tmp_ui);
                                }
                            }
                        }
                    });
                });
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#edit_img").removeClass("red_point").addClass("green_point");
                $("#display_img").removeClass("green_point").addClass("red_point");
                $("#display").removeClass("selected");
                sankore.enableDropOnWidget(true);
                $(".cont").each(function(){
                    var container = $(this);
                    
                    $("<div class='close_cont'>").appendTo(container);
                    container.find(".imgs_cont").each(function(){
                        $("<button class='del_category'>-</button>").appendTo($(this));
                        $("<button class='add_category'>+</button>").appendTo($(this));
                        $(this).attr("ondragenter", "return false;")
                        .attr("ondragleave", "$(this).css(\"background-color\",\"#e6f6ff\"); return false;")
                        .attr("ondragover", "$(this).css(\"background-color\",\"#c3e9ff\"); return false;")
                        .attr("ondrop", "$(this).css(\"background-color\",\"#e6f6ff\"); return onDropTarget(this,event);")
                        .removeClass("red_cont")
                        .removeClass("green_cont")
                        .addClass("def_cont")
                        .droppable("destroy")
                        .find(".cat_desc").removeAttr("disabled");
                        var tmp_img_cont = $(this);
                        var tmp_mask = $(this).find("input[name='mask']").val();
                        container.find(".img_block").each(function(){
                            if($(this).find("input").val() == tmp_mask){
                                $("<div class='close_img'>").appendTo($(this));
                                $(this).appendTo(tmp_img_cont);
                            }
                        });
                    });
                    container.find(".all_imgs").remove();
                });
                
                
                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("body");
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
        $(this).parent().remove();        
    });
    
    //adding new category
    $(".add_category").live("click",function(){
        addCategory($(this).parent());
    });
    
    //deleting the category
    $(".del_category").live("click",function(){
        if($(this).parent().parent().find(".imgs_cont").size() == 1){
            $(this).parent().find(".img_block").remove();
            $(this).parent().find(".cat_desc").val(sankoreLang.enter);
        } else {
            $(this).parent().remove();
        }
    });
}

//export
function exportData(){
    var array_to_export = [];
    if($("#edit").hasClass("selected")){
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.mode = "edit";
            cont_obj.conts = [];
            $(this).find(".imgs_cont").each(function(){
                var img_cont = new Object();
                img_cont.mask = $(this).find("input[name='mask']").val();
                img_cont.count = $(this).find(".img_block").size();
                img_cont.text = $(this).find(".cat_desc").val();
                img_cont.imgs = [];
                $(this).find(".img_block").each(function(){
                    var img_obj = new Object();
                    img_obj.value = $(this).find("input").val();
                    img_obj.link = $(this).find("img").attr("src");
                    img_obj.ht = $(this).find("img").height();
                    img_obj.wd = $(this).find("img").width();
                    img_cont.imgs.push(img_obj);
                });
                cont_obj.conts.push(img_cont);
            });
            array_to_export.push(cont_obj);
        });
    } else {
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.mode = "display";
            cont_obj.conts = [];
            $(this).find(".imgs_cont").each(function(){
                var img_cont = new Object();
                img_cont.mask = $(this).find("input[name='mask']").val();
                img_cont.count = $(this).find("input[name='count']").val();
                img_cont.text = $(this).find(".cat_desc").val();
                img_cont.imgs = [];
                $(this).find(".img_block").each(function(){
                    var img_obj = new Object();
                    img_obj.value = $(this).find("input").val();
                    img_obj.link = $(this).find("img").attr("src");
                    img_obj.ht = $(this).find("img").height();
                    img_obj.wd = $(this).find("img").width();
                    img_cont.imgs.push(img_obj);
                });
                cont_obj.conts.push(img_cont);
            });
            cont_obj.all_imgs = [];
            $(this).find(".all_imgs .img_block").each(function(){
                var img = new Object();
                img.value = $(this).find("input").val();
                img.link = $(this).find("img").attr("src");
                img.ht = $(this).find("img").height();
                img.wd = $(this).find("img").width();
                cont_obj.all_imgs.push(img);
            });
            array_to_export.push(cont_obj);
        });
    }
    sankore.setPreference("categoriser_images", JSON.stringify(array_to_export));
}

//import
function importData(data){
    
    var tmp = 0;    
    for(var i in data){
        if(data[i].mode == "edit"){          
            var tmp_array = [];
            var container = $("<div class='cont'>");
            var sub_container = $("<div class='sub_cont'>").appendTo(container);                  
            $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
        
            for(var j in data[i].conts){
                var imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);
    
                $("<input type='hidden' name='mask' value='" + data[i].conts[j].mask + "'/>").appendTo(imgs_container);
                $("<input type='hidden' name='count' value='" + data[i].conts[j].count + "'/>").appendTo(imgs_container);
                var tmp_div = $("<div style='width: 100%'>").appendTo(imgs_container);
                $("<input type='text' class='cat_desc' value='" + data[i].conts[j].text + "' disabled/>").appendTo(tmp_div);
                for(var k in data[i].conts[j].imgs){
                    var block_img = $("<div class='img_block' style='text-align: center;'></div>");
                    $("<input type='hidden' value='" + data[i].conts[j].imgs[k].value + "'/>").appendTo(block_img);
                    var img = $("<img src=\"" + data[i].conts[j].imgs[k].link + "\" style=\"display: inline;\"/>").appendTo(block_img);
                    if(data[i].conts[j].imgs[k].ht >= data[i].conts[j].imgs[k].wd)
                        img.attr("height", "120");
                    else{
                        img.attr("width","120");
                        img.css("margin",(120 - data[i].conts[j].imgs[k].ht)/2 + "px 0");
                    }
                    tmp_array.push(block_img);
                }
                
                imgs_container.droppable({
                    hoverClass: 'dropHere',
                    drop: function(event, ui) {
                        if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                            var tmp_ui = $(ui.draggable).parent();
                            checkOnDrop($(this), $(ui.draggable));
                            checkCorrectness(tmp_ui);
                        }
                    }
                });                                
            }
            
            var all_imgs = $("<div class='all_imgs'>").appendTo(container); 
            
            tmp_array = shuffle(tmp_array);
            for(j in tmp_array){
                tmp_array[j].draggable({
                    helper:'clone',
                    zIndex:100,
                    appendTo: 'body'
                });
                tmp_array[j].appendTo(all_imgs);
            }
            
            all_imgs.sortable();
            
            all_imgs.droppable({
                hoverClass: 'dropBack',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if(this != $(ui.draggable).parent()[0]){
                            var tmp_ui = $(ui.draggable).parent();                    
                            $(this).append($(ui.draggable));
                            checkCorrectness(tmp_ui);
                        }
                    }
                }
            });
            
            container.appendTo("body");
        } else {
            container = $("<div class='cont'>");
            sub_container = $("<div class='sub_cont'>").appendTo(container);                  
            $("<div class='number_cont'>" + (++tmp) + "</div>").appendTo(sub_container);
        
            for(j in data[i].conts){
                var tmp_img_array = [];
                imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);    
                $("<input type='hidden' name='mask' value='" + data[i].conts[j].mask + "'/>").appendTo(imgs_container);
                $("<input type='hidden' name='count' value='" + data[i].conts[j].count + "'/>").appendTo(imgs_container);
                tmp_div = $("<div style='width: 100%'>").appendTo(imgs_container);
                $("<input type='text' class='cat_desc' value='" + data[i].conts[j].text + "' disabled/>").appendTo(tmp_div);
                for(k in data[i].conts[j].imgs){
                    block_img = $("<div class='img_block' style='text-align: center;'></div>");
                    $("<input type='hidden' value='" + data[i].conts[j].imgs[k].value + "'/>").appendTo(block_img);
                    img = $("<img src=\"" + data[i].conts[j].imgs[k].link + "\" style=\"display: inline;\"/>").appendTo(block_img);
                    if(data[i].conts[j].imgs[k].ht >= data[i].conts[j].imgs[k].wd)
                        img.attr("height", "120");
                    else{
                        img.attr("width","120");
                        img.css("margin",(120 - data[i].conts[j].imgs[k].ht)/2 + "px 0");
                    }
                    tmp_img_array.push(block_img);
                }
                
                tmp_img_array = shuffle(tmp_img_array);
                for(k in tmp_img_array){
                    tmp_img_array[k].draggable({
                        helper:'clone',
                        zIndex:100,
                        appendTo: 'body'
                    });
                    tmp_img_array[k].appendTo(imgs_container);
                }
                
                imgs_container.droppable({
                    hoverClass: 'dropHere',
                    drop: function(event, ui) {
                        if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                            var tmp_ui = $(ui.draggable).parent();
                            checkOnDrop($(this), $(ui.draggable));
                            checkCorrectness(tmp_ui);
                        }
                    }
                });        
                checkCorrectness(imgs_container);
            }
            
            all_imgs = $("<div class='all_imgs'>").appendTo(container); 
            var all_imgs_arr = [];
            for(j in data[i].all_imgs){            
                block_img = $("<div class='img_block' style='text-align: center;'></div>");
                $("<input type='hidden' value='" + data[i].all_imgs[j].value + "'/>").appendTo(block_img);
                $("<img src=\"" + data[i].all_imgs[j].link + "\" style=\"display: inline;\" height=\"" + data[i].all_imgs[j].ht + "\"/>").appendTo(block_img);
                all_imgs_arr.push(block_img);
            } 
            
            all_imgs_arr = shuffle(all_imgs_arr);
            for(k in all_imgs_arr){
                all_imgs_arr[k].draggable({
                    helper:'clone',
                    zIndex:100,
                    appendTo: 'body'
                });
                all_imgs_arr[k].appendTo(all_imgs);
            }
            
            all_imgs.sortable();
            
            all_imgs.droppable({
                hoverClass: 'dropBack',
                drop: function(event, ui) {
                    if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                        if(this != $(ui.draggable).parent()[0]){
                            var tmp_ui = $(ui.draggable).parent();                    
                            $(this).append($(ui.draggable));
                            checkCorrectness(tmp_ui);
                        }
                    }
                }
            });            
            container.appendTo("body");
        }
    }
}

//example
function showExample(){
    
    var tmp_array = [];
    
    var container = $("<div class='cont'>").appendTo("body");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container_one = $("<div class='imgs_cont def_cont'>").appendTo(container);
    var imgs_container_two = $("<div class='imgs_cont def_cont'>").appendTo(container);
    var all_imgs = $("<div class='all_imgs'>").appendTo(container);

    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    
    $("<input type='hidden' name='mask' value='1'/>").appendTo(imgs_container_one);
    $("<input type='hidden' name='count' value='2'/>").appendTo(imgs_container_one);
    var tmp_div_one = $("<div style='width: 100%'>").appendTo(imgs_container_one);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.first_desc + "' disabled/>").appendTo(tmp_div_one);
    
    $("<input type='hidden' name='mask' value='2'/>").appendTo(imgs_container_two);
    $("<input type='hidden' name='count' value='3'/>").appendTo(imgs_container_two);
    var tmp_div_two = $("<div style='width: 100%'>").appendTo(imgs_container_two);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.second_desc + "' disabled/>").appendTo(tmp_div_two);
    
    var img1 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(img1);
    $("<img src=\"objects/0.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img1);
    var img2 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(img2);
    $("<img src=\"objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img2);
    var img3 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(img3);
    $("<img src=\"objects/2.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img3);
    var img4 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(img4);
    $("<img src=\"objects/3.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img4);
    var img5 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(img5);
    $("<img src=\"objects/4.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img5);  
    
    tmp_array.push(img1, img2, img3, img4, img5);
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++){
        tmp_array[i].draggable({
            helper:'clone',
            zIndex:100,
            appendTo: 'body'
        });
        tmp_array[i].appendTo(all_imgs);
    }
    all_imgs.sortable();
    
    imgs_container_one.droppable({
        hoverClass: 'dropHere',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                var tmp_ui = $(ui.draggable).parent();
                checkOnDrop($(this), $(ui.draggable));
                checkCorrectness(tmp_ui);
            }
        }
    });
    
    imgs_container_two.droppable({
        hoverClass: 'dropHere',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                var tmp_ui = $(ui.draggable).parent();
                checkOnDrop($(this), $(ui.draggable));
                checkCorrectness(tmp_ui);
            }
        }
    });
    
    all_imgs.droppable({
        hoverClass: 'dropBack',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                if(this != $(ui.draggable).parent()[0]){
                    var tmp_ui = $(ui.draggable).parent();                    
                    $(this).append($(ui.draggable));
                    checkCorrectness(tmp_ui);
                }
            }
        }
    });
}

//function that allows to add new category
function addCategory(obj){
    var imgs_container = $("<div class='imgs_cont def_cont'>").insertAfter(obj);    
    $("<input type='hidden' name='mask' value='" + returnId() + "'/>").appendTo(imgs_container);   
    $("<input type='hidden' name='count' value=''/>").appendTo(imgs_container); 
    var tmp_div = $("<div style='width: 100%'>").appendTo(imgs_container);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.enter + "'>").appendTo(tmp_div);  
    $("<button class='del_category'>-</button>").appendTo(imgs_container);
    $("<button class='add_category'>+</button>").appendTo(imgs_container);
    imgs_container.attr("ondragenter", "return false;")
    .attr("ondragleave", "$(this).css(\"background-color\",\"#e6f6ff\"); return false;")
    .attr("ondragover", "$(this).css(\"background-color\",\"#c3e9ff\"); return false;")
    .attr("ondrop", "$(this).css(\"background-color\",\"#e6f6ff\"); return onDropTarget(this,event);");
}

//add new container
function addContainer(){
    var container = $("<div class='cont'>");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);
    
    var close = $("<div class='close_cont'>").appendTo(container);
    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
    
    $("<input type='hidden' name='mask' value='" + returnId() + "'/>").appendTo(imgs_container);
    $("<input type='hidden' name='count' value=''/>").appendTo(imgs_container); 
    var tmp_div = $("<div style='width: 100%'>").appendTo(imgs_container);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.enter + "'/>").appendTo(tmp_div);    
    $("<button class='del_category'>-</button>").appendTo(imgs_container);
    $("<button class='add_category'>+</button>").appendTo(imgs_container);
    imgs_container.attr("ondragenter", "return false;")
    .attr("ondragleave", "$(this).css(\"background-color\",\"#e6f6ff\"); return false;")
    .attr("ondragover", "$(this).css(\"background-color\",\"#c3e9ff\"); return false;")
    .attr("ondrop", "$(this).css(\"background-color\",\"#e6f6ff\"); return onDropTarget(this,event);");
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

//return id
function returnId(){
    var tmp = Math.random().toString();
    return tmp.substr(2);
}

//a func for checking when smth will drop
function checkOnDrop(dest, source){
    dest.append(source); 
    var tmp_count = dest.find("input[name='count']").val();
    var tmp_mask = dest.find("input[name='mask']").val();
    if(dest.find(".img_block").size() == tmp_count){
        var tmp_right = true;                    
        dest.find(".img_block").each(function(){
            if($(this).find("input").val() != tmp_mask)
                tmp_right = false;
        });          
        if(tmp_right)
            dest.removeClass("def_cont")
            .removeClass("red_cont")
            .addClass("green_cont");
        else
            dest.removeClass("def_cont")
            .removeClass("green_cont")
            .addClass("red_cont");
    } else 
        dest.removeClass("def_cont")
        .removeClass("green_cont")
        .addClass("red_cont");
}

//checking source on correctness
function checkCorrectness(source){
    if(!source.hasClass("all_imgs")){
        var tmp_count = source.find("input[name='count']").val();
        var tmp_mask = source.find("input[name='mask']").val();
        if(source.find(".img_block").size() == tmp_count){
            var tmp_right = true;                    
            source.find(".img_block").each(function(){
                if($(this).find("input").val() != tmp_mask)
                    tmp_right = false;
            });
                    
            if(tmp_right)
                source.removeClass("def_cont")
                .removeClass("red_cont")
                .addClass("green_cont");
            else
                source.removeClass("def_cont")
                .removeClass("green_cont")
                .addClass("red_cont");
        } else 
            source.removeClass("def_cont")
            .removeClass("green_cont")
            .addClass("red_cont");
    }
}

function onDropTarget(obj, event) {
    if (event.dataTransfer) {
        var format = "text/plain";
        var textData = event.dataTransfer.getData(format);
        if (!textData) {
            alert(":(");
        }
        textData = stringToXML(textData);
        if(textData.getElementsByTagName("ready")[0].firstChild.textContent == "true"){
            var tmp = textData.getElementsByTagName("path")[0].firstChild.textContent;
            tmp = tmp.substr(1, tmp.length);       
            var img_block = $("<div class='img_block' style='text-align: center;'>");
            $("<div class='close_img'>").appendTo(img_block);            
            $("<input type='hidden' value='" + $(obj).find("input[name='mask']").val() + "'/>").appendTo(img_block);
            var tmp_img = $("<img style='display: inline;'/>").attr("src", tmp).appendTo(img_block);
            img_block.draggable({
                helper:'clone',
                zIndex:100,
                appendTo: 'body'
            });
            $(obj).append(img_block);
            
            if(tmp_img.height() == 0){
                var tmp_id = setInterval(function(){
                    if(tmp_img.height() != 0){
                        if(tmp_img.height() >= tmp_img.width())
                            tmp_img.attr("height", "120");
                        else{
                            tmp_img.attr("width","120");
                            tmp_img.css("margin",(120 - tmp_img.height())/2 + "px 0");
                        }
                        clearInterval(tmp_id);
                    }
                }, 10);
            } else {
                if(tmp_img.height() >= tmp_img.width())
                    tmp_img.attr("height", "120");
                else{
                    tmp_img.attr("width","120");
                    tmp_img.css("margin",(120 - tmp_img.height())/2 + "px 0");
                }
            }  
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