/*globals klass:true Sankore:true*/
(function(){
    "use strict";
    
    klass.define('Sankore.Util', 'Error', klass.extend({
        constructor: function (name, message) {
            this.name = name;
            this.message = message;
        },
        
        toString: function () {
            return this.name + ': ' + this.message;
        }
    }));
})();