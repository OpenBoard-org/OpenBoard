//Variables de référence

/*---------------------------------*/
//Fonctions de référence


$(function(){


$("#bild1").click(function(){
       $("#bild1").addClass("hidden")
       $("#bild2").removeClass("hidden") 
     });

$("#bild2").click(function(){
       $("#bild2").addClass("hidden")
       $("#bild3").removeClass("hidden") 
     });

$("#bild3").click(function(){
       $("#bild3").addClass("hidden")
       $("#bild1").removeClass("hidden") 
     });

   
}); 




