/*global klass:true, Sankore:true*/
(function() {
    "use strict";
    
    klass.define('Sankore', 'Button', klass.extend({
        constructor: function (text, command, useLimit, editable) {
            this.text = text;
            this.command = command;
    
            this.useLimit = typeof useLimit === 'undefined' ? -1 : useLimit;
            this.editable = typeof editable === 'undefined' ? true : editable;
        },
        
        isEditable: function () {
            return this.editable;
        },
    
        isUsable: function () {
            return this.useLimit === -1;
        },
        
        isDisabled: function () {
            return this.useLimit === 0;
        },
        
        clone: function () {
            return Sankore.Button.create(
                this.text,
                this.command,
                this.useLimit,
                this.editable
            );
        }
    }));
})();