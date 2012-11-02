var defaults = ["1/1", "2-1", "3*(3+2)", "5*3", "4+12", "0+16"];

var Card = (function() {
    var self = Object.create({}, {
        val: {
            value: -1,
            writable: true
        },
        id: {
            value: -1,
            writable: true
        },
        index: {
            value: -1,
            writable: true
        },
        isMatch: {
            value: function(card) {
                if (this.val == card.val) {
                    return true;
                }
                return false;
            }
        }
    });
    return self;
})();

var Game = (function() {
    var self = Object.create({}, {
        cards: {
            value: null,
            enumarable: true,
            writable: true
        },
        matches: {
            value: 0,
            writable: true
        },
        init: {
            value: function(count) {
                if(typeof count !== "number"){
                    log("Cards count must be an number");	
                    return;
                }
                this.cards = new Array(count);
                for (i = 0; i < this.cards.length / 2; i++) {
                    var card = Object.create(Card, {
                        val: {
                            value: i+1
                        },
                        id: {
                            value: i*2
                        }
                    });
                    card.index = card.id;
                    var card2 = Object.create(Card, {
                        val: {
                            value: i+1
                        },
                        id: {
                            value: card.index+1
                        }
                    });
                    card2.index = card2.id;
                    this.cards[card.index] = card;
                    this.cards[card2.index] = card2;
                }
            }
        },
        selected: {
            value: null,
            writable: true
        }
    });
    return self;
})();

function onTemplateLoadedCallback() {
    $("select[name=count]").each(function(i, select) {
        var min = $(select).data("min");
        var max = $(select).data("max");
        for(var i = min ; i <= max ; i=i+2) {
            var option = $("<option/>");
            option.val(i);
            option.text(fr.njin.i18n.memory.parameters.option.cards(i));
            $(select).append(option);
        }
    });
/*
	$("#timeoutValue").text($("input[name=timeout]").val());	
	$("input[name=timeout]").change(function(){
		$("#timeoutValue").text($(this).val());
	});
	*/
}

function reloadApp(app) {
    var cardsCount = parseInt(app.parameters.value("count"));
    var timeout = parseInt(app.parameters.value("timeout"))*1000;
	
    var scene = $("#scene");
    var table = $("<div id='table'/>");
    scene.empty();
    scene.append(table);

    var game = Object.create(Game);
    game.init(cardsCount);

    var rows = new Array(game.cards.length/2);
    for(i = 0 ; i < game.cards.length/2 ; i++) {
        var row = $("<div/>");
        table.append(row);
        rows[i] = row;
    }

    if(!app.onEdit)
        app.utils.shuffle(game.cards);
		
    for(i = 0 ; i < game.cards.length ; i++) {
        (function(){
            var card = game.cards[i];
            var defaultValue = defaults[card.index]
            card.index = i;

            var pFrontTextKey = cardTextParameterKey(card, 0);
            var pBackTextKey = cardTextParameterKey(card, 1);
            window.card = card;
            window.frontDisplayValue = app.parameters.value(pFrontTextKey) || "?";
            window.backDisplayValue = app.parameters.value(pBackTextKey) || defaultValue;

            var cardView = $(Mustache.render(cardTemplate, window));
            rows[i%2].append(cardView);
            cardView.data("card", card);

            var checkbox = cardView.find("input[name='switch']");
            function setSwicth(index) {
                var val = app.parameters.value("#UsePicture"+cardParameterKey(card, index)) === "true";
                var $p = cardView.find(".cards > div").eq(index);
                if(val) {
                    $p.addClass("usePicture");
                }else {
                    $p.removeClass("usePicture");
                }
                checkbox.eq(index).attr("checked", val);
            }
            checkbox.each(function(index) {
                $(this).change(function() {
                    app.parameters.value("#UsePicture"+cardParameterKey(card, index), $(this).is(':checked'));
                    setSwicth(index);
                });
            });
            setSwicth(0);
            setSwicth(1);
			
            var dropzone = cardView.find(".dropzone");
			
            function setPicture(index) {
                var f = $.parseJSON(app.parameters.value("#Picture"+cardParameterKey(card, index)));
                var pictureHolder = dropzone.eq(index).parent().find(">div:eq(0)");
                if(f !== null) {
                    var $img = $('<img src="" class="uploadPic" title="" alt="" />').attr(f);
                    pictureHolder.empty();
                    pictureHolder.append($img);
                }
            }
            dropzone.each(function(index){
                app.utils.droppable($(this), function(f) {
                    app.parameters.value("#Picture"+cardParameterKey(card, index), JSON.stringify(f));
                    setPicture(index);
                });
            });
            setPicture(0);
            setPicture(1);
        })();
    }
    if(!app.onEdit) {
        $(".cards>div:first-child").click(function(){            
            var cardView = $(this).parent();
            var card = cardView.parent().parent().data("card");
            cardView.toggleClass("flip");
            log("Flip card at index "+card.index+" w/ value "+card.val);
            if(game.selected == null)
                game.selected = card;
            else{
                var toCompare = game.selected;
                log("Compare w/ card at index "+toCompare.index+" w/ value "+toCompare.val);
                var isMatch = card.isMatch(toCompare);
                if(!isMatch) {
                    var index = toCompare.index;
                    var otherCardView = rows[index%2].find(">div").eq(Math.floor(index/2)).find(".cards:eq(0)");
                    setTimeout(function(){
                        cardView.toggleClass("flip");
                        otherCardView.toggleClass("flip");
                    }, timeout);
                }else{
                    game.matches++;
                }
                game.selected = null;
            }
        });
    }else{
        $(".card .text > div").each(function(i, e) {
            var card = $(e).parent().parent().parent().parent().parent().parent().data("card");
            var pKey = cardTextParameterKey(card, i);
            e.contentEditable = true;
            $(e).bind('blur keyup paste', function(){
                app.parameters.value(pKey, $(this).text());
            });
        });
        $(".card-container .actions button[role=flip]").click(function(){
            var cardView = $(this).parent().parent().find(">.cards:eq(0)");
            cardView.toggleClass("flip");
        });
        $(".card-container .actions button[role=flip]").click();
    }
    if(window.sankore)
        window.sankore.enableDropOnWidget(app.onEdit);
}

function cardParameterKey(card, i) {
    return "card"+card.id+(i%2==0 ? "Front":"Back");
}

function cardTextParameterKey(card, i) {
    return cardParameterKey(card, i)+"Text";
}

function reloadCallback(parameter) {
    if(parameter === undefined || parameter.key === "count")
        reloadApp(this);
}

$(document).ready(function(){
    var callbacks = {
        onTemplatesLoaded: onTemplateLoadedCallback,
        onEdit: reloadApp,
        onView: reloadApp
    };
    init(reloadCallback, {
        toolbar: toolbarTemplate, 
        parameters: parametersTemplate
    }, callbacks);
});