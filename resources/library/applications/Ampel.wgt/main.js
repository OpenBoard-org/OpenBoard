//Variables de référence

/*---------------------------------*/
//Fonctions de référence


$(function(){

$("#red").click(function(){
        $("#red").css("background-color", "red");
        $("#orange").css("background-color", "grey");
        $("#green").css("background-color", "grey");
      });

$("#orange").click(function(){
        $("#red").css("background-color", "grey");
        $("#orange").css("background-color", "orange");
        $("#green").css("background-color", "grey");
      });

$("#green").click(function(){
        $("#red").css("background-color", "grey");
        $("#orange").css("background-color", "grey");
        $("#green").css("background-color", "green");
      });
   

}); 




