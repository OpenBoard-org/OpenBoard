/*jshint browser:true, devel:true */
/*global klass:true, Sankore:true, _:true*/
(function () {
    "use strict";

    klass.define('Sankore.Editor', 'Editor', klass.extend({
        constructor: function (calculator) {
            
            // state attributes
            this.current = null;
            this.activeButton = null;
            this.enabled = false;
            this.layouts = Sankore.Util.Hash.create({
                'default': Sankore.Editor.Layout.create({
                    id: 'default', 
                    name: _('layout.classic_name'),
                    buttonMap: {
                        a1: Sankore.Button.create('mr', 'memoryRecall'),
                        b1: Sankore.Button.create('mc', 'memoryClear'),
                        c1: Sankore.Button.create('m+', 'memoryAdd'),
                        d1: Sankore.Button.create('m-', 'memorySub'),

                        a2: Sankore.Button.create('op', 'op'),
                        b2: Sankore.Button.create('(', '('),
                        c2: Sankore.Button.create(')', ')'),
                        d2: Sankore.Button.create(':', ':'),

                        a3: Sankore.Button.create('7', '7'),
                        b3: Sankore.Button.create('8', '8'),
                        c3: Sankore.Button.create('9', '9'),
                        d3: Sankore.Button.create('/', '/'),

                        a4: Sankore.Button.create('4', '4'),
                        b4: Sankore.Button.create('5', '5'),
                        c4: Sankore.Button.create('6', '6'),
                        d4: Sankore.Button.create('*', '*'),

                        a5: Sankore.Button.create('1', '1'),
                        b5: Sankore.Button.create('2', '2'),
                        c5: Sankore.Button.create('3', '3'),
                        d5: Sankore.Button.create('-', '-'),

                        a6: Sankore.Button.create('0', '0'),
                        b6: Sankore.Button.create('.', '.'),
                        c6: Sankore.Button.create('=', '='),
                        d6: Sankore.Button.create('+', '+')
                    }
                })
            });
            
            // components
            this.calculator = calculator;
            this.ui         = Sankore.UI.EditorInterface.create(this, this.calculator.eventDispatcher);
            
            this.attachEventHandlers();
        },
        
        attachEventHandlers: function () {
            var self = this, ed = this.calculator.eventDispatcher;
            
            // click on Add button
            ed.addEventListener('editor_interface.add_click', function () {
                var clone = self.createLayout();

                self.setCurrentLayout(clone.id);
            });

            // click on Remove button
            ed.addEventListener('editor_interface.remove_click', function () {
                self.removeLayout(self.current);

                self.setCurrentLayout('default');
            });

            // click on Run button
            ed.addEventListener('editor_interface.run_click', this.runCurrentLayout.bind(this));

            // load the new selected layout
            ed.addEventListener('editor_interface.layout_select', function (layoutId) {
                self.setCurrentLayout(layoutId);
            });

            // the layout name has changed
            ed.addEventListener('editor_interface.layout_name_change', function (name) {
                if (self.getCurrentLayout().name !== name && name.trim().length > 0) {
                    self.getCurrentLayout().name = name;
                    ed.notify('editor.layout_changed');
                }
            });

            // the layout description has changed
            ed.addEventListener('editor_interface.layout_description_change', function (description) {
                if (self.getCurrentLayout().description !== description) {
                    self.getCurrentLayout().description = description;
                    ed.notify('editor.layout_changed');
                }
            });

            // the command of a button has changed
            ed.addEventListener('editor_interface.button_command_change', function (command) {
                if (self.activeButton) {
                    self.getCurrentLayout().getButton(self.activeButton).command = command;
                    ed.notify('editor.layout_changed');
                }
            });

            // the text of a button has changed
            ed.addEventListener('editor_interface.button_text_change', function (text) {
                if (self.activeButton) {
                    var button = self.getCurrentLayout().getButton(self.activeButton);
                    button.text = text;

                    ed.notify('editor.button_renamed', {
                        slot: self.activeButton,
                        button: button
                    });

                    ed.notify('editor.layout_changed');
                }
            });

            // the use limit of a button has changed
            ed.addEventListener('editor_interface.button_uselimit_change', function (limit) {
                if (self.activeButton) {
                    if (!isNaN(Number(limit))) {
                        self.getCurrentLayout().getButton(self.activeButton).useLimit = limit.length === 0 ? -1 : Number(limit);
                        ed.notify('editor.layout_changed');
                    }
                }
            });

            // a button is clicked
            ed.addEventListener('main_interface.button_click', function (event) {
                if (self.enabled) {
                    self.setActiveButton(event.slot);
                }
            });

            // the editor button is click
            ed.addEventListener('main_interface.editor_click', function () {
                if (self.enabled) {
                    self.runCurrentLayout();
                } else {
                    self.enable();
                }
            });

            // the editor button is click
            ed.addEventListener('main_interface.reset_click', function () {
                if (self.enabled) {
                    self.resetActiveButton();
                }
            });
        },

        init: function (state) {
            var self = this;

            if ('layouts' in state) {
                this.loadLayouts(state.layouts);
            }
            
            if ('enabled' in state) {
                this.enabled = state.enabled;
            }
            
            this.ui.render(this.calculator.ui);

            this.setCurrentLayout(('current' in state && state.current) ? state.current : 'default');
            
            if (this.enabled) {
                this.enable();
            }
            
            if ('activeButton' in state && this.enabled) {
                this.setActiveButton(state.activeButton);
            }
        },
        
        getState: function () {
            return {
                current: this.current,
                activeButton: this.activeButton,
                enabled: this.enabled,
                layouts: this.layouts.map(function (id, layout) {
                    if (layout.isEditable()) {
                        return layout;
                    }
                })
            };
        },

        loadLayouts: function (layouts) {
            var buttonMap = {},
                layout;
            
            for (var i in layouts) {
                for (var slot in layouts[i].buttonMap) {
                    if (layouts[i].buttonMap.hasOwnProperty(slot)) {
                        buttonMap[slot] = Sankore.Button.create(
                            layouts[i].buttonMap[slot].text, 
                            layouts[i].buttonMap[slot].command, 
                            layouts[i].buttonMap[slot].useLimit
                        );
                    }
                }
                
                layout = Sankore.Editor.Layout.create({
                    id: layouts[i].id,
                    name: layouts[i].name,
                    description: layouts[i].description,
                    buttonMap: buttonMap
                });
                
                layout.setEditable(true);
                
                this.layouts.add('id', [layout]);
            }
        },

        getCurrentLayout: function () {
            if (null === this.current) {
                return null;
            }

            return this.layouts.get(this.current);
        },

        setCurrentLayout: function (id) {
            this.current = id;

            this.calculator.eventDispatcher.notify('editor.layout_selected', this.getCurrentLayout());

            this.resetActiveButton();
        },

        createLayout: function () {
            var clone = this.layouts.get('default').clone();

            clone.id = this.generateId();
            clone.name = _('layout.new_name');
            clone.setEditable(true);

            this.layouts.set(clone.id, clone);
            this.calculator.eventDispatcher.notify('editor.layout_created');

            return clone;
        },

        /**
         * don't try to understand the purpose of this method, it just generates an unique string based upon the current widget url and current time
         */
        generateId: function () {
            var values = '',
                date = new Date(),
                id = 0,
                i;

            for (i = 0; i < document.URL.length; i++) {
                values += String(document.URL.charCodeAt(i) * (date.getMilliseconds() + date.getSeconds() + date.getMinutes()));
            }

            values = values.match(/.{1,10}/g);

            for (i in values) {
                id += Number(values[i]);
            }

            return id.toString(36);
        },

        removeLayout: function (id) {
            if (confirm(_('editor.remove_alert'))) {
                this.layouts.remove(id);

                this.calculator.eventDispatcher.notify('editor.layout_removed');
            }
        },

        setActiveButton: function (slot) {
            if (slot && this.getCurrentLayout().isEditable()) {
                var button = this.getCurrentLayout().getButton(slot);

                if (button.isEditable()) {
                    this.calculator.eventDispatcher.notify('editor.button_selected', {
                        slot: slot,
                        button: button,
                        previousSlot: this.activeButton
                    });

                    this.activeButton = slot;
                }
            } else {
                this.resetActiveButton();
            }
        },

        resetActiveButton: function () {
            this.calculator.eventDispatcher.notify('editor.button_selected', {
                slot: null,
                button: null,
                previousSlot: this.activeButton
            });

            this.activeButton = null;
        },

        enable: function () {
            this.enabled = true;
            this.setActiveButton(null);
            this.calculator.eventDispatcher.notify('editor.show');
        },

        disable: function () {
            this.enabled = false;
            this.setActiveButton(null);
            this.calculator.eventDispatcher.notify('editor.hide');
        },

        getAssignableCommands: function () {
            return this.calculator.commands.map(function (k, v) {
                if (!v.isInternal()) {
                    return v;
                }
            });
        },

        getAssignableTexts: function () {
            return this.calculator.texts.map(function (k, v) {
                if (v.isEditable()) {
                    return v;
                }
            });
        },

        runCurrentLayout: function () {
            this.disable();
        }

    }));
})();