var sankoreLang = {
    display: "Afficher", 
    edit: "Modifier", 
    first_desc: "Fruits", 
    second_desc: "Légumes",
    potatoes: "Pomme de terre",
    carrot: "Carotte",
    onion: "Oignon",
    apple: "Pomme",
    pear: "Poire",
    enter: "Saisir le nom de la catégorie ici ...",
    add: "Nouveau bloc",
    text: "Texte",
    wgt_name: "Catégoriser des textes",
    reload: "Recharger",
    slate: "ardoise",
    pad: "tablette",
    none: "aucun",
    help: "Aide",
    help_content: "<p><h2>Catégoriser des textes</h2></p>"+
    "<p><h3>Classer des étiquettes de mots en fonction de la dénomination de la catégorie.</h3></p>"+

    "<p>L’activité s’effectue par un glisser-déposer de l’étiquette de mots dans la catégorie correspondante. Une fois que toutes les étiquettes de mots sont classées dans la bonne catégorie, la zone se colore en vert.</p>"+

    "<p>Le bouton “Recharger” réinitialise les exercices.</p>"+

    "<p>Le bouton “Modifier” vous permet :</p>"+
    "<ul><li>de choisir le thème de l’interactivité : tablette, ardoise ou aucun (par défaut aucun),</li>"+
    "<li>de modifier l'exercice.</li></ul>"+

"<p>En mode édition :</p>"+
"<ul><li>modifier le nom de la catégorie (par exemple “fruits”, “légumes”, “mammifères”...) en cliquant dans le champ de texte,</li>"+
"<li>cliquez sur le gros “+” situé à gauche de la catégorie ce qui vous permet de rajouter des étiquettes de mots,</li>"+
"<li>entrez des mots dans ces étiquettes,</li>"+
"<li>ajoutez ensuite une catégorie (ou plusieurs) en cliquant sur le signe “+” situé à droite de la catégorie, complétez par son nom et ajoutez des étiquettes de mots,</li>"+
"<li>supprimez une étiquette de mots, en cliquant sur la croix située dans son coin supérieur droit,</li>"+
"<li>supprimez une catégorie, en cliquant sur le signe “-” situé à droite de celle-ci,</li>"+
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
        if(sankore.preference("categoriser_text","")){
            var data = jQuery.parseJSON(sankore.preference("categoriser_text",""));
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
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $("#parameters").css("display","none");
//                $(".add_block").remove();
                $(".cont").each(function(){
                    var container = $(this);
                    var tmp_i = 0;
                    var tmp_right = "";
                    var tmp_array = [];
                    
//                    container.find(".close_cont").remove();
                    container.find(".imgs_cont").each(function(){                        
                        $(this).find(".del_category").remove();
                        $(this).find(".add_img").remove();
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
                            var tmp_text = $(this).find(".text_cont");
                            tmp_text.removeAttr("contenteditable")
                            .css("margin", ($(this).height() - tmp_text.height())/2 + "px 0px");
                            tmp_array.push($(this));
                            $(this).remove();
                        });
                        $(this).droppable({
                            hoverClass: 'dropHere',
                            drop: function(event, ui) {
                                if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                    var tmp_ui = $(ui.draggable).parent();
                                    $(this).append($(ui.draggable));
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
                            appendTo: '#data'
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
                $(this).css("display", "none");
                $("#wgt_edit").css("display", "block");
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $("#parameters").css("display","block");
                $(".cont").each(function(){
                    var container = $(this);
                    
//                    $("<div class='close_cont'>").appendTo(container);
                    container.find(".imgs_cont").each(function(){
                        $("<button class='del_category'></button>").appendTo($(this));
                        $("<button class='add_category'></button>").appendTo($(this));
                        $(this).removeClass("red_cont")
                        .removeClass("green_cont")
                        .addClass("def_cont")
                        .droppable("destroy")
                        .find(".cat_desc").removeAttr("disabled");
                        var tmp_img_cont = $(this);
                        var tmp_mask = $(this).find("input[name='mask']").val();
                        container.find(".img_block").each(function(){
                            $(this).draggable("destroy")
                            .find(".text_cont").attr("contenteditable","true")
                            .removeAttr("style");
                            if($(this).find("input").val() == tmp_mask){
                                $("<div class='close_img'>").appendTo($(this));
                                $(this).appendTo(tmp_img_cont);
                            }
                        });
                        $("<div class='add_img'>").appendTo($(this));
                    });
                    container.find(".all_imgs").remove();
                });
                
                
//                $("<div class='add_block'>" + sankoreLang.add + "</div>").appendTo("#data");
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
            }
        }
    });
    
//    //add new block
//    $(".add_block").live("click", function(){
//        addContainer();
//    });
    
    //adding new img
    $(".add_img").live("click", function(){
        addText($(this).parent(), $(this));
    });
    
//    //deleting a block
//    $(".close_cont").live("click",function(){
//        $(this).parent().remove();
//        refreshBlockNumbers();
//    });
    
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
    if($("#wgt_edit").hasClass("selected")){
        $(".cont").each(function(){
            var cont_obj = new Object();
            cont_obj.mode = "edit";
            cont_obj.conts = [];
            $(this).find(".imgs_cont").each(function(){
                var img_cont = new Object();
                cont_obj.style = $("#style_select").find("option:selected").val();
                img_cont.mask = $(this).find("input[name='mask']").val();
                img_cont.count = $(this).find(".img_block").size();
                img_cont.text = $(this).find(".cat_desc").val();
                img_cont.imgs = [];
                $(this).find(".img_block").each(function(){
                    var img_obj = new Object();
                    img_obj.value = $(this).find("input").val();
                    img_obj.text = $(this).find(".text_cont").text();
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
                cont_obj.style = $("#style_select").find("option:selected").val();
                img_cont.mask = $(this).find("input[name='mask']").val();
                img_cont.count = $(this).find("input[name='count']").val();
                img_cont.text = $(this).find(".cat_desc").val();
                img_cont.imgs = [];
                $(this).find(".img_block").each(function(){
                    var img_obj = new Object();
                    img_obj.value = $(this).find("input").val();
                    img_obj.text = $(this).find(".text_cont").text();
                    img_cont.imgs.push(img_obj);
                });
                cont_obj.conts.push(img_cont);
            });
            cont_obj.all_imgs = [];
            $(this).find(".all_imgs .img_block").each(function(){
                var img = new Object();
                img.value = $(this).find("input").val();
                img.text = $(this).find(".text_cont").text();
                cont_obj.all_imgs.push(img);
            });
            array_to_export.push(cont_obj);
        });
    }
    
    if($(".cont").size() == 0){
        var cont_obj = new Object();
        cont_obj.style = $("#style_select").find("option:selected").val();
        cont_obj.tmp = "clear";
        array_to_export.push(cont_obj);
    }
    if(window.sankore)
        sankore.setPreference("categoriser_text", JSON.stringify(array_to_export));
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
            if(data[i].mode == "edit"){          
                var tmp_array = [];
                var container = $("<div class='cont'>").appendTo("#data");
                var sub_container = $("<div class='sub_cont'>").appendTo(container);                  
//                $("<div class='number_cont'>"+ (++tmp) +"</div>").appendTo(sub_container);
        
                for(var j in data[i].conts){
                    var imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);
    
                    $("<input type='hidden' name='mask' value='" + data[i].conts[j].mask + "'/>").appendTo(imgs_container);
                    $("<input type='hidden' name='count' value='" + data[i].conts[j].count + "'/>").appendTo(imgs_container);
                    var tmp_div = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container);
                    $("<input type='text' class='cat_desc' value='" + data[i].conts[j].text + "' disabled/>").appendTo(tmp_div);
                    for(var k in data[i].conts[j].imgs){
                        var block_img = $("<div class='img_block' style='text-align: center;'></div>");
                        $("<input type='hidden' value='" + data[i].conts[j].imgs[k].value + "'/>").appendTo(block_img);                    
                        $("<div class='text_cont'>" + data[i].conts[j].imgs[k].text + "</div>").appendTo(block_img);
                        tmp_array.push(block_img);
                    }
                
                    imgs_container.droppable({
                        hoverClass: 'dropHere',
                        drop: function(event, ui) {
                            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                var tmp_ui = $(ui.draggable).parent();
                                $(this).append($(ui.draggable));
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
                        appendTo: '#data'
                    });
                    tmp_array[j].appendTo(all_imgs);
                    var tmp_text = tmp_array[j].find(".text_cont");
                    tmp_text.css("margin", (tmp_array[j].height() - tmp_text.height())/2 + "px 0px");
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
            } else {
                container = $("<div class='cont'>").appendTo("#data");
                sub_container = $("<div class='sub_cont'>").appendTo(container);                  
//                $("<div class='number_cont'>" + (++tmp) + "</div>").appendTo(sub_container);
        
                for(j in data[i].conts){
                    var tmp_img_array = [];
                    imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);    
                    $("<input type='hidden' name='mask' value='" + data[i].conts[j].mask + "'/>").appendTo(imgs_container);
                    $("<input type='hidden' name='count' value='" + data[i].conts[j].count + "'/>").appendTo(imgs_container);
                    tmp_div = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container);
                    $("<input type='text' class='cat_desc' value='" + data[i].conts[j].text + "' disabled/>").appendTo(tmp_div);
                    for(k in data[i].conts[j].imgs){
                        block_img = $("<div class='img_block' style='text-align: center;'></div>");
                        $("<input type='hidden' value='" + data[i].conts[j].imgs[k].value + "'/>").appendTo(block_img);                   
                        $("<div class='text_cont'>" + data[i].conts[j].imgs[k].text + "</div>").appendTo(block_img);
                        tmp_img_array.push(block_img);
                    }
                
                    tmp_img_array = shuffle(tmp_img_array);
                    for(k in tmp_img_array){
                        tmp_img_array[k].draggable({
                            helper:'clone',
                            zIndex:100,
                            appendTo: '#data'
                        });
                        tmp_img_array[k].appendTo(imgs_container);
                        tmp_text = tmp_img_array[k].find(".text_cont");
                        tmp_text.css("margin", (tmp_img_array[k].height() - tmp_text.height())/2 + "px 0px");
                    }
                
                    imgs_container.droppable({
                        hoverClass: 'dropHere',
                        drop: function(event, ui) {
                            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                                var tmp_ui = $(ui.draggable).parent();
                                $(this).append($(ui.draggable));
                                checkCorrectness(tmp_ui);
                            }
                        }
                    });                    
                }
            
                all_imgs = $("<div class='all_imgs'>").appendTo(container); 
                var all_imgs_arr = [];
                for(j in data[i].all_imgs){            
                    block_img = $("<div class='img_block' style='text-align: center;'></div>");
                    $("<input type='hidden' value='" + data[i].all_imgs[j].value + "'/>").appendTo(block_img);                
                    $("<div class='text_cont'>" + data[i].all_imgs[j].text + "</div>").appendTo(block_img);
                    all_imgs_arr.push(block_img);
                } 
            
                all_imgs_arr = shuffle(all_imgs_arr);
                for(k in all_imgs_arr){
                    all_imgs_arr[k].draggable({
                        helper:'clone',
                        zIndex:100,
                        appendTo: '#data'
                    });
                    all_imgs_arr[k].appendTo(all_imgs);
                    tmp_text = all_imgs_arr[k].find(".text_cont");
                    tmp_text.css("margin", (all_imgs_arr[k].height() - tmp_text.height())/2 + "px 0px");
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
                checkCorrectness(all_imgs);
            }
        }
    }
}

//example
function showExample(){
    
    changeStyle("3");
    var tmp_array = [];
    
    var container = $("<div class='cont'>").appendTo("#data");
    var sub_container = $("<div class='sub_cont'>").appendTo(container);
    var imgs_container_one = $("<div class='imgs_cont def_cont'>").appendTo(container);
    var imgs_container_two = $("<div class='imgs_cont def_cont'>").appendTo(container);
    var all_imgs = $("<div class='all_imgs'>").appendTo(container);

//    var number = $("<div class='number_cont'>1</div>").appendTo(sub_container);
    
    $("<input type='hidden' name='mask' value='1'/>").appendTo(imgs_container_one);
    $("<input type='hidden' name='count' value='2'/>").appendTo(imgs_container_one);
    var tmp_div_one = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container_one);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.first_desc + "' disabled/>").appendTo(tmp_div_one);
    
    $("<input type='hidden' name='mask' value='2'/>").appendTo(imgs_container_two);
    $("<input type='hidden' name='count' value='3'/>").appendTo(imgs_container_two);
    var tmp_div_two = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container_two);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.second_desc + "' disabled/>").appendTo(tmp_div_two);
    
    var text1 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(text1);
    $("<div class='text_cont'>" + sankoreLang.potatoes + "</div>").appendTo(text1);
    var text2 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(text2);
    $("<div class='text_cont'>" + sankoreLang.apple + "</div>").appendTo(text2);
    var text3 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(text3);
    $("<div class='text_cont'>" + sankoreLang.carrot + "</div>").appendTo(text3);
    var text4 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='1'/>").appendTo(text4);
    $("<div class='text_cont'>" + sankoreLang.pear + "</div>").appendTo(text4);
    var text5 = $("<div class='img_block' style='text-align: center;'></div>");
    $("<input type='hidden' value='2'/>").appendTo(text5);
    $("<div class='text_cont'>" + sankoreLang.onion + "</div>").appendTo(text5);  
    
    tmp_array.push(text1, text2, text3, text4, text5);
    tmp_array = shuffle(tmp_array);
    for(var i = 0; i<tmp_array.length;i++){
        tmp_array[i].draggable({
            helper:'clone',
            zIndex:100,
            appendTo: '#data'
        });
        tmp_array[i].find(".text_cont").css("margin", "21px 0px");
        tmp_array[i].appendTo(all_imgs);
    }
    all_imgs.sortable();
    
    imgs_container_one.droppable({
        hoverClass: 'dropHere',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                var tmp_ui = $(ui.draggable).parent();
                $(this).append($(ui.draggable));
                checkCorrectness(tmp_ui);
            }
        }
    });
    
    imgs_container_two.droppable({
        hoverClass: 'dropHere',
        drop: function(event, ui) {
            if($(ui.draggable).parent().parent().html() == $(this).parent().html()){
                var tmp_ui = $(ui.draggable).parent();
                $(this).append($(ui.draggable));
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

//add text block
function addText(dest, source){
 
    var text_block = $("<div class='img_block' style='text-align: center;'>").insertBefore(source);
    $("<div class='close_img'>").appendTo(text_block);            
    $("<input type='hidden' value='" + dest.find("input[name='mask']").val() + "'/>").appendTo(text_block);
    $("<div class='text_cont' contenteditable='true'>" + sankoreLang.text + "</div>").appendTo(text_block);    
}

//function that allows to add new category
function addCategory(obj){
    var imgs_container = $("<div class='imgs_cont def_cont'>").insertAfter(obj);    
    $("<input type='hidden' name='mask' value='" + returnId() + "'/>").appendTo(imgs_container);   
    $("<input type='hidden' name='count' value=''/>").appendTo(imgs_container); 
    var tmp_div = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container);
    $("<input type='text' class='cat_desc' value='" + sankoreLang.enter + "'>").appendTo(tmp_div);  
    $("<button class='del_category'></button>").appendTo(imgs_container);
    $("<button class='add_category'></button>").appendTo(imgs_container);
    $("<div class='add_img'>").appendTo(imgs_container);
}

//add new container
//function addContainer(){
//    var container = $("<div class='cont'>");
//    var sub_container = $("<div class='sub_cont'>").appendTo(container);
//    var imgs_container = $("<div class='imgs_cont def_cont'>").appendTo(container);
//    
//    var close = $("<div class='close_cont'>").appendTo(container);
//    var number = $("<div class='number_cont'>"+ ($(".cont").size() + 1) +"</div>").appendTo(sub_container);
//    
//    $("<input type='hidden' name='mask' value='" + returnId() + "'/>").appendTo(imgs_container);
//    $("<input type='hidden' name='count' value=''/>").appendTo(imgs_container); 
//    var tmp_div = $("<div style='width: 100%; overflow: hidden;'>").appendTo(imgs_container);
//    $("<input type='text' class='cat_desc' value='" + sankoreLang.enter + "'/>").appendTo(tmp_div);    
//    $("<button class='del_category'></button>").appendTo(imgs_container);
//    $("<button class='add_category'></button>").appendTo(imgs_container);
//    $("<div class='add_img'>").appendTo(imgs_container);
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

//a func for checking when smth will drop
function checkOnDrop(dest){
    var tmp_count = dest.find("input[name='count']").val();
    var tmp_mask = dest.find("input[name='mask']").val();
    if(dest.find(".img_block").size() == tmp_count){
        var tmp_right = true;                    
        dest.find(".img_block").each(function(){
            if($(this).find("input").val() != tmp_mask)
                tmp_right = false;
        });          
        if(tmp_right)
            dest.removeClass("def_cont").removeClass("red_cont").addClass("green_cont");
        else
            dest.removeClass("def_cont").removeClass("green_cont").addClass("red_cont");
    } else 
        dest.removeClass("def_cont").removeClass("green_cont").addClass("red_cont");
}

//checking source on correctness
function checkCorrectness(source){
    if(!source.hasClass("all_imgs")){
        if(source.parent().find(".all_imgs").find(".img_block").size() == 0){
            source.parent().find(".imgs_cont").each(function(){
                checkOnDrop($(this))
            })
        } else {
            source.parent().find(".imgs_cont").each(function(){
                $(this).addClass("def_cont").removeClass("green_cont").removeClass("red_cont");
            })
        }
    } else {
        if(source.find(".img_block").size() > 0){
            source.parent().find(".imgs_cont").each(function(){
                $(this).addClass("def_cont").removeClass("green_cont").removeClass("red_cont");
            })
        } else {
            source.parent().find(".imgs_cont").each(function(){
                checkOnDrop($(this))
            })
        }
    }
}