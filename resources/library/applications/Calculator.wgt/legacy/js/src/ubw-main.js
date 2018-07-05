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

function init() {
    var h = 292;
    var w = 160;
    var wh = h + 6;
    var ww = w + 6;
    var clickFlag = false;

    var ubwidget = $("#ubwidget").ubwidget({
        width: w,
        height: h
    });

    var historyTab = $("<div></div>")
        .css({
            height: h,
            marginRight: 0,
            width: 20,
            float: "left",
            backgroundImage: "url(images/historytab.png)",
            backgroundRepeat: "no-repeat"
        })
        .toggle(
            function () {
                resizeWidget(370, 294);
                historyPanel.show();
                $(".ubw-container").css({
                    backgroundImage: "url(images/back.png)",
                    width: 328
                });
                if (window.sankore) {
                    window.sankore.setPreference('historyTab', "visible");
                };
            },
            function () {
                resizeWidget(200, 294);
                historyPanel.hide();
                $(".ubw-container").css({
                    backgroundImage: "url(images/back_small.png)",
                    width: 160
                });
                if (window.sankore) {
                    window.sankore.setPreference('historyTab', "hidden");
                };
            }
    )
        .hover(
            function () {
                $(this).css({
                    backgroundImage: "url(images/historytabOver.png)"
                });
            },
            function () {
                $(this).css({
                    backgroundImage: "url(images/historytab.png)"
                });
            }
    )
        .appendTo($("body"));

    var space = $("<div></div>");
    space.css({
        width: 5,
        height: 1,
        float: "left"
    });

    var keysPanel = $("<div id='keysPanel'></div>").css({
        float: "left"
    });
    var mode = "standard";
    var calc = "";
    var displayTrunk = "";
    var historyTrunk = "";
    var lastHistory = "";
    var subtrunk = 0;
    var itrunk = 0;
    var trunkpos = 0;
    var lastchar = {
        type: "NaN",
        value: "null"
    };
    var inparenthesis = false;
    var erase = false;
    var abtn;
    var readyToCompute = false;
    var lastResult = "0";
    var gDecimalSeparator = "";
    var gThousandsSeparator = "";
    var gDecimalCode = 0;
    var gDecimalString = "";
    var gError = false;

    var historyTxt = "0";

    var display = $("<div id='display'>0</div>")
        .addClass("ubw-inputbox")
        .css({
            padding: "12px",
            marginLeft: "2px",
            marginBottom: "9px",
            marginTop: 7,
            backgroundImage: "url(images/display.png)",
            fontSize: "22px",
            fontFamily: "Arial, Helvetica, Sans-serif",
            color: "#777788",
            width: 118,
            height: 28,
            overflow: "hidden",
            textAlign: "right",
        });

    var standardPanel = $("<div></div>").css({
        float: "left"
    });

    var cKeySize = {
        w: 33,
        h: 36
    };

    var btn7 = $("<div><img src='images/touche7.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("7", 1)
        });
    var btn8 = $("<div><img src='images/touche8.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("8", 1)
        });
    var btn9 = $("<div><img src='images/touche9.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("9", 1)
        });
    var btnDiv = $("<div><img src='images/touchediv.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("/", 0)
        });
    var btn4 = $("<div><img src='images/touche4.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("4", 1)
        });
    var btn5 = $("<div><img src='images/touche5.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("5", 1)
        });
    var btn6 = $("<div><img src='images/touche6.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("6", 1)
        });
    var btnMul = $("<div><img src='images/touchef.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("*", 0)
        });
    var btn1 = $("<div><img src='images/touche1.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("1", 1)
        });
    var btn2 = $("<div><img src='images/touche2.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("2", 1)
        });
    var btn3 = $("<div><img src='images/touche3.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("3", 1)
        });
    var btnSou = $("<div><img src='images/touchem.png'/></div>").ubwbutton({
        w: 68,
        h: 34
    })
        .mousedown(function () {
            appendToDisplay("-", 0)
        });
    var btn0 = $("<div><img src='images/touche0.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("0", 1)
        });
    var btnDot = $("<div><img src='images/touchedot.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay(".", 1)
        });
    var btnC = $("<div><img src='images/touchec.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            reset()
        });
    var btnAdd = $("<div><img src='images/touchep.png'/></div>").ubwbutton({
        w: 68,
        h: 34
    })
        .mousedown(function () {
            appendToDisplay("+", 0)
        });
    var btnPaL = $("<div><img src='images/touchepg.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay("(", 1)
        });
    var btnPaR = $("<div><img src='images/touchepd.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            appendToDisplay(")", 1)
        });
    var btnEqu = $("<div><img src='images/toucheeq.png'/></div>").ubwbutton({
        w: cKeySize.w,
        h: cKeySize.h
    })
        .mousedown(function () {
            compute()
        });

    var historyPanel = $("<div id='historyPanel'></div>")
        .css({
            backgroundImage: "url(images/historyback.png)",
            width: "auto",
            height: "auto",
            float: "left",
            marginLeft: 3,
            marginRight: 2,
            marginTop: 6
        })
        .hide();

    var historyBox = $("<textarea id='historyBox'></textarea>")
        .css({
            padding: 10,
            fontSize: "20px",
            fontFamily: "Arial, Helvetica, Sans-serif",
            width: 143,
            height: 236,
            backgroundColor: "transparent",
            resize: "none",
            border: "none",
            overlay: "none",
            color: "#eeeeee"
        })
        .val("0");

    historyPanel.append(historyBox);

    var stop = $("<div></div>");
    var sleft = $("<div></div>");
    var sright = $("<div></div>");

    stop.css({
        width: 140
    })
        .append(btnAdd).append(btnSou)
        .append(btn7).append(btn8).append(btn9).append(btnMul)
        .append(btn4).append(btn5).append(btn6).append(btnDiv);

    sleft.css({
        width: 105,
        float: "left"
    })
        .append(btn1).append(btn2).append(btn3)
        .append(btn0).append(btnDot).append(btnC);

    sright.css({
        width: 45,
        float: "left"
    })
        .append(btnEqu);


    standardPanel.css({
        width: 150
    })
        .append(stop)
        .append(sleft).append(sright);

    keysPanel
        .append(display)
        .append(standardPanel);

    ubwidget
        .append(historyPanel)
        .append(keysPanel);

    if (window.sankore) {
        historyTxt = window.sankore.preference('historyTxt', historyTxt);
        var ht = window.sankore.preference('historyTab', "hidden");
        $("#historyBox").val(historyTxt);
        if (ht === "visible") {
            historyTab.trigger("click");
        };
    }

    function resizeWidget(w, h) {
        if (window.sankore) {
            window.sankore.resize(w + 2, h + 2);
        }
    }

    function compute() {
        var result;
        if (calc.length <= 1)
            result = eval(calc);
        else {
            var fNumber, lNumber, operation = "",
                fMinus = "";
            if (calc.charAt(0) == '-') {
                fMinus = "-";
                calc = calc.substr(1, calc.length);
            }
            operation = (calc.indexOf("+", 0) != -1) ? calc.charAt(calc.indexOf("+", 0)) : ((calc.indexOf("*", 0) != -1) ? calc.charAt(calc.indexOf("*", 0)) : ((calc.indexOf("/", 0) != -1) ? calc.charAt(calc.indexOf("/", 0)) : ((calc.indexOf("-", 0) != -1) ? calc.charAt(calc.indexOf("-", 0)) : "")));
            fNumber = fMinus + calc.substring(0, calc.indexOf(operation, 0));
            lNumber = calc.substring(calc.indexOf(operation, 0) + 1, calc.length);
            result = calcIt(fNumber, operation, lNumber);
        }

        $("#display").text(updateDisplay(result));
        lastResult = result;
        lastchar.type = "Nan";
        lastchar.value = "";
        calc = "";

        $("#historyBox").val($("#historyBox").val() + historyTrunk + "\n= " + updateDisplay(result) + "\n\n");
        $("#historyBox").scrollTop(99999);

        displayTrunk = "";
        historyTrunk = "";

        if (window.sankore) {
            window.sankore.setPreference('historyTxt', $("#historyBox").val());
        }

        lastHistory = $("#historyBox").val();
    }

    function calcIt(fNumber, operation, lNumber) {
        var result, fCount = "",
            lCount = "",
            length = 0;
        fCount = (fNumber.indexOf(".", 0) != -1) ? fNumber.substring(fNumber.indexOf(".", 0) + 1, fNumber.length) : "";
        lCount = (lNumber.indexOf(".", 0) != -1) ? lNumber.substring(lNumber.indexOf(".", 0) + 1, lNumber.length) : "";
        length = (fCount.length >= lCount.length) ? fCount.length : lCount.length;
        length = Math.pow(10, length);
        fNumber = Math.round(fNumber * length);
        lNumber = Math.round(lNumber * length);
        switch (operation) {
        case "+":
            result = (fNumber + lNumber) / length;
            break;
        case "-":
            result = (fNumber - lNumber) / length;
            break;
        case "*":
            result = (fNumber * lNumber) / Math.pow(length, 2);
            break;
        case "/":
            result = fNumber / lNumber;
            break;
        }
        return result;
    }

    function reset() {
        $("#display").text("0");
        calc = "";
        lastchar.type = "NaN";
        displayTrunk = "";
        historyTrunk = "";
        $("#historyBox").val(lastHistory);
        $("#historyBox").scrollTop(99999);
    }

    function formatNumberWithDelimiters(number) {
        var delimiter = gThousandsSeparator;
        var numString = number.toString();

        if (!numString)
            return "0";

        var dot = numString.indexOf(gDecimalSeparator);
        if (dot == -1)
            dot = numString.length;

        var stop = numString.length - dot;
        var characteristic = numString.substr(0, dot);
        if (!parseInt(characteristic))
            return numString;

        // see if it's a negative number
        var numIsNegative = (parseInt(characteristic) < 0)

        var newNumber = "";
        var delimiterCount = Math.floor((characteristic.length - 1) / 3);
        var extras = characteristic.length % 3;
        if (!extras && characteristic.length > 2)
            extras = 3;

        if (extras)
            newNumber = newNumber + characteristic.substr(0, extras);

        for (var i = 0; i < delimiterCount; i++) {

            if ((0 == i) && numIsNegative && (extras == 1))
                newNumber = newNumber + characteristic.substr(extras + (i * 3), 3);
            else
                newNumber = newNumber + delimiter + characteristic.substr(extras + (i * 3), 3);
        }

        return (dot ? (newNumber + numString.substr(dot, stop)) : newNumber);
    }


    function formatNumberWithScientificNotation(number) {
        if (number == 0)
            return number;
        
        var numString = number.toString();
        if (!numString)
            return "0";

        var num = new Number(numString);
        var sci = num.toExponential(5);
        if (sci == "NaN")
            sci = formatNumberWithDelimiters(numString);
        if (!sci)
            return "0";

        return sci;
    }

    function updateDisplay(number) {
        if (String(number).length > 8) {
            number = formatNumberWithScientificNotation(number);
        } else {
            number = number;
        }

        return number;
    }

    function appendToDisplay(_char, v) {
        var char = _char;

        // Display
        if ($("#historyBox").val() === "0") {
            $("#historyBox").val("");
        };

        if (char == "-" && lastchar.type == "NaN") {
            char = "m";
        };

        // char is a number 
        if (char <= 0 || char > 0 || char == "." || char == "m" || char == "(" || char == ")") {
            if (char == "m") {
                char = "-";
            };

            if (lastchar.type == "NaN") {
                displayTrunk = "";
            };

            if (lastchar.value == "." && char == ".") {
                calc = calc.substr(0, calc.length - 1);
                displayTrunk = displayTrunk.substr(0, displayTrunk.length - 1);
            };

            if (calc == "0") {
                if (char != "0") {
                    if (char != ".") {
                        calc = String(char);
                        displayTrunk = String(char);
                        historyTrunk = String(char);
                        lastchar.type = "Number";
                    } else {
                        calc += String(char);
                        displayTrunk += String(char);
                        historyTrunk += String(char);
                        lastchar.type = "Number";
                    }
                }
            } else {
                char = (char == ".") ? ((displayTrunk.indexOf(".", 0) != -1) ? "" : ".") : char;
                calc += String(char);
                displayTrunk += String(char);
                historyTrunk += String(char);
                lastchar.type = "Number";
            }
        }
        // char is an operator
        else {
            if (calc.length === 0) {
                var endCalc = calc;
                calc = lastResult + endCalc;
                historyTrunk = updateDisplay(lastResult);
            }

            if (lastchar.type == "NaN") {
                calc = calc.substr(0, calc.length - 1);
                calc += char;
                $("#historyBox").val($("#historyBox").val().substr(0, $("#historyBox").val().length - 1));
            } else {
                try {
                    calc = eval(calc) + String(char);
                } catch (e) {}
            }

            lastchar.type = "NaN";
            try {
                displayTrunk = eval(calc.substr(0, calc.length - 1));
            } catch (e) {}
            $("#historyBox").val($("#historyBox").val() + historyTrunk + char);
            $("#historyBox").scrollTop(99999);
            historyTrunk = "";
        }

        lastchar.value = char;
        $("#display").text(updateDisplay(displayTrunk));
    }

    $("#historyBox").click(function () {
        if (!clickFlag) {
            $(this).select();
            clickFlag = true;
        } else {
            clickFlag = false;
            $(this).blur();
        }
    });
    $(document).disableTextSelect();
};