/*global klass:true, Sankore:true*/
(function () {
    "use strict";

    klass.define('Sankore.Editor', 'Layout', klass.extend({
        constructor: function (data) {
            this.id = data.id || null;
            this.name = data.name || null;
            this.description = data.description || null;
            this.editable = false;
            this.buttonMap = data.buttonMap || {};
        },

        setEditable: function (editable) {
            this.editable = !! editable;
        },

        isEditable: function () {
            return this.editable;
        },

        getButton: function (slot) {
            return this.buttonMap[slot] || null;
        },

        clone: function () {
            var clonedMap = {};
            
            for (var index in this.buttonMap) {
                if (this.buttonMap.hasOwnProperty(index)) {
                    clonedMap[index] = this.buttonMap[index].clone();
                }
            }
            
            return Sankore.Editor.Layout.create({
                id: this.id,
                name: this.name,
                description: this.description,
                editable: this.editable,
                buttonMap: clonedMap
            });
        }
    }));
})();