/*global klass:true, Sankore:true */
(function () {
    "use strict";

    /**
     * Base class for expression
     */
    klass.define('Sankore.Calculus', 'Expression', klass.extend({
        constructor: function () {

        },

        getValue: function () {
            return null;
        },
        
        isInteger: function () {
            try {
                var value = this.getValue();
                
                return value === Math.floor(value);
            } catch (e) {
                return 0;
            }
        },

        toString: function () {
            return '';
        },

        isCompound: function () {
            return false;
        }
    }));

    /**
     * Calculus operand
     */
    klass.define('Sankore.Calculus', 'Operand', Sankore.Calculus.Expression.extend({
        constructor: function (value) {
            this.value = Number(value);

            if (isNaN(this.value)) {
                throw Sankore.Util.Error.create('InvalidNumber', '"' + String(value) + '" is a not a number');
            }
        },

        getValue: function () {
            return this.value;
        },

        toString: function () {
            return String(this.value);
        }
    }));

    /**
     * Unary operator (+, -)
     */
    klass.define('Sankore.Calculus', 'Operation', Sankore.Calculus.Expression.extend({
        constructor: function (operator, right) {
            this.operator = operator;

            if (!Sankore.Calculus.Expression.isPrototypeOf(right)) {
                right = Sankore.Calculus.Operand.create(right);
            }

            this.right = right;
        },

        getPrecedence: function () {
            return Sankore.Calculus.Operation.getOperatorPrecedence(this.operator);
        },

        isLeftAssociative: function () {
            return false;
        },

        isCompound: function () {
            return true;
        },

        getValue: function () {
            var value = Number(this.right.getValue());

            if (this.operator === '-') {
                value *= -1;
            }

            return value;
        },

        toString: function () {
            var string = this.right.toString();

            if (this.operator !== '-') {
                return string;
            }

            if (this.right.isCompound()) {
                string = '(' + string + ')';
            }

            return '-' + string;
        }
    }));

    Sankore.Calculus.Operation.getOperatorPrecedence = function (operator) {
        switch (operator) {
        case '+':
        case '-':
            return 1;
        case '*':
        case '/':
        case ':':
            return 2;
        }
    };

    /**
     * Binary operator (+, -, *, /)
     */
    klass.define('Sankore.Calculus', 'BinaryOperation', Sankore.Calculus.Operation.extend({
        constructor: function (left, operator, right) {
            Sankore.Calculus.Operation.constructor.call(this, operator, right);

            if (!Sankore.Calculus.Expression.isPrototypeOf(left)) {
                left = Sankore.Calculus.Operand.create(left);
            }

            this.left = left;
        },

        isLeftAssociative: function () {
            return true;
        },

        getValue: function () {
            var leftValue = this.left.getValue(),
                rightValue = this.right.getValue();

            switch (this.operator) {
            case '+':
                return leftValue + rightValue;

            case '-':
                return leftValue - rightValue;

            case '*':
                return leftValue * rightValue;

            case '/':
                if (0 === rightValue) {
                    throw Sankore.Util.Error.create('ZeroDivision', 'Division by zero');
                }

                return leftValue / rightValue;

            default:
                throw Sankore.Util.Error.create('InvalidOperator', 'This is not a valid operator : ' + this.operator);
            }
        },

        toString: function () {
            if (this.isInteger()) {
                return String(this.getValue());
            }
            
            var leftString = this.left.toString(),
                rightString = this.right.toString(),
                string = '';

            if (Sankore.Calculus.Operation.isPrototypeOf(this.left)) {
                if (this.left.getPrecedence() < this.getPrecedence()) {
                    leftString = '(' + leftString + ')';
                }
            }

            if (Sankore.Calculus.Operation.isPrototypeOf(this.right)) {
                if (this.right.getPrecedence() < this.getPrecedence()) {
                    rightString = '(' + rightString + ')';
                }
            }

            return leftString + String(this.operator) + rightString;
        }
    }));

    /**
     * Euclidean division operator
     */
    klass.define('Sankore.Calculus', 'EuclideanDivisionOperation', Sankore.Calculus.BinaryOperation.extend({
        constructor: function (left, right) {
            Sankore.Calculus.BinaryOperation.constructor.call(this, left, ':', right);
        },

        getValue: function () {
            var rightValue = this.right.getValue();
            
            if (0 === rightValue) {
                throw Sankore.Util.Error.create('ZeroDivision', 'Division by zero');
            }
            
            return Math.floor(this.left.getValue() / rightValue);
        },

        getRemainder: function () {
            var rightValue = this.right.getValue();
            
            if (0 === rightValue) {
                throw Sankore.Util.Error.create('ZeroDivision', 'Division by zero');
            }
            
            return this.left.getValue() % rightValue;
        }
    }));
})();