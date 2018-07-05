/*globals klass: true, Sankore: true*/
(function() {
    "use strict";
    
    klass.define('Sankore.Util', 'Hash', klass.extend({
        constructor: function (elements) {
            this.elements = elements || {};
        },

        length: function () {
            return this.keys().length;
        },

        keys: function () {
            return Object.keys(this.elements);
        },

        set: function (id, value) {
            this.elements[id] = value;
        },
        
        add: function (id, values) {
            for (var i in values) {
                this.set(values[i][id], values[i]);
            }
        },
        
        has: function (id) {
            return this.keys().indexOf(id) !== -1;
        },

        get: function (id, def) {
            if (typeof this.elements[id] !== 'undefined') {
                return this.elements[id];
            }

            if (typeof def !== 'undefined') {
                return def;
            }

            return null;
        },
        
        pos: function (id) {
            var pos = 0;
            
            for (var i in this.elements) {
                if (this.elements.hasOwnProperty(i) && i === id) {
                    return pos;
                }
                
                pos++;
            }
            
            return null;
        },
        
        remove: function (id) {
            return delete this.elements[id];
        },

        map: function (closure) {
            var output = [],
                called;

            for (var id in this.elements) {
                if (this.elements.hasOwnProperty(id)) {
                    called = closure.call(this, id, this.elements[id]);
                    if (called) {
                        output.push(called);
                    }
                }
            }

            return output;
        }
    }));
})();