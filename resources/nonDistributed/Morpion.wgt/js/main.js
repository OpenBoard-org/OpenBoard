var player;

var defaults = [["1/1", "3*(3+2)", "5-4"],["12+5", "(3*3)+2", "35/5"],["9*9", "3+2", "11+4"]];

function operationParameterKey(i,j) {
	return "cell"+i+","+j;
}


function cellOperationParameterKey(cell) {
	return operationParameterKey(cell.data("i"),cell.data("j"));
}

function reloadApp(app) {
	var scene = $("#scene");
	scene.empty();

	var playerInfo = $("<div id='playerInfo'/>");
	scene.append(playerInfo);
	
	var table = $("<div id='table'/>");
	scene.append(table);

	var gameEnd = false;
	
	player = 0;
	changePlayer();

	for(i = 0 ; i < 3 ; i++) {
		var row = $("<div/>");
		table.append(row);
		for(j = 0 ; j < 3 ; j++) {
			var key = operationParameterKey(i,j);
			window.cell = {i: i , j: j, operation: app.parameters.value(key) || defaults[i][j]};
			var cell = $(Mustache.render(cellTemplate, window));
			row.append(cell);
			if(!app.onEdit) {
				cell.find("span.result:eq(0)").get(0).contentEditable = true;
				cell.find("span.result:eq(0)").focus(function(){
					var input = $(this);
					var parentCell = input.parent().parent();
					log(parentCell.data("player"));
					if(!gameEnd && parentCell.data("player") === undefined) {
						parentCell.parent().parent().find(".onAnswer").removeClass("onAnswer");
						parentCell.addClass("onAnswer");
					}
				});
				cell.find("span.result:eq(0)").bind('blur', function(){
					var v = parseInt($(this).text().trim());
					if(isNaN(v)) {
						$(this).text("");
						var parentCell = $(this).parent().parent();
						parentCell.removeClass("onAnswer");
					}
				});
				
				cell.find("button[role=submit]").click(function(e){
					e.stopPropagation();
					var parentCell = $(this).parent().parent().parent();
					var input = parentCell.find("span.result:eq(0)");
					if(!gameEnd && parentCell.data("player") === undefined) {
						parentCell.parent().parent().find(".onAnswer").removeClass("onAnswer");
						var operation = input.data("operation");
						var toCompare = eval(operation);
						var submitted = input.text();
						log("Compare "+toCompare+" to submitted "+submitted);
						if(toCompare == submitted) {
							log("Player "+player+" successfull answer.");
							
							input.get(0).contentEditable = false;
							
							parentCell.addClass("answered succeed player"+player);
							parentCell.data("player", player);
														
							$("#feedback-succeed").addClass("expand");
							setTimeout(function(){
								$("#feedback-succeed").addClass("closed");
								$("#feedback-succeed").removeClass("expand");
							}, 2000);

							var debug = parentCell.find(".debug");
							debug.text(debug.text()+":"+player);
							
							var wincells = isGameEnd(parentCell.data("i"), parentCell.data("j"));
							if(wincells != null) {
								gameEnd = true;
								table.addClass("finish");
								wincells.addClass("win");
							}
						}else{
							log("Player "+player+" unsuccessfull answer.");						
							parentCell.addClass("answered failed");
							$("#feedback-failed").addClass("expand");
							setTimeout(function(){
								parentCell.removeClass("answered failed");
								input.text("");
							}, 1000);
							setTimeout(function(){
								$("#feedback-failed").addClass("closed");
								$("#feedback-failed").removeClass("expand");
							}, 2000);
						}
						if(!gameEnd)
							changePlayer();
					}
				});
			}else{
				cell.find(".operation").each(function(i, operation){
					var c = $(operation).parent().parent();
					var key = cellOperationParameterKey(c);
					operation.contentEditable = true;
					$(operation).bind('blur keyup paste', function(){
						app.parameters.value(key, $(this).text());
					});
				});
			}			
		}
	}
}

function reloadCallback(parameter) {
	if(parameter === undefined)
		reloadApp(this);
}

function changePlayer() {
	log("Change player");
	player=player%2+1;
	$("#playerInfo").text( fr.njin.i18n.morpion.playerInfo(player, getSignForPlayer(player)));
}

function getSignForPlayer(player) {
	return player === 1 ? "X" : "O";
}

function isGameEnd(i, j) {
	var cells = isCellsWin($("#table>div:eq("+i+")>div"));
	if(cells)
		return cells;
	cells =	isCellsWin($("#table>div:eq(0)>div:eq("+j+") , #table>div:eq(1)>div:eq("+j+") , #table>div:eq(2)>div:eq("+j+")"));
	if(cells)
		return cells;
	cells = isCellsWin($("#table>div:eq(0)>div:eq(0) , #table>div:eq(1)>div:eq(1) , #table>div:eq(2)>div:eq(2)"));
	if(cells)
		return cells;
	cells = isCellsWin($("#table>div:eq(2)>div:eq(0) , #table>div:eq(1)>div:eq(1) , #table>div:eq(0)>div:eq(2)"));
	return cells;
}

function isCellsWin(cells) {
	var n = ((3*player) - (parseInt(cells.eq(0).data("player")) + parseInt(cells.eq(1).data("player")) + parseInt(cells.eq(2).data("player"))));
	if( n == 0 )
		return cells;
	return null;
}

$(document).ready(function(){
	var callbacks = {
		onEdit: reloadApp,
		onView: reloadApp
	};
	init(reloadCallback, {toolbar: toolbarTemplate, parameters: parametersTemplate}, callbacks);
});