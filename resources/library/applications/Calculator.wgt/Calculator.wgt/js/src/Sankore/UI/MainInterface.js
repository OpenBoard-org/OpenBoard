/*jshint browser:true */
/*global klass:true, Sankore:true, _:true*/
(function () {
    "use strict";

    klass.define('Sankore.UI', 'MainInterface', klass.extend({
        constructor: function (id, ed, texts, withEditor) {
            this.id = id;
            this.dispatcher = ed;
            this.texts = texts;
            this.withEditor = withEditor;

            this.title = null;

            this.caret = document.createElement('i');
            this.caret.className = 'caret';

            this.frontScreen = null;
            this.expressionRow = null;
            this.resultRow = null;
            this.flagRow = null;

            this.flags = [];
    
            this.rearScreen = null;

            this.buttons = Sankore.Util.Hash.create();

            this.rendered = false;

            this.attachEventListeners();
        },

        attachEventListeners: function () {
            var self = this;

            // the main screen text has changed
            this.dispatcher.addEventListener('keystroke_line.changed', function (keystrokeLine) {
                self.updateExpressionRow(keystrokeLine.getAsText());
            });

            // the internal memory has changed
            this.dispatcher.addEventListener('calculator.memory_changed', function (memory) {
                if (null !== memory) {
                    self.addFlag('M');
                } else {
                    self.removeFlag('M');
                }

                self.updateFlagRow();
            });

            // the op memory has changed
            this.dispatcher.addEventListener('calculator.op_changed', function (op) {
                if (null !== op) {
                    self.addFlag('OP');
                } else {
                    self.removeFlag('OP');
                }

                self.updateFlagRow();
            });

            // a new result has been computed
            this.dispatcher.addEventListener('calculator.output_changed', function (event) {
                if (null !== event.output) {
                    self.updateResultRow(event.output);
                } else if (null !== event.error) {
                    self.updateResultRow(event.error);
                } else {
                    self.updateResultRow('');
                }
            });

            // the history has changed
            this.dispatcher.addEventListener('calculator.history_changed', this.updateRearScreen.bind(this));

            // a new layout is loaded
            this.dispatcher.addEventListener('calculator.layout_loaded', function (layout) {
                self.renderButtons(layout.buttonMap);
                
                self.changeTitle(layout.name);

                self.clearRearScreen();
            });

            // a button has been disabled
            this.dispatcher.addEventListener('calculator.button_disabled', function (event) {
                var buttonEl = self.buttons.get(event.slot);

                if (buttonEl) {
                    buttonEl.disabled = true;
                }
            });

            // a button has been enabled
            this.dispatcher.addEventListener('calculator.button_enabled', function (slot) {
                var buttonEl = self.buttons.get(slot);

                if (buttonEl) {
                    buttonEl.disabled = false;
                }
            });

            // the editor is shown/hidden
            this.dispatcher.addEventListener('editor.show', this.hideRearScreen.bind(this));
            this.dispatcher.addEventListener('editor.hide', this.showRearScreen.bind(this));

            // a button has been selected
            this.dispatcher.addEventListener('editor.button_selected', function (event) {
                var newButtonEl = self.buttons.get(event.slot),
                    oldButtonEl = self.buttons.get(event.previousSlot);

                if (oldButtonEl) {
                    oldButtonEl.parentElement.classList.remove('edit');
                }

                if (newButtonEl) {
                    newButtonEl.parentElement.classList.add('edit');
                }
            });

            // a button has been renamed
            this.dispatcher.addEventListener('editor.button_renamed', function (event) {
                var buttonEl = self.buttons.get(event.slot),
                    text = self.texts.get(event.button.text),
                    hasEditClass;

                if (buttonEl) {
                    buttonEl.innerText = text.button;

                    hasEditClass = buttonEl.parentElement.classList.contains('edit');
                    buttonEl.parentElement.className = text.type;

                    if (hasEditClass) {
                        buttonEl.parentElement.classList.add('edit');
                    }
                }
            });
        },

        _clearElement: function (element) {
            while (element.firstChild) {
                element.removeChild(element.firstChild);
            }
        },

        getRootElement: function () {
            return document.getElementById(this.id);
        },

        render: function () {
            var root = this.getRootElement();

            this._clearElement(root);

            if (!root.classList.contains('calculator')) {
                root.classList.add('calculator');
            }

            this.createBaseMarkup(root);

            this.rearScreen = document.createElement('ul');
            root
                .getElementsByClassName('rear-screen')
                .item(0)
                .appendChild(this.rearScreen);

            this.frontScreen = document.createElement('ul');
            root
                .getElementsByClassName('front-screen')
                .item(0)
                .appendChild(this.frontScreen);

            this.expressionRow = document.createElement('li');
            this.expressionRow.classList.add('expression-row');
            this.frontScreen.appendChild(this.expressionRow);

            this.flagRow = document.createElement('li');
            this.flagRow.classList.add('flag-row');
            this.frontScreen.appendChild(this.flagRow);

            this.resultRow = document.createElement('li');
            this.resultRow.classList.add('result-row');
            this.frontScreen.appendChild(this.resultRow);

            this.rendered = true;
        },

        renderButtons: function (buttonMap) {
            var buttonsTrs,
                map = {},
                buttons = document.createDocumentFragment(),
                rootElement = this
                    .getRootElement()
                    .getElementsByClassName('screen')
                    .item(0)
                    .parentElement;

            // map copy in order to prevent alteration of the original map
            for (var slot in buttonMap) {
                map[slot] = buttonMap[slot];
            }

            // adding constant non-editable buttons 
            map.a0 = Sankore.Button.create('s', 'del', -1, false);
            map.b0 = Sankore.Button.create('l', 'left', -1, false);
            map.c0 = Sankore.Button.create('r', 'right', -1, false);
            map.d0 = Sankore.Button.create('c', 'clear', -1, false);

            buttonsTrs = rootElement.getElementsByClassName('buttons');
            while (buttonsTrs.length > 0) {
                rootElement.removeChild(buttonsTrs[0]);
            }

            buttonsTrs = this.createButtons(map);

            // we use a DocumentFragment to avoid excessive and useless repaints
            for (var i in buttonsTrs) {
                buttons.appendChild(buttonsTrs[i]);
            }

            rootElement.appendChild(buttons);
        },

        createBaseMarkup: function (element) {
            var table = document.createElement('table'),
                tr = document.createElement('tr'),
                rearTd = document.createElement('td'),
                frontTd = document.createElement('td');

            rearTd.className = 'rear-screen';
            rearTd.setAttribute('rowspan', '8');
            tr.appendChild(rearTd);

            frontTd.className = 'front-screen';
            frontTd.setAttribute('colspan', '4');
            tr.appendChild(frontTd);

            tr.className = 'screen';
            table.appendChild(tr);
            
            element.appendChild(this.createTitle());
            element.appendChild(this.createControls());
            element.appendChild(table);
        },
        
        createTitle: function () {
            this.title = document.createElement('span');
            
            this.title.appendChild(document.createTextNode('Chargement...'));
            this.title.className = 'title';
            
            return this.title;
        },

        createControls: function () {
            var controlsDiv = document.createElement('div'),
                editorButton = document.createElement('button'),
                resetButton = document.createElement('button'),
                self = this;

            if (this.withEditor) {
                editorButton.setAttribute('type', 'button');
                editorButton.appendChild(document.createTextNode(_('controls.editor')));
                editorButton.addEventListener('click', function (e) {
                    self.dispatcher.notify('main_interface.editor_click');
                });
                controlsDiv.appendChild(editorButton);
            }

            resetButton.setAttribute('type', 'button');
            resetButton.appendChild(document.createTextNode(_('controls.reset')));
            resetButton.addEventListener('click', function (e) {
                self.dispatcher.notify('main_interface.reset_click');
            });
            controlsDiv.appendChild(resetButton);

            controlsDiv.className = 'controls';

            return controlsDiv;
        },

        createButtons: function (buttonMap) {
            var trs = [],
                i;

            for (i = 0; i < 7; i++) {
                trs.push(this.createButtonRow(i, buttonMap));
            }

            return trs;
        },

        createButtonRow: function (rowNumber, buttonMap) {
            var tr = document.createElement('tr'),
                i;

            tr.className = 'buttons';

            for (i = 'a'; i <= 'd'; i = String.fromCharCode(i.charCodeAt(0) + 1)) {
                tr.appendChild(this.createButton(i + rowNumber, buttonMap));
            }

            return tr;
        },

        createButton: function (slot, buttonMap) {
            var td = document.createElement('td'),
                self = this,
                text,
                buttonEl,
                button = buttonMap[slot];

            td.setAttribute('data-slot', slot);
            td.setAttribute('data-editable', button.isEditable());

            if (typeof button !== 'undefined') {
                buttonEl = document.createElement('button');
                text = this.texts.get(button.text);
                buttonEl.innerText = text.button;

                if (text.type !== 'normal') {
                    td.classList.add(text.type);
                }

                buttonEl.addEventListener('mousedown', function (e) {
                    self.handleButtonClick(this, e, button, slot);
                });

                this.buttons.set(slot, buttonEl);

                this.dispatcher.notify('main_interface.button_created', {
                    button: button,
                    slot: slot
                });

                td.appendChild(buttonEl);
            }

            return td;
        },

        handleButtonClick: function (el, event, button, slot) {
            this.dispatcher.notify('main_interface.button_click', {
                button: button,
                slot: slot
            });
        },

        updateFlagRow: function () {
            var span;

            this._clearElement(this.flagRow);

            for (var i in this.flags) {
                span = document.createElement('span');
                span.appendChild(document.createTextNode(this.flags[i]));

                this.flagRow.appendChild(span);
            }
        },

        updateExpressionRow: function (keystrokeText) {
            this._clearElement(this.expressionRow);
            this.expressionRow.appendChild(document.createTextNode(keystrokeText[0]));
            this.expressionRow.appendChild(this.caret);
            this.expressionRow.appendChild(document.createTextNode(keystrokeText[1]));
        },

        updateResultRow: function (result) {
            this._clearElement(this.resultRow);
            this.resultRow.appendChild(this.createResultRow(result));
        },

        createResultRow: function (result) {
            try {
                if (Sankore.Calculus.EuclideanDivisionOperation.isPrototypeOf(result)) {
                    return this.createResultRowEuclidean(result.getValue(), result.getRemainder());
                } else if (Sankore.Calculus.Expression.isPrototypeOf(result)) {
                    return this.createResultRowExpression(result.getValue());
                } else if (Sankore.Util.Error.isPrototypeOf(result)) {
                    return this.createResultRowError(result);
                } else {
                    return this.createResultRowText(String(result));
                }
            } catch (e) {
                return this.createResultRowError(e);
            }

            return null;
        },

        createResultRowEuclidean: function (quotient, remainder) {
            var p = document.createElement('p'),
                quotientSpan = document.createElement('span'),
                remainderSpan = document.createElement('span');

            p.classList.add('euclidean');

            remainderSpan.classList.add('remainder');
            remainderSpan.textContent = this.formatResultValue(remainder);
            p.appendChild(remainderSpan);

            quotientSpan.classList.add('quotient');
            quotientSpan.textContent = this.formatResultValue(quotient);
            p.appendChild(quotientSpan);

            return p;
        },

        createResultRowExpression: function (value) {
            return document.createTextNode(this.formatResultValue(value));
        },

        createResultRowText: function (text) {
            return document.createTextNode(text);
        },

        createResultRowError: function (error) {
            var errorDiv = document.createElement('div'),
                text = _('error.common');

            if (error.name === 'ZeroDivision') {
                text = _('error.zero_division');
            }

            errorDiv.classList.add('error');
            errorDiv.appendChild(document.createTextNode(text));

            return errorDiv;
        },

        formatResultValue: function (raw) {
            var fixed = raw.toFixed(Math.max(0, 10 - raw.toFixed().length)),
                last;

            if (fixed.indexOf('.') !== -1) {
                while (true) {
                    last = fixed.charAt(fixed.length - 1);

                    if (last === "0") {
                        fixed = fixed.slice(0, -1);
                        continue;
                    }

                    if (last === '.') {
                        fixed = fixed.slice(0, -1);
                    }

                    break;
                }
            }

            // si le nombre de chiffres dépassent 10, on tronque à 10 à droite
            // et on arrondit au dernier (wtf)
            if (Number(fixed) > (1e10 - 1)) {
                fixed = Number(fixed.substring(0, 11) / 10).toFixed();
            }
            
            if (fixed.indexOf('.') !== -1) {
                fixed = fixed.replace('.', _('text.comma'));
            }

            return fixed;
        },

        updateRearScreen: function (history) {
            var expressionRow,
                resultRow,
                rows = document.createDocumentFragment();

            for (var i in history) {
                expressionRow = document.createElement('li');
                expressionRow.className = 'expression-row';
                expressionRow.appendChild(document.createTextNode(history[i].expression));
                rows.appendChild(expressionRow);

                resultRow = document.createElement('li');
                resultRow.className = 'result-row';
                resultRow.appendChild(this.createResultRow(history[i].output));
                rows.appendChild(resultRow);
            }

            this.clearRearScreen();
            this.rearScreen.appendChild(rows);
            this.rearScreen.lastChild.scrollIntoView();
        },

        clearRearScreen: function () {
            this._clearElement(this.rearScreen);
        },

        showRearScreen: function () {
            this.rearScreen.style.display = 'block';
            this.showTitle();
        },

        hideRearScreen: function () {
            this.rearScreen.style.display = 'none';
            this.hideTitle();
        },

        addFlag: function (flag) {
            if (this.flags.indexOf(flag) === -1) {
                this.flags.push(flag);

                this.flags.sort();
            }
        },

        removeFlag: function (flag) {
            var idx = this.flags.indexOf(flag);
            if (idx !== -1) {
                this.flags.splice(idx, 1);

                this.flags.sort();
            }
        },
        
        changeTitle: function (title) {
            this._clearElement(this.title);
            this.title.appendChild(document.createTextNode(title));
        },
        
        showTitle: function () {
            this.title.style.visibility = 'visible';
        },
        
        hideTitle: function () {
            this.title.style.visibility = 'hidden';
        }
    }));
})();