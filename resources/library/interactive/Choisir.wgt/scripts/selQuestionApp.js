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

var questionArray;
var currentQstId = "";

function init(){

    //variables
    var toggleFlag = false;
    var endFlag = false;
    var mode = false;
    questionArray = new Array();  
    var popupFlag = false
    var flagForSelect = false;
    
    // toggle button
    var buttonDiv = $("<div id='buttonDiv' class='buttonDiv'>").appendTo("body");
    var toggleButton = $("<button id='toggleButton' class='toggleButton'><</button>").appendTo("#buttonDiv");
    buttonDiv.css({
        top:"10px",
        right:0
    });
    
    //popup message
    var popupText = $("<div id='popupWordInfo' class='popupWordInfo'></div>").appendTo("body");
    
    // adding question block
    var addQstDiv = $("<div id='addQstDiv' class='addQstDiv'>").appendTo("body");
    var addQstButton = $("<button id='addQstButton' class='addQstButton'>").appendTo("#addQstDiv");    
    var addQsqSpan1 = $("<span id='addQsqSpan1'>Q1</span>").appendTo("#addQstButton");
    var addQsqSpan2 = $("<span id='addQsqSpan2'>Add new question ... </span>").appendTo("#addQstButton");
    
    //import saved data
    if(window.sankore){
        if(sankore.preference("qstArrayData","")){
            questionArray = jQuery.parseJSON(sankore.preference("qstArrayData",""));
            for(var i in questionArray){
                addQstBlock(questionArray[i].id, questionArray[i].text, questionArray[i].type,"style='display: none;'");
                for(var j in questionArray[i].answers)
                    addAnsBlock(questionArray[i].answers[j].id, questionArray[i].id, questionArray[i].answers[j].text, true, questionArray[i].rightAns, questionArray[i].type);
            }
        /*for(var i in questionArray)
                setInputSelected(questionArray[i].id, questionArray[i].type, questionArray[i].rightAns);
            alert(2); */          
        }
    }
    
    //saving widget data into sankore object for a correct import
    $("body").mouseout(function(){
        if(window.sankore)
            sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
    })
    // add question
    addQstButton.click(function(){        
        //question block
        var id = Math.round(Math.random()*1000);       
        var obj = new Question();
        obj.id = id;
        questionArray.push(obj);
        
        addQstBlock(id, "Enter your question here ...", "","");
                
        if(window.sankore)
            sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
    });
    
    //toggle mode
    toggleButton.click(function(){
        if(mode){
            editData();
            mode = false;
        } else {
            
            if(window.sankore)
                sankore.setPreference("qstArrayData", JSON.stringify(questionArray));  
            
            if(checkArrayOnFill(questionArray) != 0)
                displayData(true);
            else
                displayData(false);
            mode = true;
        }
        toggleButton.trigger("mouseout");
    });
    
    // toggle button events
    toggleButton.mouseover(function(){
        if(!toggleFlag && !endFlag){
            endFlag = true;
            toggleButton.animate({
                width:"72px"
            },"fast",function(){
                toggleFlag = true;
                if(!mode)
                    toggleButton.text("Display");
                else
                    toggleButton.text("Edit");
            });
        }
    });
    
    toggleButton.mouseout(function(){
        if(toggleFlag && endFlag){
            endFlag = false;
            toggleButton.animate({
                width:"20px"
            },"fast", function(){
                toggleButton.text("<");
                toggleFlag = false;
            });
        }
    });
    
    //set used at this moment question id into the variable 
    $(".qstDiv").live('mouseover', function() {
        currentQstId = this.id;
    });
    
    //set used at this moment question id into the variable 
    $(".qstDivDisplay").live('mouseover', function() {
        if(!flagForSelect)
            currentQstId = this.id.replace("qstDivDisplay","");
    });
       
    //adding new answer
    $(".ansAdd").live('click', function(){
        
        var id = Math.round(Math.random()*1000);        
        var obj = new Answer();
        obj.id = id;
        getNeededElement(questionArray, currentQstId).answers.push(obj);
        
        addAnsBlock(id, currentQstId, "Enter the answer here ...");
    });
    
    //set answer text
    $(".ansContent").live('keyup', function(event){
        var id = $(event.target).attr("id").replace("ansContent","");
        getNeededElement(getNeededElement(questionArray, currentQstId).answers,id).text = $(event.target).text();
    //questionArray[currentQstId].answers[id].text = $(event.target).text();
    });
    
    //set question text
    $(".qstContent").live('keyup', function(event){
        var id = $(event.target).attr("id").replace("qstContent","");
        getNeededElement(questionArray, id).text = $(event.target).text();
    });
    
    //question div border
    $(".qstContent").live('mouseover', function(event){
        $(event.target).css({
            border:"3px solid #00C6FF"
        });
    });
    
    // deleting question div border
    $(".qstContent").live('mouseout', function(event){
        $(event.target).css({
            border:"3px solid #ccc"
        });
    });
    
    //answer div border
    $(".ansContent").live('mouseover', function(event){
        $(event.target).css({
            border:"3px solid #00C6FF"
        });
    });
    
    // deleting answer div border
    $(".ansContent").live('mouseout', function(event){
        $(event.target).css({
            border:"3px solid #ccc"
        });
    });
    
    //delete answer
    $(".ansDelete").live('click', function(){
        popupText.hide();
        var id = this.id.replace("ansDelete","");
        $("#" + currentQstId + " #" + id).remove();
        for(var i in questionArray)
            if(questionArray[i].id == currentQstId){
                questionArray[i].rightAns = questionArray[i].rightAns.replace($("#" + currentQstId + " #" + id + " input:checkbox").val(),"");
                for(var j in questionArray[i].answers)
                    if(questionArray[i].answers[j].id == id){
                        delete questionArray[i].answers[j];
                        break;
                    }
            }
        refreshAns();
    });
    
    //delete question
    $(".qstDelete").live('click', function(){
        popupText.hide();
        $("#" + currentQstId).remove();
        for(var i in questionArray)
            if(questionArray[i].id == currentQstId){
                delete questionArray[i];
                break;
            }
        refreshQst();
    });
    
    //change options
    $(".changeOptions").live('click', function(){
        $("#" + currentQstId + "qstOptions .changeOptions").hide();
        $("#" + currentQstId + "qstOptions .applyChanges").show();
        $("#" + currentQstId + "qstOptChoice").show('fast');
    });
    
    //apply changes
    $(".applyChanges").live('click', function(){
        $("#" + currentQstId + "qstOptions .applyChanges").hide();
        $("#" + currentQstId + "qstOptions .changeOptions").show();
        $("#" + currentQstId + "qstOptChoice").hide('fast');
    });
    
    //select option
    $(".qstOptChoice input:radio").live('click', function(event){
        if(!mode){
            $("#" + currentQstId + "qstOptChoice input:radio").removeAttr("checked");
            $(event.target).attr("checked", "checked");
            getNeededElement(questionArray, currentQstId).type = $(event.target).attr("value");
        }
    });
    
    //select right ansver in edit mode and answer question in display mode
    $(".newAnswer input").live('click', function(event){

        if(!mode){
            if(getNeededElement(questionArray, currentQstId).type == "1" || getNeededElement(questionArray, currentQstId).type == "3"){
                $("#" + currentQstId + "ansDiv input").removeAttr("checked");
                $(event.target).attr("checked", "checked");
                getNeededElement(questionArray, currentQstId).rightAns = $(event.target).attr("value");
            //alert(getNeededElement(questionArray, currentQstId).rightAns);
            } else {
                //alert(event.target.checked)
                (event.target.checked) ? $(event.target).attr("checked", "checked") : $(event.target).removeAttr("checked", "checked");
                getNeededElement(questionArray, currentQstId).rightAns = "";
                for(var i in $("#" + currentQstId + "ansDiv input")){
                    if($("#" + currentQstId + "ansDiv input")[i].checked)
                        getNeededElement(questionArray, currentQstId).rightAns += $("#" + currentQstId + "ansDiv input")[i].value + ", ";
                }
            }
        } else {
            if(event.target.type == "radio"){
                if(event.target.value == getNeededElement(questionArray, currentQstId).rightAns)
                    $(event.target).next().next().css("background-color","#6c0");
                else
                    $(event.target).next().next().css("background-color","#f66");
            } else {
                //alert(event.target.value + " | " + getNeededElement(questionArray, currentQstId).rightAns + " | " + getNeededElement(questionArray, currentQstId).rightAns.replace(/,/g,"") + " | " + getNeededElement(questionArray, currentQstId).rightAns.replace(",","").indexOf(event.target.value + " ", 0) )
                if(getNeededElement(questionArray, currentQstId).rightAns.replace(/,/g,"").indexOf(event.target.value + " ", 0) != -1)
                    $(event.target).next().next().css("background-color","#6c0");
                else
                    $(event.target).next().next().css("background-color","#f66");
            }
        }
    });
    
    //popup messages
    $(".qstDelete").live('mouseover', function(evt){
        popupFlag = true;
        popupText.text("Delete question")
        .css("top", evt.pageY + 15)
        .css("left", evt.pageX - 40)
        .css({
            width:"120px"
        })
        .show("fast", function(){
            if(!popupFlag)
                popupText.hide();
        });
    });
    
    $(".ansDelete").live('mouseover', function(evt){
        popupFlag = true;
        popupText.text("Delete answer")
        .css("top", evt.pageY + 15)
        .css("left", evt.pageX - 40)
        .css({
            width:"110px"
        })
        .show("fast", function(){
            if(!popupFlag)
                popupText.hide();
        });
    });
    
    $(".newAnswer input").live('mouseover', function(evt){
        if(!mode){
            popupFlag = true;
            popupText.text("Right answer")
            .css("top", evt.pageY + 15)
            .css("left", evt.pageX - 40)
            .css({
                width:"105px"
            })
            .show("fast", function(){
                if(!popupFlag)
                    popupText.hide();
            });
        }
    });
    
    $(".qstDelete, .ansDelete, .newAnswer input").live('mousemove', function(evt){
        if(!mode){
            popupText.css("top", evt.pageY + 15)
            .css("left", evt.pageX - 40);
        }
    });
    
    $(".qstDelete, .ansDelete, .newAnswer input").live('mouseout', function(evt){
        if(!mode){
            popupFlag = false;
            popupText.hide();
        }
    });

    //answer the questions
    
    $("select").live('change', function(evt){
        if(mode){
            //alert(currentQstId + " | " + event.target.value + " | " + getNeededElement(questionArray, currentQstId).rightAns)
            if(event.target.value == getNeededElement(questionArray, currentQstId).rightAns)
                $(event.target).parent().css("background-color","#6c0");
            else
                $(event.target).parent().css("background-color","#f66");
            flagForSelect = false;
        }
    });
    
    $("select").live('mousedown', function(evt){
        if(mode){
            flagForSelect = true;
        }
    });
    
    //toggle button click trigger
    toggleButton.trigger("click");
    //show data in display mode
    function displayData(flag){
        $("#addQstDiv").hide();
        $(".qstDiv").hide();
        addToPage(questionArray, flag);
    }
    
    //set widget in edit mode
    function editData(){
        $(".qstDivDisplay").remove();
        
        $("#addQstDiv").show('fast');
        $(".qstDiv").show('fast');
    }
    
    // show questions and answers in display mode
    function addToPage(array, flag){
        if(flag){
            var counter = 1;
            for(var i in array){

                var qstDiv = $("<div class='qstDivDisplay' id='" + array[i].id + "qstDivDisplay'>");        
                var spanOptConn = $("<div class='spanOptConn'>").appendTo(qstDiv);             
                var qstNumber = $("<span class='qstNumber'>Question " + counter + "</span>").appendTo(spanOptConn);        
                var qstContent = $("<div class='qstContentDisplay'>" + array[i].text + "</div>").appendTo(qstDiv);        
                var ansDiv = $("<div class='ansDiv' id='" + array[i].id + "ansDiv'>").appendTo(qstDiv);

                var ansCount = 1;
                var type = array[i].type;
                var selInput = $("<select>");
                if(type == 3){
                    var newAnswer = $("<div class='newAnswer'>"); 
                    newAnswer.appendTo(ansDiv);
                    selInput.appendTo(newAnswer);
                    $("<option value='0'>Choise the right answer</option>").appendTo(selInput);
                }
                for(var j in array[i].answers){  
                    switch(type){
                        case "1":
                            newAnswer = $("<div class='newAnswer'>");
                            var ansInput = $("<input type='radio' name='" + counter + "' value='" + array[i].answers[j].value + "' style='float: left; margin-right: 10px;'/>").appendTo(newAnswer);
                            var ansSpan = $("<span class='ansSpanDisplay'>" + ansCount + ".</span>").appendTo(newAnswer);                        
                            var ansContent = $("<div class='ansContentDisplay'>" + array[i].answers[j].text + "</div>").appendTo(newAnswer);
                            newAnswer.appendTo(ansDiv);
                            break;
                        case "2":
                            newAnswer = $("<div class='newAnswer'>");
                            ansInput = $("<input type='checkbox' value='" + array[i].answers[j].value + "' style='float: left; margin-right: 10px;'/>").appendTo(newAnswer);
                            ansSpan = $("<span class='ansSpanDisplay'>" + ansCount + ".</span>").appendTo(newAnswer);                        
                            ansContent = $("<div class='ansContentDisplay'>" + array[i].answers[j].text + "</div>").appendTo(newAnswer);
                            newAnswer.appendTo(ansDiv);
                            break;
                        case "3":
                            ansInput = $("<option value='" + array[i].answers[j].value + "'>" + array[i].answers[j].text + "</option>").appendTo(selInput);
                            break;
                    }               
                    ansCount++;
                }
                qstDiv.appendTo("body");
                counter++;
            }
        } else {
            counter = 1;
            qstDiv = $("<div class='qstDivDisplay'>");        
            spanOptConn = $("<div class='spanOptConn'>").appendTo(qstDiv);             
            qstNumber = $("<span class='qstNumber'>Question " + counter + "</span>").appendTo(spanOptConn);        
            qstContent = $("<div class='qstContentDisplay'>This is an example of the question.</div>").appendTo(qstDiv);        
            ansDiv = $("<div class='ansDiv'>").appendTo(qstDiv);
            
            ansCount = 1;
            for(j = 0; j < 3; j++){  
                newAnswer = $("<div class='newAnswer'>");
                ansInput = $("<input type='radio' name='1' style='float: left; margin-right: 10px;'/>").appendTo(newAnswer);
                ansSpan = $("<span class='ansSpanDisplay'>" + ansCount + ".</span>").appendTo(newAnswer);                        
                ansContent = $("<div class='ansContentDisplay'>This is an example of the answer " + ansCount + ".</div>").appendTo(newAnswer);
                newAnswer.appendTo(ansDiv);                        
                ansCount++;
            }
            qstDiv.appendTo("body");
        }
    }
}

//add new question block in a edit mode
function addQstBlock(id, text, type, style){
    var qstDiv = $("<div class='qstDiv' id='" + id + "' " + style + ">");
    var spanOptConn = $("<div class='spanOptConn'>").appendTo(qstDiv);
        
    var count = $(".qstNumber").size();       
    var qstNumber = $("<span class='qstNumber'>Q" + (count + 1) + "</span>").appendTo(spanOptConn);
        
    var qstOptions = $("<div class='qstOptions' id='" + id + "qstOptions'>").appendTo(spanOptConn);
    var changeOptions = $("<button class='changeOptions'>Options</button>").appendTo(qstOptions);
    var applyChanges = $("<button class='applyChanges' style='display: none;'>Close</button>").appendTo(qstOptions);
    var qstDelete = $("<button class='qstDelete'>").appendTo(qstOptions);
        
    var qstOptChoice = $("<div class='qstOptChoice' id='" + id + "qstOptChoice' style='display: none;'>").appendTo(qstDiv);
    var optDesc = $("<div style='height: 65px;'>").appendTo(qstOptChoice);
    var optDescImg = $("<div class='optDescImg'>").appendTo(optDesc);
    var optDescText = $("<div class='optDescText'>You can choose any of these three options of displaying your answers. See a short their description.</div>").appendTo(optDesc);
        
    var type1 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType1 = $("<div class='contentType'>").appendTo(type1);
    var divType1 = $("<div class='divType1'>").appendTo(contentType1);
    var textType1 = $("<div class='textType'>This option allow to choose one answer only and the answers are displayed as radio buttons.</div>").appendTo(contentType1);
        
    var type2 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType2 = $("<div class='contentType'>").appendTo(type2);
    var divType2 = $("<div class='divType2'>").appendTo(contentType2);
    var textType2 = $("<div class='textType'>This option allow to choose several answers and the answers are displayed as checkboxes.</div>").appendTo(contentType2);
        
    var type3 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType3 = $("<div class='contentType'>").appendTo(type3);
    var divType3 = $("<div class='divType3'>").appendTo(contentType3);
    var textType3 = $("<div class='textType'>This option allow to choose one answer only and the answers are displayed as pull-down menu.</div>").appendTo(contentType3);

    switch(type){
        case "1":
            var radioType1 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='1' checked/><div></div>").appendTo(type1);
            var radioType2 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='2'/><div></div>").appendTo(type2);
            var radioType3 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='3'/><div></div>").appendTo(type3);
            break;
        case "2":
            radioType1 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='1'/><div></div>").appendTo(type1);
            radioType2 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='2' checked/><div></div>").appendTo(type2);
            radioType3 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='3'/><div></div>").appendTo(type3);
            break;
        case "3":
            radioType1 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='1'/><div></div>").appendTo(type1);
            radioType2 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='2'/><div></div>").appendTo(type2);
            radioType3 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='3' checked/><div></div>").appendTo(type3);
            break;
        default:
            radioType1 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='1'/><div></div>").appendTo(type1);
            radioType2 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='2'/><div></div>").appendTo(type2);
            radioType3 = $("<div class='radioDiv'><div class='radioType'><input type='radio' value='3'/><div></div>").appendTo(type3);
            break;
    }
    
    var qstContent = $("<div class='qstContent' id='" + id + "qstContent' contenteditable='true'>" + text + "</div>").appendTo(qstDiv);
        
    var ansDiv = $("<div class='ansDiv' id='" + id + "ansDiv'>").appendTo(qstDiv);
    var ansAdd = $("<button class='ansAdd'>Add answer</button>").appendTo(ansDiv);
    qstDiv.insertBefore("#addQstDiv");
    $("#addQsqSpan1").text("Q" + (count + 2));
}

//add answers
function addAnsBlock(id, currId, text, stage, rightAns, type){
    var newAnswer = $("<div class='newAnswer' id='" + id + "'>");
    var value;
    var check = "";
    if(stage){
        value = getNeededElement(getNeededElement(questionArray, currId).answers,id).value;
        if(type != "2"){
            if(rightAns == value)
                check = "checked='true'";
        }
        else {
            rightAns = rightAns.replace(/,/g,"");
            if(rightAns.indexOf(value + " ", 0) != -1)
                check = "checked='true'";                
        }
    }
    else {
        value = ($("#" + currId + " .newAnswer input:checkbox").last().val()) ? parseInt($("#" + currId + " .newAnswer input:checkbox").last().val()) + 1 : 1;
        getNeededElement(getNeededElement(questionArray, currId).answers,id).value = value;
    }
    var count = $("#" + currId + " .newAnswer").size() + 1;
    var input = $("<input type='checkbox' style='float: left;' value='" + value + "' " + check + ">").appendTo(newAnswer);
    var ansSpan = $("<span class='ansSpan'>A" + count + "</span>").appendTo(newAnswer);
    var ansContent = $("<div class='ansContent' id='" + id +"ansContent' contenteditable='true'>" + text + "</div>").appendTo(newAnswer);
    var ansDelete = $("<button class='ansDelete' id='" + id + "ansDelete'>").appendTo(newAnswer);
    newAnswer.insertBefore("#" + currId + "ansDiv .ansAdd");    
        
    if(window.sankore)
        sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
}

//get needed array element
function getNeededElement(array,id){
    for(var i in array)
        if(array[i].id == id)
            return array[i];
}

//check on fill
function checkArrayOnFill(array){
    var count = 0;
    
    for(var i in array)
        count++;
    
    return count;
}

//refresh answers numbers
function refreshAns(){
    var count = $("#" + currentQstId + " .newAnswer").size();        
    for(var i = 0; i < count; i ++)
        $($("#" + currentQstId + " .newAnswer span")[i]).text("A" + (i+1));
}

//refresh questions numbers
function refreshQst(){
    var count = $(".qstNumber").size();        
    for(var i = 0; i < count; i ++)
        $($(".qstNumber")[i]).text("Q" + (i+1));
    $("#addQsqSpan1").text("Q" + ++count);
}

//question constructor
function Question(){

    this.text = "";
    
    this.type = "1";
    
    this.id = "";
    
    this.rightAns = "";
    
    this.answers = new Array();

}

//answer constructor
function Answer(){
    
    this.id = "";
    
    this.text = "";
    
    this.value = "";
}