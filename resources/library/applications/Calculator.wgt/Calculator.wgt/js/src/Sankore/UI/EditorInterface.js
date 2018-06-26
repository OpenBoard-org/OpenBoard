/*jshint browser:true*/
/*global klass:true, Sankore: true, _:true*/
(function () {
    "use strict";

    klass.define('Sankore.UI', 'EditorInterface', klass.extend({
        constructor: function (editor, ed) {
            this.editor = editor;
            this.dispatcher = ed;
            this.hidden = true;

            this.editArea = null;
            this.layoutSelect = null;
            this.layoutNameInput = null;
            this.layoutDescriptionInput = null;
            this.assignationDiv = null;
            this.runButton = null;
            this.addButton = null;
            this.removeButton = null;

            this.attachEventListeners();

            this.rendered = false;
        },

        attachEventListeners: function () {
            var self = this;

            this.dispatcher.addEventListener('editor.show', this.show.bind(this));
            this.dispatcher.addEventListener('editor.hide', this.hide.bind(this));

            this.dispatcher.addEventSubscriber({
                events: ['editor.layout_changed', 'editor.layout_created', 'editor.layout_removed'],
                listener: this.updateLayoutSelectElement.bind(this)
            });

            this.dispatcher.addEventListener('editor.layout_selected', function (layout) {
                self.loadLayout(layout);

                self.selectLayout(layout.id);
            });

            this.dispatcher.addEventListener('editor.button_selected', function (e) {
                var context = e.button;

                if (null === context && self.editor.getCurrentLayout()) {
                    context = self.editor.getCurrentLayout().isEditable();
                }

                self.renderAssignation(context);
            });
        },

        _clearElement: function (element) {
            while (element.firstChild) {
                element.removeChild(element.firstChild);
            }
        },

        _map: function (iterable, callback) {
            var mapped = [],
                idx;

            for (idx in iterable) {
                if (iterable.hasOwnProperty(idx)) {
                    mapped.push(callback.call(iterable, iterable[idx], idx));
                }
            }

            return mapped;
        },

        render: function (mainInterface) {
            var layoutNameLabel,
                layoutDescriptionLabel,
                assignationDiv,
                assignationText,
                self = this;

            this.editArea = document.createElement('div');
            this.editArea.classList.add('edit-area');

            this.layoutSelect = this.createLayoutSelectElement();
            this.editArea.appendChild(this.layoutSelect);

            this.addButton = document.createElement('button');
            this.addButton.className = 'small';
            this.addButton.setAttribute('type', 'button');
            this.addButton.addEventListener('click', function (e) {
                self.dispatcher.notify('editor_interface.add_click');
            });

            this.removeButton = this.addButton.cloneNode();
            this.removeButton.classList.add('remove');
            this.removeButton.addEventListener('click', function (e) {
                self.dispatcher.notify('editor_interface.remove_click');
            });

            this.addButton.classList.add('add');

            this.editArea.appendChild(this.addButton);
            this.editArea.appendChild(this.removeButton);

            this.editArea.appendChild(document.createElement('hr'));

            layoutNameLabel = document.createElement('label');
            layoutNameLabel.appendChild(document.createTextNode(_('editor.layout_name.label')));
            this.editArea.appendChild(layoutNameLabel);

            this.layoutNameInput = document.createElement('input');
            this.layoutNameInput.setAttribute('name', 'layout_name');
            this.layoutNameInput.setAttribute('type', 'text');
            this.layoutNameInput.setAttribute('maxlength', 32);
            this.layoutNameInput.addEventListener('keyup', function (e) {
                self.dispatcher.notify('editor_interface.layout_name_change', this.value);
            });
            this.layoutNameInput.addEventListener('change', function (e) {
                if (this.value.trim().length === 0) {
                    this.value = self.editor.getCurrentLayout().name;
                    self.dispatcher.notify('editor_interface.layout_name_change', this.value);
                }
            });
            this.editArea.appendChild(this.layoutNameInput);

            layoutDescriptionLabel = document.createElement('label');
            layoutDescriptionLabel.appendChild(document.createTextNode(_('editor.layout_description.label')));
            this.editArea.appendChild(layoutDescriptionLabel);

            this.layoutDescriptionInput = document.createElement('textarea');
            this.layoutDescriptionInput.setAttribute('name', 'layout_description');
            this.layoutDescriptionInput.setAttribute('maxlength', 140);
            this.layoutDescriptionInput.addEventListener('keyup', function(e) {
                self.dispatcher.notify('editor_interface.layout_description_change', this.value);
            });
            this.editArea.appendChild(this.layoutDescriptionInput);

            this.assignationDiv = document.createElement('div');
            this.assignationDiv.classList.add('assignation');

            this.editArea.appendChild(this.assignationDiv);

            this.runButton = document.createElement('button');
            this.runButton.classList.add('run');
            this.runButton.setAttribute('type', 'button');
            this.runButton.appendChild(document.createTextNode(_('editor.run_button')));
            this.runButton.addEventListener('click', function (e) {
                self.dispatcher.notify('editor_interface.run_click');
            });

            this.editArea.appendChild(this.runButton);

            mainInterface.rearScreen.parentElement.appendChild(this.editArea);

            this.rendered = true;
            
            this.hide();
        },

        createSelectElement: function (data, name, className, selectedValue) {
            var select = document.createElement('select'),
                option;

            select.setAttribute('name', name);

            if (className) {
                select.className = className;
            }

            for (var i in data) {
                option = document.createElement('option');
                option.setAttribute('value', data[i].value);

                if (typeof selectedValue !== 'undefined' && selectedValue === data[i].value) {
                    option.selected = true;
                }

                option.appendChild(document.createTextNode(data[i].text));

                select.appendChild(option);
            }

            return select;
        },

        createLayoutSelectElement: function () {
            var select = this.createSelectElement(
                    this.editor.layouts.map(function (k, layout) {
                        return {
                            text: layout.name,
                            value: layout.id
                        };
                    }), 
                    'layouts', 
                    'layout-select', 
                    this.editor.current
                ),
                self = this;

            select.addEventListener('change', function (e) {
                self.dispatcher.notify('editor_interface.layout_select', this.value);
            });

            return select;
        },

        updateLayoutSelectElement: function () {
            var select = this.createLayoutSelectElement();

            this.editArea.replaceChild(select, this.layoutSelect);

            this.layoutSelect = select;
        },
        
        selectLayout: function (selected) {
            this.layoutSelect.value = selected;
        },

        show: function () {
            if (this.rendered) {
                this.editArea.style.display = 'block';
                this.hidden = false;
            }
        },

        hide: function () {
            if (this.rendered) {
                this.editArea.style.display = 'none';
                this.hidden = true;
            }
        },

        loadLayout: function (layout) {
            this.layoutNameInput.value = layout.name;
            this.layoutNameInput.disabled = !layout.isEditable();

            this.layoutDescriptionInput.value = layout.description;
            this.layoutDescriptionInput.disabled = !layout.isEditable();

            this.removeButton.disabled = !layout.isEditable();

            this.renderAssignation(layout.isEditable());
        },

        renderAssignation: function (context) {
            var innerEl, textLabel, textSelect, commandLabel, commandSelect, useLabel, useInput, useHelp,
                text,
                self = this;

            if (false === context) {
                innerEl = document.createElement('em');
                innerEl.appendChild(document.createTextNode(_('editor.assignation.disabled')));
            } else if (Sankore.Button.isPrototypeOf(context)) {
                innerEl = document.createDocumentFragment();

                textLabel = document.createElement('label');
                textLabel.appendChild(document.createTextNode(_('editor.assignation.text.label')));
                innerEl.appendChild(textLabel);

                text = this.editor.calculator.texts.get(context.text);
                
                textSelect = this.createSelectElement(
                    this._map(this.editor.getAssignableTexts(), function (text) {
                        return {
                            text: text.button,
                            value: text.id
                        }; 
                    }),
                    'button_text', 
                    '',
                    text.id
                );
                textSelect.addEventListener('change', function (e) {
                    self.dispatcher.notify('editor_interface.button_text_change', e.target.value);
                });
                innerEl.appendChild(textSelect);

                commandLabel = document.createElement('label');
                commandLabel.appendChild(document.createTextNode(_('editor.assignation.command.label')));
                innerEl.appendChild(commandLabel);

                commandSelect = this.createSelectElement(
                    this._map(this.editor.getAssignableCommands(), function (command) {
                        return {
                            text: command.name,
                            value: command.id
                        };
                    }),
                    'button_command',
                    '',
                    context.command
                );
                commandSelect.addEventListener('change', function (e) {
                    self.dispatcher.notify('editor_interface.button_command_change', e.target.value);
                });
                innerEl.appendChild(commandSelect);

                useLabel = document.createElement('label');
                useLabel.appendChild(document.createTextNode(_('editor.assignation.use_limit.label')));
                innerEl.appendChild(useLabel);

                useInput = document.createElement('input');
                useInput.setAttribute('type', 'text');
                useInput.setAttribute('name', 'button_count');
                useInput.value = (context.useLimit === -1 ? '' : context.useLimit);
                useInput.addEventListener('change', function (e) {
                    self.dispatcher.notify('editor_interface.button_uselimit_change', e.target.value);
                });
                innerEl.appendChild(useInput);

                useHelp = document.createElement('span');
                useHelp.className = 'help';
                useHelp.appendChild(document.createTextNode(_('editor.assignation.use_limit.help')));
                innerEl.appendChild(useHelp);
            } else {
                innerEl = document.createElement('em');
                innerEl.appendChild(document.createTextNode(_('editor.assignation.enabled')));
            }

            this._clearElement(this.assignationDiv);
            this.assignationDiv.appendChild(innerEl);
        }
    }));
})();