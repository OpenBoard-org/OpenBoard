/*jshint plusplus: true*/
/*global klass: true, Sankore: true*/
(function () {
    "use strict";

    klass.define('Sankore.Calculus', 'Engine', klass.extend({
        constructor: function () {
            this.expressions = [];
            this.operators = [];
        },

        init: function () {
            this.expressions = [];
            this.operators = [];
        },

        evaluate: function (expressionString) {
            var tokens = [],
                lastToken,
                penultimateToken,
                item;
            
            expressionString = expressionString.replace(/\s+/g, '');
            
            for (var i in expressionString) {
                item = expressionString[i];
                
                if (tokens.length > 0) {
                    lastToken = tokens[tokens.length - 1];
                } else {
                    lastToken = undefined;
                }

                if (tokens.length > 1) {
                    penultimateToken = tokens[tokens.length - 2];
                } else {
                    penultimateToken = undefined;
                }
                
                if ('0123456789.'.indexOf(item) !== -1) {
                    if (
                        !isNaN(Number(lastToken)) ||
                        lastToken === '.' ||
                        lastToken === '-.' ||
                        (
                            lastToken === '-' && 
                            (
                                penultimateToken === '(' || 
                                penultimateToken === undefined
                            )
                        )
                    ) {
                        tokens[tokens.length - 1] += item;
                    } else {
                        tokens.push(item);
                    }
                } else {
                    tokens.push(item);
                }
            }
            
            for (var j in tokens) {
                if (tokens[j].length > 1 && tokens[j].charAt(tokens[j].length - 1) === '.') {
                    throw Sankore.Util.Error.create('InvalidExpression', 'Trailing comma'); 
                }
            }
            
            return this.computeExpression(tokens);
        },

        computeExpression: function (tokens) {
            var operatorCheck = function (stack, token) {
                    var prec = Sankore.Calculus.BinaryOperation.getOperatorPrecedence,
                        top;

                    while (true) {
                        top = stack.operators[stack.operators.length - 1];

                        if (stack.operators.length === 0 || top === '(' || prec(token) > prec(top)) {
                            return stack.operators.push(token);
                        }

                        stack.reduce();
                    }
                };

            this.init();

            for (var i in tokens) {
                switch (tokens[i]) {
                case '(':
                    this.operators.push(tokens[i]);
                    break;

                case ')':
                    if (this.operators.length === 0) {
                        throw Sankore.Util.Error.create('InvalidExpression', 'Trailing closing brackets');
                    }

                    while (this.operators.length !== 0 && this.operators[this.operators.length - 1] !== '(') {
                        this.reduce();
                    }

                    if (this.operators[this.operators.length - 1] === '(') {
                        this.operators.pop(); // get rid of the extra paren '('
                    }

                    break;

                case '+':
                case '-':
                case '*':
                case '/':
                case ':':
                    operatorCheck(this, tokens[i]);
                    break;

                default:
                    this.expressions.push(Sankore.Calculus.Operand.create(tokens[i]));
                }
            }

            while (this.operators.length !== 0) {
                this.reduce();
            }

            // if there's not one and only one expression in the stack, the expression is invalid
            if (this.expressions.length !== 1) {
                throw Sankore.Util.Error.create('InvalidExpression', '"' + tokens.join(' ') + '" is not a valid expression');
            }

            return this.expressions.pop();
        },

        reduce: function () {
            var right = this.expressions.pop(),
                left = this.expressions.pop(),
                operator = this.operators.pop(),
                operation;

            if (typeof operator === 'undefined' || typeof left === 'undefined' || typeof right === 'undefined') {
                throw Sankore.Util.Error.create('InvalidExpression', 'Invalid expression');
            }

            if (operator === ':') {
                operation = Sankore.Calculus.EuclideanDivisionOperation.create(left, right);
            } else {
                operation = Sankore.Calculus.BinaryOperation.create(left, operator, right);
            }

            this.expressions.push(operation);
        }

    }));
})();