//Variables de référence

/*---------------------------------*/
//Fonctions de référence

var text="https://www.amxa.ch";


$(function(){


$("#ok").click(function()
  {
    text=$('#qrtext').val();
    $("#codediv").html("");
    $("#codediv").qrcode({text: text});
    $("#formdiv").addClass("hidden");
    $("#display").html(text);
    $("body").removeClass("grey");
    $("#codediv").removeClass("hidden");
    $("#textdiv").removeClass("hidden");
  });

$("#codediv").click(function(){
    $("#codediv").addClass("hidden");
    $("#textdiv").addClass("hidden");
    $("body").addClass("grey");
    $("#formdiv").removeClass("hidden"); 
   });

   
}); 




