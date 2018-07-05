/*jshint devel:true, browser:true*/
/*globals klass:true, Sankore:true, _:true */
(function () {
    "use strict";

    klass.define('Sankore', 'Calculator', klass.extend({
        constructor: function (id, options) {
            Sankore.Util.i18n.load(options.locale || 'en');

            // state attributes
            this.currentLayout = null;
            this.lastError = null; // last raised error
            this.memory = null; // internal memory
            this.op = null; // op memory
            this.output = null; // last evaluated resut
            this.history = []; // operation history
            this.buttonUseCount = {}; // button use count

            // local attributes
            this.expressionString = ''; // current expressionString
            this.unpredictable = options.unpredictableMode || false;

            // components
            this.eventDispatcher = Sankore.Util.EventDispatcher.create();
            this.commands = Sankore.Util.Hash.create(); // command hash
            this.texts = Sankore.Util.Hash.create(); // text hash
            this.calculusEngine = Sankore.Calculus.Engine.create();
            this.keystrokeLine = Sankore.KeystrokeLine.create(this.eventDispatcher);
            this.ui = Sankore.UI.MainInterface.create(id, this.eventDispatcher, this.texts, this.unpredictable);
            this.editor = Sankore.Editor.Editor.create(this);

            if (typeof options.ready !== 'undefined') {
                this.eventDispatcher.addEventListener('calculator.create', options.ready.bind(this));
            }
            
            var c = Sankore.Text.create.bind(Sankore.Text);
            this.texts.add('id', [
               c('0', '0', '0'),
               c('1', '1', '1'),
               c('2', '2', '2'),
               c('3', '3', '3'),
               c('4', '4', '4'),
               c('5', '5', '5'),
               c('6', '6', '6'),
               c('7', '7', '7'),
               c('8', '8', '8'),
               c('9', '9', '9'),
               c('+', '+', '+', 'alt'),
               c('-', '-', '-', 'alt'),
               c('*', '\u00D7', '\u00D7', 'alt'),
               c('/', '\u00F7', '\u00F7', 'alt'),
               c(':', '\u22A2', '\u22A2', 'alt'),
               c('=', '=', '=', 'alt'),
               c('.', _('text.comma'), _('text.comma')),
               c('(', '(', '('),
               c(')', ')', ')'),
               c('op', 'OP', 'OP'),
               c('mr', 'MR', 'MR'),
               c('mc', 'MC', 'MC'),
               c('m+', 'M+', 'M+'),
               c('m-', 'M-', 'M-'),
               c('s', '', _('text.del'), 'alt', false),
               c('l', '', '\u2190', 'alt', false),
               c('r', '', '\u2192', 'alt', false),
               c('c', '', 'C', 'danger', false)
            ]);

            c = Sankore.Command.create.bind(Sankore.Command);
            this.commands.add('id', [
                // add 0 to the expression string
                c('0', _('command.zero'), function (args) {
                    this.expressionString += '0';
                }),

                // add 1 to the expression string
                c('1', _('command.one'), function (args) {
                    this.expressionString += '1';
                }),

                // add 2 to the expression string
                c('2', _('command.two'), function (args) {
                    this.expressionString += '2';
                }),

                // add 3 to the expression string
                c('3', _('command.three'), function (args) {
                    this.expressionString += '3';
                }),

                // add 4 to the expression string
                c('4', _('command.four'), function (args) {
                    this.expressionString += '4';
                }),

                // add 5 to the expression string
                c('5', _('command.five'), function (args) {
                    this.expressionString += '5';
                }),

                // add 6 to the expression string
                c('6', _('command.six'), function (args) {
                    this.expressionString += '6';
                }),

                // add 7 to the expression string
                c('7', _('command.seven'), function (args) {
                    this.expressionString += '7';
                }),

                // add 8 to the expression string
                c('8', _('command.eight'), function (args) {
                    this.expressionString += '8';
                }),

                // add 9 to the expression string
                c('9', _('command.nine'), function (args) {
                    this.expressionString += '9';
                }),

                // add + to the expression string
                c('+', _('command.plus'), function (args) {
                    this.expressionString += '+';
                }),

                // add - to the expression string
                c('-', _('command.minus'), function (args) {
                    this.expressionString += '-';
                }),

                // add * to the expression string
                c('*', _('command.times'), function (args) {
                    this.expressionString += '*';
                }),

                // add / to the expression string
                c('/', _('command.divide'), function (args) {
                    this.expressionString += '/';
                }),

                // add : to the expression string
                c(':', _('command.euclidean_divide'), function (args) {
                    this.expressionString += ':';
                }),

                // evaluate the current expression
                Sankore.InterruptingCommand.create('=', _('command.equal'), function (args) {
                    this.evaluateStack();
                }),

                // add . to the expression string
                c('.', _('command.comma'), function (args) {
                    this.expressionString += '.';
                }),

                // add ( to the expression string
                c('(', _('command.open_parenthesis'), function (args) {
                    this.expressionString += '(';
                }),

                // add ) to the expression string
                c(')', _('command.close_parenthesis'), function (args) {
                    this.expressionString += ')';
                }),

                // store the current (incomplete) expression if not already, else
                // call the incomplete expression, append it to the expression string then evaluate
                Sankore.InterruptingCommand.create('op', _('command.op'), function (args) {
                    if (null === this.op) {
                        if (this.expressionString.length > 1 &&
                            -1 !== '+-*/:'.indexOf(this.expressionString[0])
                        ) {
                            try {
                                // we parse the OP expression with a fake left operand (1)
                                // if it raised exceptions during parsing or evaluating, 
                                // the expression is not good for an OP operation
                                this.calculusEngine.evaluate('(1)' + this.expressionString).getValue();
                                
                                this.op = this.expressionString;
                                this.eventDispatcher.notify('calculator.op_changed', this.op);
                            } catch (e) {
                            }
                        }

                        this.expressionString = '';
                        this.output = null;
                    } else {
                        // case : the OP command is called immediately. The previous results is used
                        // as the left operand
                        if (0 === this.expressionString.length && null !== this.output) {
                            this.expressionString = '(' + this.output.toString() + ')';
                        }

                        this.expressionString += this.op;
                        this.execCommand('=');
                    }
                }),

                // evaluate the expression string and add it to the memory
                Sankore.InterruptingCommand.create('memoryAdd', _('command.memory_add'), function (args) {
                    this.execCommand('=');

                    try {
                        if (null === this.memory) {
                            this.memory = 0;
                        }

                        this.memory += this.output.getValue();
                    } catch (e) {
                        this.memory = null;
                    }

                    this.eventDispatcher.notify('calculator.memory_changed', this.memory);
                }),

                // evaluate the expression string and substract it from the memory
                Sankore.InterruptingCommand.create('memorySub', _('command.memory_sub'), function (args) {
                    this.execCommand('=');

                    try {
                        if (null === this.memory) {
                            this.memory = 0;
                        }

                        this.memory -= this.output.getValue();
                    } catch (e) {
                        this.memory = null;
                    }

                    this.eventDispatcher.notify('calculator.memory_changed', this.memory);
                }),

                // add the current memory value to the expression string
                c('memoryRecall', _('command.memory_recall'), function (args) {
                    if (null !== this.memory) {
                        this.expressionString += '(' + this.memory.toString() + ')';
                    }
                }),

                // clear the memory
                c('memoryClear', _('command.memory_clear'), function (args) {
                    this.memory = null;

                    this.eventDispatcher.notify('calculator.memory_changed', this.memory);
                }),

                // clear the output, memory, op and expression string
                Sankore.InternalCommand.create('clear', _('command.clear'), function (args) {
                    this.reset();

                    this.eventDispatcher.notify('calculator.memory_changed', this.memory);
                    this.eventDispatcher.notify('calculator.op_changed', this.op);
                    this.eventDispatcher.notify('calculator.output_changed', {
                        output: this.output,
                        error: this.lastError
                    });
                }),

                // move the caret left
                Sankore.InternalCommand.create('left', _('command.left'), function (args) {
                    this.keystrokeLine.moveCaretLeft();
                }),

                // move the caret right
                Sankore.InternalCommand.create('right', _('command.right'), function (args) {
                    this.keystrokeLine.moveCaretRight();
                }),

                // delete the keystroke before the caret
                Sankore.InternalCommand.create('del', _('command.del'), function (args) {
                    var keystroke = this.keystrokeLine.del();

                    if (keystroke && this.getButtonUseCount(keystroke.slot)) {
                        this.buttonUseCount[keystroke.slot]--;
                        if (this.getButtonUseCount(keystroke.slot) < this.currentLayout.getButton(keystroke.slot).useLimit) {
                            this.eventDispatcher.notify('calculator.button_enabled', keystroke.slot);
                        }
                    }
                })
            ]);

            this.attachEventHandlers();

            this.eventDispatcher.notify('calculator.create');
        },

        attachEventHandlers: function () {
            var self = this, ed = this.eventDispatcher;

            if (this.unpredictable) {
                ed.addEventListener('editor.show', this.reload.bind(this));
                ed.addEventListener('editor.hide', this.reload.bind(this));
            }

            ed.addEventListener('editor.layout_selected', function (layout) {
                self.loadLayout(layout.id);
            });

            ed.addEventListener('main_interface.button_created', function (event) {
                if (!self.editor.enabled && event.button.isDisabled()) {
                    ed.notify('calculator.button_disabled', event);
                }
            });

            ed.addEventListener('main_interface.button_click', function (event) {
                if (!self.editor.enabled) {
                    self.useButton(event.slot);

                    self.keystroke(event.slot, event.button);

                    if (!event.button.isUsable() && self.getButtonUseCount(event.slot) >= event.button.useLimit) {
                        ed.notify('calculator.button_disabled', event);
                    }
                }
            });

            ed.addEventListener('main_interface.reset_click', function () {
                self.reload();
            });
        },

        reset: function () {
            this.memory = null;
            this.op = null;
            this.output = null;
            this.lastError = null;
            this.expressionString = [];
            this.keystrokeLine.reset();
        },

        init: function (state) {
            var self = this;

            this.ui.render();

            this.editor.init(state.editor ||  {});

            if (null !== this.layout) {
                this.loadLayout(state.layout || 'default');
            }

            if ('buttonUseCount' in state) {
                this.buttonUseCount = state.buttonUseCount;
            }

            if ('keystrokes' in state) {
                this.keystrokeLine.loadState(state.keystrokes || {});
            }

            if ('output' in state && state.output) {
                this.expressionString = state.output || '';

                this.evaluateStack();
            }

            if ('error' in state && state.error) {
                this.lastError = Sankore.Util.Error.create(state.error.name, state.error.message);

                this.eventDispatcher.notify('calculator.output_changed', {
                    output: this.output,
                    error: this.lastError
                });
            }

            if ('memory' in state) {
                this.memory = state.memory;

                this.eventDispatcher.notify('calculator.memory_changed', this.memory);
            }

            if ('op' in state) {
                this.op = state.op;

                this.eventDispatcher.notify('calculator.op_changed', this.op);
            }

            if ('history' in state && state.history.length > 0) {
                this.history = state.history.map(function (log) {
                    return {
                        expression: log.expression,
                        output: self.calculusEngine.evaluate(log.output)
                    };
                });

                this.eventDispatcher.notify('calculator.history_changed', this.history);
            }

            this.eventDispatcher.notify('calculator.init');
        },

        getState: function () {
            return {
                layout: this.currentLayout ? this.currentLayout.id : null,
                error: this.lastError,
                memory: this.memory,
                op: this.op,
                output: this.output ? this.output.toString() : null,
                buttonUseCount: this.buttonUseCount,
                editor: this.editor.getState(),
                keystrokes: this.keystrokeLine.getState(),
                history: this.history.map(function (log) {
                    return {
                        expression: log.expression,
                        output: log.output.toString()
                    };
                })
            };
        },

        loadLayout: function (layoutId) {
            this.currentLayout = this.editor.layouts.get(layoutId);

            this.execCommand('clear');
            this.buttonUseCount = {};
            this.history = [];

            this.eventDispatcher.notify('calculator.layout_loaded', this.currentLayout);
        },

        reload: function () {
            this.loadLayout(this.currentLayout.id);
        },

        execCommand: function (name, args) {
            var command = this.commands.get(name);

            command.exec(this, args);

            this.eventDispatcher.notify('calculator.command_executed', {
                command: command,
                args: args
            });
        },

        evaluateStack: function () {
            try {
                if (this.expressionString.length > 0) {
                    this.output = this.calculusEngine.evaluate(this.expressionString);
                } else {
                    this.output = null;
                }

                this.lastError = null;
            } catch (e) {
                this.lastError = e;
                this.output = null;
            }

            this.expressionString = '';

            this.eventDispatcher.notify('calculator.output_changed', {
                output: this.output,
                error: this.lastError
            });
        },

        keystroke: function (slot, button) {
            var command = this.commands.get(button.command),
                text = this.texts.get(button.text);

            if (!command.isInternal()) {
                if (command.isInterrupting()) {
                    this.keystrokeLine.moveCaretToEnd();
                }

                this.keystrokeLine.hit({
                    slot: slot,
                    text: text.screen,
                    command: button.command
                });
            } else {
                this.execCommand(command.id);
            }

            if (command.isInterrupting()) {
                this.execute();
            }
        },

        execute: function () {
            var length = this.keystrokeLine.count(),
                i;

            for (i = 0; i < length; i++) {
                this.execCommand(this.keystrokeLine.at(i).command);
            }

            this.eventDispatcher.notify('calculator.executed');

            if (!this.lastError && this.output) {
                try {
                    this.backup(
                        this.keystrokeLine.getAsText().join(''),
                        this.output
                    );
                } catch (e) {}
            }

            this.keystrokeLine.reset();
        },

        useButton: function (slot) {
            if (typeof this.buttonUseCount[slot] === 'undefined') {
                this.buttonUseCount[slot] = 0;
            }

            this.buttonUseCount[slot]++;
        },

        getButtonUseCount: function (slot) {
            return this.buttonUseCount[slot] || 0;
        },

        backup: function (expression, output) {
            try {
                // precompute output value for raising potentials exceptions, ugly hack though
                output.getValue();

                this.history.push({
                    expression: expression,
                    output: output
                });

                this.eventDispatcher.notify('calculator.history_changed', this.history);
            } catch (e) {}
        }

    }));
})();