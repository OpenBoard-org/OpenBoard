/*global klass:true, Sankore:true*/
(function () {
    "use strict";

    klass.define('Sankore', 'KeystrokeLine', klass.extend({
        constructor: function (dispatcher) {
            this.dispatcher = dispatcher;

            this.keystrokes = [];
            this.caret = 0;
        },

        notify: function () {
            this.dispatcher.notify('keystroke_line.changed', this);
        },

        hit: function (keystroke) {
            this.keystrokes.splice(this.caret, 0, keystroke);
            this.caret++;
            this.notify();
        },

        del: function () {
            if (this.caret > 0) {
                var deleted = this.keystrokes.splice(this.caret - 1, 1)[0];
                this.caret--;
                this.notify();
                
                return deleted;
            }
        },

        moveCaretLeft: function () {
            if (this.caret > 0) {
                this.caret--;
                this.notify();
            }
        },

        moveCaretRight: function () {
            if (this.caret < this.keystrokes.length) {
                this.caret++;
                this.notify();
            }
        },

        moveCaretToEnd: function () {
            this.caret = this.keystrokes.length;
            this.notify();
        },

        reset: function () {
            this.caret = 0;
            this.keystrokes = [];
            this.notify();
        },

        count: function () {
            return this.keystrokes.length;
        },

        at: function (index) {
            if (typeof this.keystrokes[index] !== 'undefined') {
                return this.keystrokes[index];
            }

            throw Sankore.Util.Error.create('OutOfRangeError', 'No keystroke at index ' + index);
        },

        getAsText: function () {
            return [
                this.getTextAtRange(0, this.caret),
                this.getTextAtRange(this.caret, this.keystrokes.length)
            ];
        },

        getTextAtRange: function (from, to) {
            var i, output = '';

            if (from < 0) {
                throw Sankore.Util.Error.create('OutOfRangeError', 'Cannot get keystroke before index 0');
            }

            if (from > this.keystrokes.length) {
                throw Sankore.Util.Error.create('OutOfRangeError', 'Cannot get keystroke after index ' + this.keystrokes.length);
            }

            for (i = from; i < to; i++) {
                output += this.at(i).text;
            }

            return output;
        },
        
        getState: function () {
            return {
                keystrokes: this.keystrokes,
                caret: this.caret
            };
        },
        
        loadState: function (state) {
            this.keystrokes = state.keystrokes || {};
            this.caret = state.caret || 0;
            this.notify();
        }

    }));
})();