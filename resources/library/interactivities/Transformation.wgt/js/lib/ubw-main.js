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
function log(object) {
    console.log(object);
}

function initAfterI18nMessagesLoaded(reload, templates, callbacks) {
    document.title = fr.njin.i18n.document.title;
	
    var ubwidget = $("#ubwidget");
	
    var parameters = Object.create(Parameters,{
        container: {
            value: ubwidget
        },
        delegate: {
            value: window.sankore || Object.create(ParametersDelegate)
        }
    });
	
    var app = Object.create(App, {
        container: {
            value: ubwidget
        },
        parameters: {
            value: parameters
        },
        reload: {
            value: reload
        }
    });
    if(!window.sankore.preference("trans","")){
        app.parameters.value("#Picture0before", JSON.stringify({
            alt: "coq.png", 
            src: "images/coq.png", 
            title: "coq.png"
        }))
        app.parameters.value("#Picture0after", JSON.stringify({
            alt: "poule.png", 
            src: "images/poule.png", 
            title: "poule.png"
        }))
        app.parameters.value("#Picture1before", JSON.stringify({
            alt: "boeuf.png", 
            src: "images/boeuf.png", 
            title: "boeuf.png"
        }))
        app.parameters.value("#Picture1after", JSON.stringify({
            alt: "vache.png", 
            src: "images/vache.png", 
            title: "vache.png"
        }))
        app.parameters.value("#UsePicture0before", true)
        app.parameters.value("#UsePicture0after", true)
        app.parameters.value("#UsePicture1before", true)
        app.parameters.value("#UsePicture1after", true)
    }
    app.init();
    app.onEdit = false;	
    //e("#Picture"+key, JSON.stringify(f));
        
        
    if(templates.toolbar) {
        $("#toolbar").html(Mustache.render(templates.toolbar, window));
    }
    if(templates.parameters) {
        $("#parameters").html(Mustache.render(templates.parameters, window));
    }
	
    if(callbacks.onTemplatesLoaded && typeof callbacks.onTemplatesLoaded === 'function') {
        callbacks.onTemplatesLoaded(app);
    }
	
    log("Update setting views with stored parameters");
    $("#parameters (input|select)[role=parameter]").each(function(i, input) {
        (function(input){
            var key = input.name;
            var type = input.type;
            var isRadioOrcheckbox = (type !== undefined && (type === "checkbox" || type === "radio"));
            var stored = parameters.value(key);
            if(isRadioOrcheckbox)
                $(input).attr("checked", (typeof stored === "string" ? stored === "true" : stored));
            else
                $(input).val(stored);
            $(input).change(function(){
                var val = (isRadioOrcheckbox ? $(this).is(":checked") : $(this).val());
                parameters.value(key, val);
            });
        })(input);
    });
	
    log("Toobar Initialisation");
    $("button[role=edit]").click(function(){
        app.onEdit = true;
        $(document.body).addClass("onEdit");
        if(callbacks.onEdit && typeof callbacks.onEdit === 'function') {
            callbacks.onEdit(app);
        }
    });
    $("button[role=view]").click(function(){
        app.onEdit = false;		
        $(document.body).removeClass("onEdit");
        if(callbacks.onView && typeof callbacks.onView === 'function') {
            callbacks.onView(app);
        }
    });
    $("button[role=reload]").click(function(){
        app.reload();
    });
    $("button[role=help]").click(function(){
        $("body").toggleClass("showHelp");
    });
	
    $("select[name='themes']").change(function() {
        $("body").get(0).className = $("body")[0].className.replace(/\btheme-[^\s]*\b/gi, '');
        $("body").addClass("theme-"+$(this).val());
    });
    $("body").addClass("theme-"+$("select[name='themes']").val());
}

function init(reload, templates, callbacks){
    var locale = window.sankore ? sankore.locale() : "";
    $.i18n.properties({
        name: 'Messages', 
        path: 'i18n/',
        language: locale,
        callback: function(){ 
            initAfterI18nMessagesLoaded(reload, templates, callbacks);
        }
    });
}

var Parameters = (function(){
    var self = Object.create({}, {
        container: {
            value: null
        },
        delegate: {
            value: null
        },
        value: {
            value: function(key, value) {
                if(value === undefined) {
                    var val = this.delegate.preference(key);
                    log("Retrieve parameter value ["+val+"] as type ["+(typeof val)+"] for key : ["+key+"]");
                    return (val !== undefined && (typeof val !== "string" || val)) ? val : $("#ubwidget").data(key);
                }
                else {
                    log("Set parameter value ["+value+"] for key : ["+key+"]");
                    this.delegate.setPreference(key, value);
                    this.container.trigger("preferenceChange", {
                        key: key, 
                        value: value
                    });					
                }
            }
        }
    });
    return self;
})();

var ParametersDelegate = (function(){
    var self = Object.create({}, {
        preference: {
            value: function(key) {
                return this[key];
            }
        },
        setPreference: {
            value: function(key, value) {
                this[key] = value;
            }
        }
    });
    return self;
})();

var App = (function() {
    var self = Object.create({}, {
        container: {
            value: null
        },
        parameters: {
            value: null
        },
        init: {
            value: function() {
                var that = this;
                this.container.bind("preferenceChange", function(evt, parameter) {
                    that.reload(parameter);
                });
                this.reload();
            }
        },
        reload: {
            value: function(parameter) {
                if(parameter === undefined)
                    log("Reload");
                else
                    log("Update");
            }
        },
        utils: {
            value: Object.create({}, {
                shuffle: {
                    value: function(array) {
                        var unpickedSize = array.length;
                        while(unpickedSize > 0) {
                            var n = Math.floor(Math.random()*unpickedSize);
                            var temp = array[unpickedSize-1];
                            array[unpickedSize-1] = array[n];
                            array[n] = temp;
                            unpickedSize--;
                        }
                    }
                },
                guid: {
                    value: function() {
                        var S4 = function() {
                            return (((1+Math.random())*0x10000)|0).toString(16).substring(1);
                        };
                        return (S4()+S4()+"-"+S4()+"-"+S4()+"-"+S4()+"-"+S4()+S4()+S4());
                    }
                },
                droppable: {
                    value: function($e, callback) {							
                        $e.bind("dragover", function(){
                            $(this).addClass("hover");
                            return false;
                        });
                        $e.bind("dragleave", function(){
                            $(this).removeClass("hover");
                            return false;
                        });
                        $e.bind("drop", function(e){
                            $(this).removeClass("hover");
                            e = e || window.event;
                            e.preventDefault();
                            // jQuery wraps the originalEvent, so we try to detect that here...
                            e = e.originalEvent || e;
							
                            if(window.sankore) {
                                function stringToXML(text){
                                    if (window.ActiveXObject){
                                        var doc=new ActiveXObject('Microsoft.XMLDOM');
                                        doc.async='false';
                                        doc.loadXML(text);
                                    } else {
                                        var parser=new DOMParser();
                                        doc=parser.parseFromString(text,'text/xml');
                                    }
                                    return doc;
                                }
                                var file = stringToXML(e.dataTransfer.getData("text/plain"));
                                callback({
                                    src: $(file).find("path:eq(0)").text()
                                });
                                return false;
                            }

                            // Using e.files with fallback because e.dataTransfer is immutable and can't be overridden in Polyfills (http://sandbox.knarly.com/js/dropfiles/).
                            var files = (e.files || e.dataTransfer.files);
                            for (var i = 0; i < files.length; i++) {
                                // Loop through our files with a closure so each of our FileReader's are isolated.
                                (function (i) {
                                    if(typeof FileReader !== "undefined") {
                                        var reader = new FileReader();
                                        reader.onload = function (event) {
                                            callback({
                                                src: event.target.result,
                                                title: (files[i].name),
                                                alt: (files[i].name)
                                            });
                                        };
                                        reader.readAsDataURL(files[i]);
                                    }
                                })(i);
                            }
                            return false;
                        });
                    }
                }
            })
        }
    });
    return self;
})();