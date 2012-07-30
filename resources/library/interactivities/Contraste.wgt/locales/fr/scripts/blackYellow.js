/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

var sankoreLang = {
    display: "Afficher", 
    edit: "Modifier", 
    add: "Ajouter", 
    enter_data: "Saisir la donnée:", 
    enter_result: "Saisir le résultat:", 
    ok: "Ok", 
    cancel: "Annuler",
    wgt_name: "Contraste",
    reload: "Recharger",
    slate: "ardoise",
    pad: "tablette",
    none: "aucun",
    help: "aide",
    help_content: "Ceci est un exemple de contenu de l'aide ..."
};

function init(){

    //variables
    var toggleFlag = false; // detects toggling in toggle button
    var endFlag = false; // ending of toggling in toggle button
    var addToggleStart = false; // detects toggling in add button
    var addToggleEnd = false; // ending of toggling in add button
    var shadowOver = false;
    var mode = false;
    var popupFlag = false
    var flagForSelect = false;
    var dragElement = null; //the element that must be dragging
    var lang = ""; //locale language
    var coords = {
        left:0,
        top:0
    }

    var resizeObj = {
        y:false,
        x:false,
        width:0,
        height:0
    }
    
    var opacityChanged = false;
    
    $("#wgt_display").text(sankoreLang.display);
    $("#wgt_edit").text(sankoreLang.edit);
    $("#wgt_add").text(sankoreLang.add);
    $("#wgt_name").text(sankoreLang.wgt_name);
    $("#wgt_reload").text(sankoreLang.reload);
    $("#wgt_help").text(sankoreLang.help);
    $("#help").html(sankoreLang.help_content);
    $(".style_select option[value='1']").text(sankoreLang.slate);
    $(".style_select option[value='2']").text(sankoreLang.pad);
    $(".style_select option[value='3']").text(sankoreLang.none);
    
    if(window.sankore){
        if(sankore.preference("blackYellowData","")){
            var importArray = jQuery.parseJSON(sankore.preference("blackYellowData",""));
            for(var i in importArray){
                var tmpReadyTask = $("<div class='readyTask'>");
                var exprContainer = $("<div class='taskContainer' style='color: yellow;'>" + importArray[i].data1 + "</div>").appendTo(tmpReadyTask);
                var resContainer = $("<div class='taskContainer' style='color: black;'>"+ importArray[i].data2 + "</div>").appendTo(tmpReadyTask);
                tmpReadyTask.width(importArray[i].width)
                .height(importArray[i].height)
                .css("position","absolute")
                .css("top",importArray[i].top)
                .css("left",importArray[i].left);                                
                tmpReadyTask.appendTo("#data");
            }      
        }
        $(document).disableTextSelect();
    }
    
    /* ------------- BUTTONS -------------*/
    
    if(sankore.preference("by_style","")){
        changeStyle(sankore.preference("by_style",""));
        $(".style_select").val(sankore.preference("by_style",""));
    } else
        changeStyle("3")

    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){                
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $(".style_select").css("display","none");                
                $(this).css("display", "none");
                $("#wgt_add").css("display", "none");
                $("#wgt_edit").css("display", "block");
                mode = false;
                $(".leftDiv, .rightDiv").animate({
                    "opacity":"1"
                },"slow",function(){
                    if(opacityChanged){
                        if($(".editContainer").size() != 0){
                            $(".editContainer").each(function(index, domElem){
                                var tmpReadyTask = $(domElem).find(".readyTask");
                                tmpReadyTask.width($(domElem).width())
                                .height($(domElem).height())
                                .css("position","absolute")
                                .css("top",$(domElem).position().top)
                                .css("left",$(domElem).position().left)
                                .find(".taskContainer").removeAttr("contenteditable");                                
                                
                                $(domElem).remove();
                                tmpReadyTask.appendTo("#data");
                            });
                        }
                        opacityChanged = false;
                    }
                });
                $(document).disableTextSelect(); 
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $(".style_select").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_add").css("display", "block");
                $("#wgt_display").css("display", "block");
                mode = true; 
                $(document).enableTextSelect(); 
                $(".leftDiv, .rightDiv").animate({
                    "opacity":"0.4"
                },"slow",function(){
                    if(!opacityChanged){
                        if($(".readyTask").size() != 0){
                            $(".readyTask").each(function(index, domElem){     
                                var editContent = $("<div class='editContainer'>").width($(domElem).width() + 10).height($(domElem) + 10).appendTo("#data");
                                var closeItem = $("<div class='closeItem'>").appendTo(editContent);
                                var rightResize = $("<div class='rightResize'>").appendTo(editContent);
                                var bottomResize = $("<div class='bottomResize'>").appendTo(editContent);
                                editContent.css("top", $(domElem).position().top).css("left", $(domElem).position().left);
                                $(domElem).css("position","static")
                                .width("100%")
                                .height("100%")
                                .find(".taskContainer").attr("contenteditable", "true");
                                $(domElem).appendTo(editContent);
                            });
                        }
                        opacityChanged = true;
                    }
                });
            }
        }
    });
    
    $("#wgt_help").click(function(){
        var tmp = $(this);
        if($(this).hasClass("open")){
            $("#help").slideUp("100", function(){
                tmp.removeClass("open");
                $("#data").show();
            });
        } else {            
            $("#data").hide();
            $("#help").slideDown("100", function(){
                tmp.addClass("open");
            });
        }
    });

    $("#wgt_reload").click(function(){
        window.location.reload();
    });
    
    $("#wgt_add").click(function(){
        shadowDiv.show("fast", function(){
            shadowOver = true;
            popupBack.show("slow");            
        });
        $(document).disableTextSelect();
    });
    
    $(".style_select option[value='1']").text(sankoreLang.slate);
    $(".style_select option[value='2']").text(sankoreLang.pad);
    
    $(".style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
    
    /* -------------- END OF WORK WITH BUTTONS ---------------*/
    
    //basic divs
    var leftDiv = $("<div id='leftDiv' class='leftDiv'>").appendTo("#data");
    var rightDiv = $("<div id='rightDiv' class='rightDiv'>").appendTo("#data");
    
    //divs for adding a new item
    var shadowDiv = $("<div id='shadowDiv' class='shadowDiv'>").appendTo("#data");
    var popupBack = $("<div id='popupBack' class='popupBack'>").appendTo("#data");
    
    //input fields and buttons for a popup window
    var expressionDiv = $("<div id='expressionDiv' class='popupContainers'>").appendTo(popupBack);
    var experssionLabel = $("<span id='experssionLabel' class='popupLabels'><b>" + sankoreLang.enter_data + "</b></span>").appendTo(expressionDiv);
    var expresionText = $("<input type='text' id='expresionText' class='expresionInput'/>").appendTo(expressionDiv);
    
    var resultDiv = $("<div id='resultDiv' class='popupContainers'>").appendTo(popupBack);
    var resultLabel = $("<span id='resultLabel' class='popupLabels'><b>" + sankoreLang.enter_result + "</b></span>").appendTo(resultDiv);
    var resultText = $("<input type='text' id='resultText' class='expresionInput'/>").appendTo(resultDiv);
    
    var popupButtonsDiv= $("<div id='popupButtonsDiv' class='popupContainers'>").appendTo(popupBack);
    var cancelButton = $("<input type='button' id='cancelButton' class='popupButtons' value='" + sankoreLang.cancel + "'/>").appendTo(popupButtonsDiv);
    var okButton = $("<input type='button' id='okButton' class='popupButtons' value='" + sankoreLang.ok + "'/>").appendTo(popupButtonsDiv);
    
    /* -------------- A WORK WITH POPUP BUTTONS AND FIELDS ---------------*/
    
    $("#resultText, #expresionText").keyup(function(){
        if($(this).val()){
            $(this).css("background-color", "#ff9");
        }
    });
    
    cancelButton.click(function(){
        $(document).enableTextSelect();
        popupBack.hide("slow", function(){
            $("#resultText, #expresionText").val("")
            .css("background-color", "#ffc");
            shadowDiv.hide("fast");
            shadowOver = false; 
        });        
    });
    
    okButton.click(function(){
        $(document).enableTextSelect();
        if(checkEmptyFields(expresionText) && checkEmptyFields(resultText)){
            popupBack.hide("slow", function(){
                shadowDiv.hide("fast");
                shadowOver = false;
                addTask(expresionText.val(), resultText.val());
                exportToSankore();
                $("#resultText, #expresionText").val("")
                .css("background-color", "#ffc");
            })
        }
    });
    
    /* -------------- THE END OF WORK WITH POPUP BUTTONS AND FIELDS ---------------*/    
    
    // a work with dragging possibility    
    $("input:text, .style_select").mouseover(function(){
        $(document).enableTextSelect(); 
    });
                        
    $("input:text, .style_select").mouseout(function(){
        $(document).disableTextSelect(); 
    });
                    
    $(".readyTask, .editContainer").live("mousedown",function(event){
        if(!shadowOver){
            dragElement = $(this);
            coords.left = event.pageX - $(this).position().left;
            coords.top = event.pageY - $(this).position().top;
            resizeObj.width = $(this).width();
            resizeObj.height = $(this).height();
        }
    });
    
    $(".rightResize").live("mousedown",function(event){
        if(!shadowOver){
            resizeObj.x = true;
        }
    });
    
    $(".bottomResize").live("mousedown",function(event){
        if(!shadowOver){
            resizeObj.y = true;
        }
    });
    
    $("body").mouseup(function(event){
        if(!shadowOver){
            if (dragElement) {
                exportToSankore();
            }
            dragElement = null;
            resizeObj.x = false;
            resizeObj.y = false;
        }
    });
    
    $("body").mousemove(function(event){
        if(dragElement && !shadowOver){
            if(resizeObj.x)
                dragElement.width(event.pageX - dragElement.position().left);
            else if(resizeObj.y)                
                dragElement.height(event.pageY - dragElement.position().top);
            else
                dragElement.css("top",event.pageY - coords.top).css("left", event.pageX - coords.left);
        }
    });
    
    //closing item
    $(".closeItem").live("click", function(){
        if(!shadowOver){
            $(this).parent().remove();
        }
    });
    
    //$("#leftDiv,#rightDiv,#shadowDiv").css("height", $(window).height());
    popupBack.css("top", ($(window).height() - 138)*50/$(window).height() + "%");
    popupBack.css("left", ($(window).width() - 360)*50/$(window).width() + "%");
    
    $(window).resize(function(){
        //$("#leftDiv,#rightDiv,#shadowDiv").css("height", $(window).height());
        popupBack.css("top", ($(window).height() - 138)*50/$(window).height() + "%");
        popupBack.css("left", ($(window).width() - 360)*50/$(window).width() + "%");
    });
    
    if (window.widget) {
        window.widget.onleave = function(){
            //exportToSankore();
            sankore.setPreference("by_style", $(".style_select").find("option:selected").val());
        }
    }
    
    // export data
    function exportToSankore(){        
               
        var arrayToExport = new Array();        
        if(mode){            
            if($(".editContainer").size() != 0){
                $(".editContainer").each(function(index, domElem){
                    var objToExport = {
                        data1:"",
                        data2:"",
                        width:0,
                        height:0,
                        top:0,
                        left:0
                    } 
                    objToExport.data1 = $(domElem).find(".readyTask").find(":first-child").text();
                    objToExport.data2 = $(domElem).find(".readyTask").find(":last-child").text();
                    objToExport.width = $(domElem).width();
                    objToExport.height = $(domElem).height();
                    objToExport.top = $(domElem).position().top;
                    objToExport.left = $(domElem).position().left;
                    arrayToExport.push(objToExport);
                });
            }
        } else {      
            if($(".readyTask").size() != 0){
                $(".readyTask").each(function(index, domElem){
                    var objToExport = {
                        data1:"",
                        data2:"",
                        width:0,
                        height:0,
                        top:0,
                        left:0
                    } 
                    objToExport.data1 = $(domElem).find(":first-child").text();
                    objToExport.data2 = $(domElem).find(":last-child").text();
                    objToExport.width = $(domElem).width();
                    objToExport.height = $(domElem).height();
                    objToExport.top = $(domElem).position().top;
                    objToExport.left = $(domElem).position().left;
                    arrayToExport.push(objToExport);
                });
            }              
        }
        sankore.setPreference("blackYellowData", JSON.stringify(arrayToExport));
    }
}

//checking empty fields
function checkEmptyFields(field){
    if(field.val() == ""){
        field.css("background-color", "red");
        return false;
    } else {
        field.css("background-color", "#ff9");
        return true;
    }
}

//adding a new task to the page
function addTask(expression, result){
    var lastItem = $(".editContainer:last"),
        lastItemPos = lastItem.length ? lastItem.position().top : 45,
        lastItemHeight = lastItem.length ? lastItem.height() : 0;
    var editContent = $("<div class='editContainer'>").width(240).height(70).css("top", lastItemPos + lastItemHeight + 15 + "px").appendTo("#data");
    var closeItem = $("<div class='closeItem'>").appendTo(editContent);
    var rightResize = $("<div class='rightResize'>").appendTo(editContent);
    var bottomResize = $("<div class='bottomResize'>").appendTo(editContent);
    var main = $("<div class='readyTask'>");
    var exprContainer = $("<div class='taskContainer' style='color: yellow;' contenteditable='true'>" + expression + "</div>").appendTo(main);
    var resContainer = $("<div class='taskContainer' style='color: black;' contenteditable='true'>"+ result + "</div>").appendTo(main);
    main.appendTo(editContent);
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
            $("#wgt_edit").removeClass("pad_color").removeClass("pad_edit");
            $("#wgt_help").removeClass("pad_color").removeClass("pad_help");
            $("#wgt_display").removeClass("pad_color").removeClass("pad_edit");
            $("#wgt_add").removeClass("pad_color").removeClass("pad_add");
            $("#wgt_name").removeClass("pad_color");
            $(".style_select").removeClass("pad_select").removeClass("none_select").val(val);
            $("body, html").removeClass("without_radius");
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
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_help").addClass("pad_color").addClass("pad_help");
            $("#wgt_display").addClass("pad_color").addClass("pad_edit");
            $("#wgt_add").addClass("pad_color").addClass("pad_add");
            $("#wgt_name").addClass("pad_color");
            $(".style_select").addClass("pad_select").removeClass("none_select").val(val);
            $("body, html").removeClass("without_radius");
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
            $("#wgt_reload").addClass("pad_color").addClass("pad_reload");
            $("#wgt_edit").addClass("pad_color").addClass("pad_edit");
            $("#wgt_help").addClass("pad_color").addClass("pad_help");
            $("#wgt_display").addClass("pad_color").addClass("pad_edit");
            $("#wgt_add").addClass("pad_color").addClass("pad_add");
            $("#wgt_name").addClass("pad_color");
            $(".style_select").addClass("none_select").val(val);
            $("body, html").addClass("without_radius");
            break;
    }
}