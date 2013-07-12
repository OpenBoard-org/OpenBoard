
var sankoreLang = {
    display: "Afficher", 
    edit: "Modifier", 
    short_desc: "Placez les images dans l'ordre croissant.", 
    add: "Nouveau bloc",
    enter: "Saisir votre consigne ici ...",
    wgt_name: "Ordonner des images",
    reload: "Recharger",
    slate: "ardoise",
    pad: "tablette",
    none: "aucun",
    help: "Aide",
    help_content: "<p><h2>Ordonner des images</h2></p>"+
    "<p><h3>Classer des images dans l’ordre demandé.</h3></p>"+

    "<p>La consigne indique dans quel ordre les images doivent être classées. Le classement des images s’effectue par un glisser-déposer. Une fois que toutes les images sont correctement classées, la zone se colore en vert.</p>"+

    "<p>Le bouton “Recharger” réinitialise les exercices.</p>"+

    "<p>Le bouton “Modifier” vous permet :</p>"+
    "<ul><li>de choisir le thème de l’interactivité : tablette, ardoise ou aucun (par défaut aucun),</li>"+
    "<li>de modifier l'exercice.</li></ul>"+

    "<p>Dans le mode édition :</p>"+
    "<ul><li>modifiez la consigne en cliquant sur le champ de texte,</li>"+
    "<li>insérez des images dans les zones de dépôt par glisser-déposer des images à partir de votre bibliothèque,</li>"+
    "<li>modifiez éventuellement l’ordre des images par un glisser-déposer de l’image concernée pour la mettre au bon endroit dans la suite,</li>"+
    "<li>ajoutez une zone image, en cliquant sur le gros “+” en bas,</li>"+
    "<li>supprimez une zone image, en cliquant sur la croix située dans son coin supérieur droit,</li>"+
    "<li>changez limage en cliquant sur l’icône située au milieu à droite de l’image,</li>"+ 
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
        if(sankore.preference("odr_des_imgs","")){
            var data = jQuery.parseJSON(sankore.preference("odr_des_imgs",""));
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
                $("#parameters").css("display","none");
//                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_i = 0;
                    var tmp_right = "";
                    var tmp_array = [];
                    
                    container.find(".text_cont").removeAttr("contenteditable");
                    container.find(".add_img").remove();
//                    container.find(".close_cont").remove();
                    container.find(".img_block").each(function(){
                        if($(this).find("img").attr("src") != "img/drop_img.png"){
                            $(this).find(".close_img").remove();
                            $(this).find(".clear_img").remove();
                            $(this).find(".numb_img").remove();
                            $(this).removeAttr("ondragenter")
                            .removeAttr("ondragleave")
                            .removeAttr("ondragover")
                            .removeAttr("ondrop");
                            $(this).find("input").val(++tmp_i)
                            tmp_right += tmp_i + "*";
                        } else 
                            $(this).remove();
                    });
                    container.find(".imgs_cont>input").val(tmp_right);
                    
                    container.find(".img_block").each(function(){
                        $(this).css("float","");
                        tmp_array.push($(this));
                    });                    
                    tmp_array = shuffle(tmp_array);
                    for(var i = 0; i<tmp_array.length;i++)
                        tmp_array[i].appendTo(container.find(".imgs_cont"));
                    container.find(".imgs_cont").sortable( {
                        update: checkResult
                    } );
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
                    container.find(".text_cont").attr("contenteditable","true");
                    //container.find(".imgs_cont").sortable("destroy");
                    container.find(".imgs_cont").css("background-color", "");
                    
                    var add_img = $("<div class='add_img'>");
                    container.find(".img_block").each(function(){
                        $(this).attr("ondragenter", "return false;")
                        .attr("ondragleave", "$(this).css(\"background-color\",\"\"); return false;")
                        .attr("ondragover", "$(this).css(\"background-color\",\"#ccc\"); return false;")
                        .attr("ondrop", "$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);")
                        //.css("float","left");
                        $("<div class='close_img'>").appendTo($(this));
                        $("<div class='clear_img'>").appendTo($(this));
                        $("<div class='numb_img'>" + $(this).find("input").val() + "</div>").appendTo($(this));
                    });
                    rightOrder(container.find(".imgs_cont"));
                    container.find(".imgs_cont").append(add_img)
                });                
                
//                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("#data");
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
            }
        }
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
        if(tmp_obj.find(".img_block").size() > 0){
            refreshImgNumbers(tmp_obj);
        }
        
    });
    
    //cleaning an image
    $(".clear_img").live("click",function(){
        //$(this).parent().find(".fill_img").remove();
        $(this).parent().find("img").attr("src","img/drop_img.png");
    });
}

//export
function exportData(){
    var array_to_export = [];
    if($("#edit").hasClass("selected")){
        $(".cont").each(function(){
            var container = $(this);
            var tmp_right = "";
            var tmp_i = 0;
            container.find(".img_block").each(function(){
                if($(this).html().match(/<img/)){
                    $(this).find("input").val(++tmp_i)
                    tmp_right += tmp_i + "*";
                }
            });
            container.find(".imgs_cont>input").val(tmp_right);
        });
    }
    $(".cont").each(function(){
        var cont_obj = new Object();
        cont_obj.style = $("#style_select").find("option:selected").val();
        cont_obj.text = $(this).find(".text_cont").text();
        cont_obj.right = $(this).find(".imgs_cont>input").val();
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
    
    if($(".cont").size() == 0){
        var cont_obj = new Object();
        cont_obj.style = $("#style_select").find("option:selected").val();
        cont_obj.tmp = "clear";
        array_to_export.push(cont_obj);
    }
    if(window.sankore)
        sankore.setPreference("odr_des_imgs", JSON.stringify(array_to_export));
    if($("#wgt_display").hasClass("selected")){
        if(window.sankore)
            sankore.setPreference("odr_des_imgs_state", "display");
    }
    else{
        if(window.sankore)
            sankore.setPreference("odr_des_imgs_state", "edit");
    }
}

//import
function importData(data){
        
    for(var i in data){
        if(data[i].tmp){
            changeStyle(data[i].style);
            $("#style_select").val(data[i].style);
        }
        else {
            if(i == 0){
                changeStyle(data[i].style);
                $("#style_select").val(data[i].style);
            }
            var tmp_array = [];
            var container = $("<div class='cont'>");
            var sub_container = $("<div class='sub_cont'>").appendTo(container);
            var imgs_container = $("<div class='imgs_cont'>").appendTo(container);    
        
//            var number = $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
            var text = $("<div class='text_cont'>" + data[i].text + "</div>").appendTo(sub_container);
    
            $("<input type='hidden' value='" + data[i].right + "'/>").appendTo(imgs_container);
        
            for(var j in data[i].imgs){
                var img_block = $("<div class='img_block' style='text-align: center;'>");
                var img = $("<img src='../../" + data[i].imgs[j].link + "' style='display: inline;'>");
                img.height(data[i].imgs[j].ht);
                if((120 - data[i].imgs[j].ht) > 0)
                    img.css("margin",(120 - data[i].imgs[j].ht)/2 + "px 0");
                var hidden_input = $("<input type='hidden'>").val(data[i].imgs[j].value);
                img_block.append(hidden_input).append(img);
                tmp_array.push(img_block);
            }
            if(sankore.preference("odr_des_imgs_state","")){
                if(sankore.preference("odr_des_imgs_state","") == "edit")
                    tmp_array = shuffle(tmp_array);
            } else 
                tmp_array = shuffle(tmp_array);
            
            for(j = 0; j<tmp_array.length;j++)
                tmp_array[j].appendTo(imgs_container);
            imgs_container.sortable().bind('sortupdate', function(event, ui) {
                checkResult(event);
            }); 
            container.appendTo("#data"); 
            imgs_container.trigger("sortupdate")                          
        }        
    }
}

//example
function showExample(){
    
    changeStyle("3");
    var tmp_array = [];
    
    var container = $("<div class='cont'>");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);

//    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    var text = $("<div class='text_cont'>" + sankoreLang.short_desc + "</div>").appendTo(sub_container);
    
    $("<input type='hidden' value='1*2*3*4*5*'/>").appendTo(imgs_container);
    
    var img1 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(img1);
    $("<img src=\"../../objects/0.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img1);
    var img2 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(img2);
    $("<img src=\"../../objects/1.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img2);
    var img3 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='3'/>").appendTo(img3);
    $("<img src=\"../../objects/2.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img3);
    var img4 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='4'/>").appendTo(img4);
    $("<img src=\"../../objects/3.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img4);
    var img5 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='5'/>").appendTo(img5);
    $("<img src=\"../../objects/4.gif\" style=\"display: inline;\" height=\"120\"/>").appendTo(img5);  
    
    tmp_array.push(img1, img2, img3, img4, img5);
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++)
        tmp_array[i].appendTo(imgs_container);
    imgs_container.sortable().bind('sortupdate', function(event, ui) {
        checkResult(event);
    });

    container.appendTo("#data");
}

//check result
function checkResult(event)
{
    if($("#wgt_display").hasClass("selected")){
        var str = "";
        var right_str = $(event.target).find("input").val();
        $(event.target).find(".img_block").each(function(){
            str += $(this).find("input").val() + "*";
        });
        if(str == right_str)
            $(event.target).css("background-color","#9f9");
        else
            $(event.target).css("background-color","");
    } else {
        refreshImgNumbers($(event.target));
    }
}

//add new container
//function addContainer(){
//    var container = $("<div class='cont'>");
//    var sub_container = $("<div class='sub_cont'>").appendTo(container);
//    var imgs_container = $("<div class='imgs_cont'>").appendTo(container);
//    
//    var close = $("<div class='close_cont'>").appendTo(container);
//    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
//    var text = $("<div class='text_cont' contenteditable>" + sankoreLang.enter + "</div>").appendTo(sub_container);
//    
//    $("<input type='hidden' value='1*2*3*4*5*'/>").appendTo(imgs_container);
//    var add_img = $("<div class='add_img'>").appendTo(imgs_container);
//    container.insertBefore($(".add_block"));
//}

//add new img block
function addImgBlock(dest){
    var img_block = $("<div class='img_block' ondragenter='return false;' ondragleave='$(this).css(\"background-color\",\"\"); return false;' ondragover='$(this).css(\"background-color\",\"#ccc\"); return false;' ondrop='$(this).css(\"background-color\",\"\"); return onDropTarget(this,event);' style='text-align: center;'></div>").insertBefore(dest);
    var tmp_counter = dest.parent().find(".img_block").size();
    $("<div class='close_img'>").appendTo(img_block);
    $("<div class='clear_img'>").appendTo(img_block);
    $("<div class='numb_img'>" + tmp_counter + "</div>").appendTo(img_block);
    $("<input type='hidden' value='" + tmp_counter + "'/>").appendTo(img_block);
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

//regulation the images in right order
function rightOrder(source){
    var tmp_arr = [];
    var tmp_var;
    source.find(".img_block").each(function(){
        tmp_arr.push($(this));
        $(this).remove();
    });
    for(var i = 0; i < tmp_arr.length; i++)
        for(var j = 0; j < tmp_arr.length - 1; j++){
            if(tmp_arr[j].find("input").val() > tmp_arr[j+1].find("input").val()){
                tmp_var = tmp_arr[j];
                tmp_arr[j] = tmp_arr[j+1];
                tmp_arr[j+1] = tmp_var;
            }
        }
    for(i in tmp_arr)
        source.append(tmp_arr[i]);
}

//refresh the numbers of the images
function refreshImgNumbers(source){
    var tmp = 1;
    source.find(".img_block").each(function(){
        $(this).find("input").val(tmp);
        $(this).find(".numb_img").text(tmp);
        tmp++;
    });
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