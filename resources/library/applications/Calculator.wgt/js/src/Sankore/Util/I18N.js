/*jshint devel:true, browser:true*/
/*globals klass:true, Sankore:true*/
(function () {
    "use strict";

    klass.define('Sankore.Util', 'I18N', klass.extend({
        
        catalogs: Sankore.Util.Hash.create(),
        
        constructor: function () {
            this.catalog = {};
        },
        
        load: function (locale) {
            var localeId = locale.split(/-|_/)[0].toLowerCase();
            
            if (!Sankore.Util.I18N.catalogs.has(localeId)) {
                localeId = 'en';
            }
            
            this.catalog = Sankore.Util.I18N.catalogs.get(localeId, {});
        },

        translate: function (id) {
            return id.split('.').reduce(
                function (root, id) {
                    if (root && id in root) {
                        return root[id];
                    }
                     
                    return null;
                }, 
                this.catalog
            ) || id;
        }
    }));
    
    // global instance
    Sankore.Util.i18n = Sankore.Util.I18N.create();
    
    // global helper
    window._ = function (id) {
        return Sankore.Util.i18n.translate(id);
    };
})();