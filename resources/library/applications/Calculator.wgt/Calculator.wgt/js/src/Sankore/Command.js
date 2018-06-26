/*global klass:true, Sankore:true*/
(function () {
    "use strict";

    klass.define('Sankore', 'Command', klass.extend({
        constructor: function (id, name, closure) {
            this.id = id;
            this.name = name;
            this.closure = closure;
        },

        getId: function () {
            return this.id;
        },

        getName: function () {
            return this.name;
        },

        exec: function (scope, args) {
            this.closure.call(scope, args);
        },

        isInterrupting: function () {
            return false;
        },

        isInternal: function () {
            return false;
        }
    }));

    klass.define('Sankore', 'InterruptingCommand', Sankore.Command.extend({
        constructor: function (id, name, closure) {
            Sankore.Command.constructor.call(this, id, name, closure);
        },

        isInterrupting: function () {
            return true;
        }
    }));

    klass.define('Sankore', 'InternalCommand', Sankore.Command.extend({
        constructor: function (id, name, closure) {
            Sankore.Command.constructor.call(this, id, name, closure);
        },

        isInternal: function () {
            return true;
        }
    }));
})();