/*globals klass:true, Sankore:true*/
(function() {
    "use strict";
    
    klass.define('Sankore', 'Text', klass.extend({
        constructor: function (id, screen, button, type, editable) {
            this.id = id;
            this.screen = screen;
            this.button = button;
            
            this.type = typeof type !== 'undefined' ? type : 'normal';
            this.editable = typeof editable !== 'undefined' ? !!editable : true;
        },
        
        isEditable: function () {
            return this.editable;
        },
        
        setEditable: function (editable) {
            this.editable = !!editable;
        }
    }));
})();