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

var sankoreLang = {display: "Afficher", edit: "Modifier", add: "Ajouter", enter_data: "Entrer les données:", enter_result: "Entrez le résultat:", ok: "Ок", cancel: "Annuler"};

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
                tmpReadyTask.appendTo("body");
            }      
        }
        $(document).disableTextSelect();
    }
    
    /* ------------- BUTTONS -------------*/
    
    // toggle button
    var buttonDiv = $("<div id='buttonDiv' class='buttonDiv'>").appendTo("body");
    var toggleButton = $("<button id='toggleButton' class='toggleButton'><</button>").appendTo("#buttonDiv");
    buttonDiv.css({
        top:"10px",
        right:0
    });    
          
    //toggle mode
    toggleButton.click(function(){
        if(!shadowOver){
            if(mode){
                mode = false;
                addButtonDiv.css("display","none");
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
                                tmpReadyTask.appendTo("body");
                            });
                        }
                        opacityChanged = false;
                    }
                });
                $(document).disableTextSelect();   
            } else {            
                mode = true;
                addButtonDiv.css("display","block");  
                $(document).enableTextSelect(); 
                $(".leftDiv, .rightDiv").animate({
                    "opacity":"0.4"
                },"slow",function(){
                    if(!opacityChanged){
                        if($(".readyTask").size() != 0){
                            $(".readyTask").each(function(index, domElem){     
                                var editContent = $("<div class='editContainer'>").width($(domElem).width() + 10).height($(domElem) + 10).appendTo("body");
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
            toggleButton.trigger("mouseout");
        }
    });
    
    // toggle button events
    toggleButton.mouseover(function(){
        if(!shadowOver){
            if(!toggleFlag && !endFlag){
                endFlag = true;
                toggleButton.animate({
                    width:"115px"
                },"fast",function(){
                    toggleFlag = true;
                    if(!mode)
                        toggleButton.text(sankoreLang.edit);
                    else
                        toggleButton.text(sankoreLang.display);
                });
            }
        }
    });
    
    toggleButton.mouseout(function(){
        if(!shadowOver){
            if(toggleFlag && endFlag){
                endFlag = false;
                toggleButton.animate({
                    width:"20px"
                },"fast", function(){
                    toggleButton.text("<");
                    toggleFlag = false;
                });
            }
        }
    });
    
    //add button
    var addButtonDiv = $("<div id='addButtonDiv' class='addButtonDiv'>").appendTo("body");
    var addButton = $("<button id='addButton' class='addButton'>+</button>").appendTo("#addButtonDiv");
    addButtonDiv.css({
        top:"47px",
        right:0
    });    
    
    // add button events
    addButton.click(function(){
        shadowDiv.show("fast", function(){
            shadowOver = true;
            popupBack.show("slow");            
        });
        $(document).disableTextSelect();
        addButton.trigger("mouseout");
    });
     
    addButton.mouseover(function(){
        if(!shadowOver){
            if(!addToggleStart && !addToggleEnd){
                addToggleEnd = true;
                addButton.animate({
                    width:"115px"
                },"fast",function(){
                    addToggleStart = true;
                    addButton.text(sankoreLang.add);
                });
            }
        }
    });
    
    addButton.mouseout(function(){
        if(!shadowOver){
            if(addToggleStart && addToggleEnd){
                addToggleEnd = false;
                addButton.animate({
                    width:"20px"
                },"fast", function(){
                    addButton.text("+");
                    addToggleStart = false;
                });
            }
        }
    });
    
    /* -------------- END OF WORK WITH BUTTONS ---------------*/
    
    //basic divs
    var leftDiv = $("<div id='leftDiv' class='leftDiv'>").appendTo("body");
    var rightDiv = $("<div id='rightDiv' class='rightDiv'>").appendTo("body");
    
    //divs for adding a new item
    var shadowDiv = $("<div id='shadowDiv' class='shadowDiv'>").appendTo("body");
    var popupBack = $("<div id='popupBack' class='popupBack'>").appendTo("body");
    
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
                $("#resultText, #expresionText").val("")
                .css("background-color", "#ffc");
            })
        }
    });
    
    /* -------------- THE END OF WORK WITH POPUP BUTTONS AND FIELDS ---------------*/    
    
    // a work with dragging possibility    
    $("input:text").mouseover(function(){
        $(document).enableTextSelect(); 
    });
                        
    $("input:text").mouseout(function(){
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
    
    $("#leftDiv,#rightDiv,#shadowDiv").css("height", $(window).height());
    popupBack.css("top", ($(window).height() - 138)*50/$(window).height() + "%");
    popupBack.css("left", ($(window).width() - 360)*50/$(window).width() + "%");
    
    $(window).resize(function(){
        $("#leftDiv,#rightDiv,#shadowDiv").css("height", $(window).height());
        popupBack.css("top", ($(window).height() - 138)*50/$(window).height() + "%");
        popupBack.css("left", ($(window).width() - 360)*50/$(window).width() + "%");
    });
    
    $("html").mouseout(function(){
        if(window.sankore)
            exportToSankore();        
    });
    
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
    var editContent = $("<div class='editContainer'>").width(240).height(70).appendTo("body");
    var closeItem = $("<div class='closeItem'>").appendTo(editContent);
    var rightResize = $("<div class='rightResize'>").appendTo(editContent);
    var bottomResize = $("<div class='bottomResize'>").appendTo(editContent);
    var main = $("<div class='readyTask'>");
    var exprContainer = $("<div class='taskContainer' style='color: yellow;' contenteditable='true'>" + expression + "</div>").appendTo(main);
    var resContainer = $("<div class='taskContainer' style='color: black;' contenteditable='true'>"+ result + "</div>").appendTo(main);
    main.appendTo(editContent);
}