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
    edit: "Edit",
    display:"Display",
    question:"Question",
    example_question:"This is an example of the question",
    answer:"This is one possible answer",
    q:"Q",
    add_new_question:" Add a new question ...",
    options:"Options",
    close:"Close",
    delete_question:"Delete question",
    delete_answer:"Delete the possibility",
    right_answer:"Right answer",
    template_question:"Enter your question here ...",
    template_answer:"Enter one possible answer here ...",
    add_answer:"Add one possible answer",
    select_text:"Choose the right answer",
    options_desc:"You can choose one of the three options of displaying possibilities.",
    radio_desc:"One right answer only and the possibilities are displayed as radio buttons.",
    checkbox_desc:"Several right answers and possibilities are displayed as checkboxes.",
    select_desc:"One right answer only and possibilities are displayed as pull-down menu.",
    a:"A",
    wgt_name: "Choose the right answer",
    reload: "Reload",
    slate: "slate",
    pad: "pad",
    none: "none",
    help: "Help",
    help_content: 
"<p><h2> Choose the right answer</h2></p>" +
"<p><h3> Multiple-Choice Question (MCQ)</h3></p>" +
"<p>Multiple-choice question. The goal is to choose the correct answer.</p>" +
"<p>“Reload” button resets the exercises.</p>" +

"<p> Enter the “Edit” mode to :</p>" +
"<ul><li> choose the theme of interactivity : pad, slate or none (none by default),</li>" +
"<li> modify the exercise.</li></ul>" +

"<p>In edition mode :</p>" +
"<ul> <li> click on “Add a new question”,</li>" +
"<li>edit the text field clicking on the text field “Enter your question here ... ”,</li>" +
"<li>click on “Options” to select the type of our MCQ (one correct answer, multiple correct answers, drop down) and close it,</li>" +
"<li>add a new possible answer if needed,</li>" +
"<li>modify the text field clicking on it,</li>" +
"<li>set the correct answer by clicking in the checkbox to the left of the text field,</li>" +
"<li>remove a label clicking on the cross button on the frame,</li>" +
"<li>delete a possible answer clicking on the cross on the left,</li>" +
"<li>“Display” button comes back to the activity.</li></ul>",
    theme: "Theme"
};

var questionArray;
var currentQstId = "";
var lang = ""; //locale language
var begin = true;

function init(){

    //variables
    var toggleFlag = false;
    var endFlag = false;
    var mode = true;
    questionArray = new Array();  
    var popupFlag = false
    var flagForSelect = false;    
    
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
    
    //popup message
    var popupText = $("<div id='popupWordInfo' class='popupWordInfo'></div>").appendTo("#data");
    
    // adding question block
    var addQstDiv = $("<div id='addQstDiv' class='addQstDiv'>").appendTo("#data");
    var addQstButton = $("<button id='addQstButton' class='addQstButton'>").appendTo("#addQstDiv");    
    var addQsqSpan1 = $("<span id='addQsqSpan1'>" + sankoreLang.q + "1</span>").appendTo("#addQstButton");
    var addQsqSpan2 = $("<span id='addQsqSpan2'>" + sankoreLang.add_new_question + "</span>").appendTo("#addQstButton");
    
    //import saved data
    if(window.sankore){
        if(sankore.preference("qstArrayData","") && sankore.preference("qstArrayData","") != "[]")
            questionArray = jQuery.parseJSON(sankore.preference("qstArrayData",""));         
        else
            questionArray = jQuery.parseJSON('[{"text":"' + sankoreLang.example_question + '","type":"1","id":538,"rightAns":"2","answers":[{"id":953,"text":"' + sankoreLang.answer + ' 1.","value":1,"state":"","was":false},{"id":526,"text":"' + sankoreLang.answer + ' 2.","value":2,"state":"","was":false},{"id":473,"text":"' + sankoreLang.answer + ' 3.","value":3,"state":"","was":false}]}]');
        
        for(i in questionArray){
            addQstBlock(questionArray[i].id, questionArray[i].text, questionArray[i].type,"style='display: none;'");
            for(j in questionArray[i].answers)
                addAnsBlock(questionArray[i].answers[j].id, questionArray[i].id, questionArray[i].answers[j].text, true, questionArray[i].rightAns, questionArray[i].type);
        }
        displayData();
    }
    else{ 
        questionArray = jQuery.parseJSON('[{"text":"' + sankoreLang.example_question + '","type":"1","id":538,"rightAns":"2","answers":[{"id":953,"text":"' + sankoreLang.answer + ' 1.","value":1,"state":"","was":false},{"id":526,"text":"' + sankoreLang.answer + ' 2.","value":2,"state":"","was":false},{"id":473,"text":"' + sankoreLang.answer + ' 3.","value":3,"state":"","was":false}]}]');
        for(i in questionArray){
            addQstBlock(questionArray[i].id, questionArray[i].text, questionArray[i].type,"style='display: none;'");
            for(j in questionArray[i].answers)
                addAnsBlock(questionArray[i].answers[j].id, questionArray[i].id, questionArray[i].answers[j].text, true, questionArray[i].rightAns, questionArray[i].type);
        }
        displayData();
    }
    
    //saving widget data into sankore object for a correct import
    if (window.widget) {
        window.widget.onleave = function(){
            sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
            sankore.setPreference("choisir_style", $("#style_select").find("option:selected").val());
        }
    }
    
    if(window.sankore)
        if(sankore.preference("choisir_style","")){
            changeStyle(sankore.preference("choisir_style",""));
            $("#style_select").val(sankore.preference("choisir_style",""));
        } else
            changeStyle("3")

    $("#wgt_display, #wgt_edit").click(function(event){
        if(this.id == "wgt_display"){
            if(!$(this).hasClass("selected")){                
                $(this).addClass("selected");
                $("#wgt_edit").removeClass("selected");
                $("#parameters").css("display","none");                
                $(this).css("display", "none");
                $("#wgt_edit").css("display", "block");
                displayData(true);
                mode = true;
                if(window.sankore){
                    sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
                    sankore.setPreference("choisir_style", $("#style_select").find("option:selected").val());
                }
            }
        } else {            
            if(!$(this).hasClass("selected")){
                $(this).addClass("selected");
                $("#wgt_display").removeClass("selected");
                $("#parameters").css("display","block");                
                $(this).css("display", "none");
                $("#wgt_display").css("display", "block");
                editData();
                mode = false;
            }
        }
    });

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

    $("#wgt_reload").text(sankoreLang.reload).click(function(){
        if($("#wgt_edit").css("display") == "none")
            $("#wgt_display").trigger("click");
        else{
            editData();
            mode = false;
            displayData(true);
            mode = true;
        }
    });
    
    $("#style_select option[value='1']").text(sankoreLang.slate);
    $("#style_select option[value='2']").text(sankoreLang.pad);
    
    $("#style_select").change(function (event){
        changeStyle($(this).find("option:selected").val());
    })
    
    // add question
    addQstButton.click(function(){        
        //question block
        var id = Math.round(Math.random()*1000);       
        var obj = new Question();
        obj.id = id;
        questionArray.push(obj);
        
        addQstBlock(id, sankoreLang.template_question, "","");
                
        if(window.sankore)
            sankore.setPreference("qstArrayData", JSON.stringify(questionArray));
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
        
        addAnsBlock(id, currentQstId, sankoreLang.template_answer);
    });
    
    //set answer text
    $(".ansContent").live('keyup', function(event){
        var id = $(event.target).attr("id").replace("ansContent","");
        getNeededElement(getNeededElement(questionArray, currentQstId).answers,id).text = $(event.target).text();
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
                        if(j == 0)
                            questionArray[i].answers.shift();
                        else
                        if((j+1) == questionArray[i].answers.length)
                            questionArray[i].answers.pop();
                        else
                            questionArray[i].answers = questionArray[i].answers.slice(0,j).concat(questionArray[i].answers.slice(j+1));  
                        break;
                    }
            }
        refreshAns();
    });
    
    //delete question
//    $(".qstDelete").live('click', function(){
//        popupText.hide();
//        $("#" + currentQstId).remove();
//        for(var i in questionArray)
//            if(questionArray[i].id == currentQstId){
//                if(i == 0)
//                    questionArray.shift();
//                else
//                if((i+1) == questionArray.length)
//                    questionArray.pop();
//                else
//                    questionArray = questionArray.slice(0,i).concat(questionArray.slice(i+1));                
//                break;
//            }
//        refreshQst();
//    });
    
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
            } else {
                (event.target.checked) ? $(event.target).attr("checked", "checked") : $(event.target).removeAttr("checked", "checked");
                getNeededElement(questionArray, currentQstId).rightAns = "";
                for(var i in $("#" + currentQstId + "ansDiv input")){
                    if($("#" + currentQstId + "ansDiv input")[i].checked)
                        getNeededElement(questionArray, currentQstId).rightAns += $("#" + currentQstId + "ansDiv input")[i].value + ", ";
                }
            }
        } else {
            if(event.target.type == "radio"){
                checkingAnswers("radio", getNeededElement(questionArray, currentQstId).answers, event.target.value);
                if(event.target.value == getNeededElement(questionArray, currentQstId).rightAns)
                    $(event.target).next().next().css("background-color","#6c0");
                else
                    $(event.target).next().next().css("background-color","red");
            } else {
                checkingAnswers("box", getNeededElement(questionArray, currentQstId).answers, event.target.value, event.target.checked);
                if(getNeededElement(questionArray, currentQstId).rightAns.replace(/,/g,"").indexOf(event.target.value + " ", 0) != -1)
                    $(event.target).next().next().css("background-color","#6c0");
                else
                    $(event.target).next().next().css("background-color","red");
            }
        }
    });
    
    //popup messages
//    $(".qstDelete").live('mouseover', function(evt){
//        popupFlag = true;
//        popupText.text(sankoreLang.delete_question)
//        .css("top", evt.pageY + 15)
//        .css("left", evt.pageX - 40)
//        .css({
//            width:"130px"
//        })
//        .show("fast", function(){
//            if(!popupFlag)
//                popupText.hide();
//        });
//    });
    
    $(".ansDelete").live('mouseover', function(evt){
        popupFlag = true;
        popupText.text(sankoreLang.delete_answer)
        .css("top", evt.pageY + 15)
        .css("left", evt.pageX - 40)
        .css({
            width:"130px"
        })
        .show("fast", function(){
            if(!popupFlag)
                popupText.hide();
        });
    });
    
    $(".newAnswer input").live('mouseover', function(evt){
        if(!mode){
            popupFlag = true;
            popupText.text(sankoreLang.right_answer)
            .css("top", evt.pageY + 15)
            .css("left", evt.pageX - 40)
            .css({
                width:"130px"
            })
            .show("fast", function(){
                if(!popupFlag)
                    popupText.hide();
            });
        }
    });
    
    $(".ansDelete, .newAnswer input").live('mousemove', function(evt){
        if(!mode){
            popupText.css("top", evt.pageY + 15)
            .css("left", evt.pageX - 40);
        }
    });
    
    $(".ansDelete, .newAnswer input").live('mouseout', function(evt){
        if(!mode){
            popupFlag = false;
            popupText.hide();
        }
    });

    //answer the questions
    
    $("select").live('change', function(evt){
        if(mode){
            checkingAnswers("sel", getNeededElement(questionArray, currentQstId).answers, event.target.value);
            if(event.target.value == getNeededElement(questionArray, currentQstId).rightAns)
                $(event.target).css("background-color","#6c0");
            else
                $(event.target).css("background-color","red");
            if(event.target.value == 0)
                $(event.target).css("background-color","");
            flagForSelect = false;
        }
    });
    
    $("select").live('mousedown', function(evt){
        if(mode){
            flagForSelect = true;
        }
    });
    
    //toggle button click trigger
    //toggleButton.trigger("click");
    //show data in display mode
    function displayData(){
//        $("#addQstDiv").hide();
        $(".qstDiv").hide();
        addToPage(questionArray);
    }
    
    //set widget in edit mode
    function editData(){
        for(var i in questionArray)            
            for(var j in questionArray[i].answers){
                questionArray[i].answers[j].state = "";
                questionArray[i].answers[j].was = false
            }               
                
        $(".qstDivDisplay").remove();
        
//        $("#addQstDiv").show('fast');
        $(".qstDiv").show('fast');
    }
    
    // show questions and answers in display mode
    function addToPage(array){
            var counter = 1;
            for(var i in array){

                var qstDiv = $("<div class='qstDivDisplay' id='" + array[i].id + "qstDivDisplay'>");        
                var spanOptConn = $("<div class='spanOptConn'>").appendTo(qstDiv);             
//                var qstNumber = $("<span class='qstNumber'>" + sankoreLang.question + " " + counter + "</span>").appendTo(spanOptConn);        
                var qstContent = $("<div class='qstContentDisplay'>" + array[i].text + "</div>").appendTo(qstDiv);        
                var ansDiv = $("<div class='ansDiv' id='" + array[i].id + "ansDiv'>").appendTo(qstDiv);

                var ansCount = 1;
                var type = array[i].type;
                var selInput = $("<select>");
                if(type == 3){
                    var newAnswer = $("<div class='newAnswer'>"); 
                    newAnswer.appendTo(ansDiv);
                    var selectSpan = $("<span id='answerText'>").appendTo(newAnswer);
                    selInput.appendTo(selectSpan);
                    $("<option value='0'>" + sankoreLang.select_text + "</option>").appendTo(selInput);
                }
                for(var j in array[i].answers){  
                    switch(type){
                        case "1":
                            var local_state = "";
                            var local_color = "";
                            if(begin){
                                local_state = array[i].answers[j].state;
                                local_color = (array[i].answers[j].value == array[i].rightAns)?((array[i].answers[j].was)?"style='background-color: #6c0;'":""):((array[i].answers[j].was)?"style='background-color: red;'":"");
                            }
                            newAnswer = $("<div class='newAnswer'>");
                            var ansInput = $("<input type='radio' name='" + counter + "' value='" + array[i].answers[j].value + "' " + local_state + " style='float: left; margin-right: 10px;'/>").appendTo(newAnswer);
                            var ansSpan = $("<span class='ansSpanDisplay'>" + ansCount + ".</span>").appendTo(newAnswer);                        
                            var ansContent = $("<div class='ansContentDisplay' " + local_color + "><span id='answerText'>" + array[i].answers[j].text + "</span></div>").appendTo(newAnswer);
                            newAnswer.appendTo(ansDiv);
                            break;
                        case "2":
                            local_state = "";
                            local_color = "";
                            if(begin){
                                local_state = (array[i].answers[j].state)?"checked":"";
                                local_color = (array[i].rightAns.replace(/,/g,"").indexOf(array[i].answers[j].value + " ", 0) != -1)?((array[i].answers[j].was)?"style='background-color: #6c0;'":""):((array[i].answers[j].was)?"style='background-color: red;'":"");
                            }
                            newAnswer = $("<div class='newAnswer'>");
                            ansInput = $("<input type='checkbox' value='" + array[i].answers[j].value + "' " + local_state + " style='float: left; margin-right: 10px;'/>").appendTo(newAnswer);
                            ansSpan = $("<span class='ansSpanDisplay'>" + ansCount + ".</span>").appendTo(newAnswer);                        
                            ansContent = $("<div class='ansContentDisplay' " + local_color + "><span id='answerText'>" + array[i].answers[j].text + "</span></div>").appendTo(newAnswer);
                            newAnswer.appendTo(ansDiv);
                            break;
                        case "3":
                            local_state = "";
                            local_color = "";
                            if(begin){
                                local_state = (array[i].answers[j].state)?"selected":"";
                                local_color = (array[i].answers[j].value == array[i].rightAns)?((array[i].answers[j].was)?"#6c0":""):((array[i].answers[j].was)?"red":"");
                            }
                            ansInput = $("<option value='" + array[i].answers[j].value + "' " + local_state + ">" + array[i].answers[j].text + "</option>").appendTo(selInput);
                            if(local_state && local_color)
                                selInput.css("background-color",local_color);
                            break;
                    }               
                    ansCount++;
                }
                qstDiv.appendTo("#data");
                counter++;
            }
            begin = false;
    }
}

//add new question block in a edit mode
function addQstBlock(id, text, type, style){
    var qstDiv = $("<div class='qstDiv' id='" + id + "' " + style + ">");
    var spanOptConn = $("<div class='spanOptConn'>").appendTo(qstDiv);
        
//    var count = $(".qstNumber").size();       
//    var qstNumber = $("<span class='qstNumber'>" + sankoreLang.q + (count + 1) + "</span>").appendTo(spanOptConn);
        
    var qstOptions = $("<div class='qstOptions' id='" + id + "qstOptions'>").appendTo(spanOptConn);
    var changeOptions = $("<button class='changeOptions'>" + sankoreLang.options + "</button>").appendTo(qstOptions);
    var applyChanges = $("<button class='applyChanges' style='display: none;'>" + sankoreLang.close + "</button>").appendTo(qstOptions);
//    var qstDelete = $("<button class='qstDelete'>").appendTo(qstOptions);
        
    var qstOptChoice = $("<div class='qstOptChoice' id='" + id + "qstOptChoice' style='display: none;'>").appendTo(qstDiv);
    var optDesc = $("<div style='height: 65px;'>").appendTo(qstOptChoice);
    var optDescImg = $("<div class='optDescImg'>").appendTo(optDesc);
    var optDescText = $("<div class='optDescText'>" + sankoreLang.options_desc + "</div>").appendTo(optDesc);
        
    var type1 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType1 = $("<div class='contentType'>").appendTo(type1);
    var divType1 = $("<div class='divType1'>").appendTo(contentType1);
    var textType1 = $("<div class='textType'>" + sankoreLang.radio_desc + "</div>").appendTo(contentType1);
        
    var type2 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType2 = $("<div class='contentType'>").appendTo(type2);
    var divType2 = $("<div class='divType2'>").appendTo(contentType2);
    var textType2 = $("<div class='textType'>" + sankoreLang.checkbox_desc + "</div>").appendTo(contentType2);
        
    var type3 = $("<div class='type'>").appendTo(qstOptChoice);
    var contentType3 = $("<div class='contentType'>").appendTo(type3);
    var divType3 = $("<div class='divType3'>").appendTo(contentType3);
    var textType3 = $("<div class='textType'>" + sankoreLang.select_desc + "</div>").appendTo(contentType3);

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
    var ansAdd = $("<button class='ansAdd'>" + sankoreLang.add_answer + "</button>").appendTo(ansDiv);
    qstDiv.insertBefore("#addQstDiv");
//    $("#addQsqSpan1").text(sankoreLang.q + (count + 2));
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
    var ansSpan = $("<span class='ansSpan'>" + sankoreLang.a + count + "</span>").appendTo(newAnswer);
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
        $($("#" + currentQstId + " .newAnswer span")[i]).text(sankoreLang.a + (i+1));
}

//refresh questions numbers
function refreshQst(){
    var count = $(".qstNumber").size();        
    for(var i = 0; i < count; i ++)
        $($(".qstNumber")[i]).text(sankoreLang.q + (i+1));
    $("#addQsqSpan1").text(sankoreLang.q + ++count);
}

//check answers
function checkingAnswers(type, array, value, state){
    switch(type){
        case "radio":
            for(var i in array)
                if(array[i].value == value){
                    array[i].state = "checked";
                    array[i].was = true;
                } else 
                    array[i].state = "";                
            break;
        case "box":
            for(i in array)
                if(array[i].value == value){
                    array[i].state = state;
                    array[i].was = true;
                }     
            break;
        case "sel":
            for(i in array)
                if(array[i].value == value){
                    array[i].state = "selected";
                    array[i].was = true;
                } else 
                    array[i].state = ""; 
            break;
    }
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
    
    this.state = "";
    
    this.was = false;
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
