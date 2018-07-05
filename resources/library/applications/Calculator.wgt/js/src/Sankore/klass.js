/*global window:true*/
(function () {
    "use strict";

    // polyfill
    if (!Function.prototype.bind) {
        Function.prototype.bind = function (oThis) {
            if (typeof this !== "function") {
                throw new TypeError("Function.prototype.bind - what is trying to be bound is not callable");
            }

            var aArgs = Array.prototype.slice.call(arguments, 1),
                fToBind = this,
                fNOP = function () {},
                fBound = function () {
                    return fToBind.apply(this instanceof fNOP && oThis ? this : oThis,
                        aArgs.concat(Array.prototype.slice.call(arguments)));
                };

            fNOP.prototype = this.prototype;
            fBound.prototype = new fNOP();

            return fBound;
        };
    }

    window.klass = {
        create: function () {
            var self = Object.create(this);

            if (typeof self.constructor === "function") {
                self.constructor.apply(self, arguments);
            }

            return self;
        },

        extend: function (object) {
            var self = Object.create(this);

            if (!object) {
                return self;
            }

            Object.keys(object).forEach(function (key) {
                self[key] = object[key];
            });

            return self;
        },

        define: function (namespace, name, object) {
            var createNamespace = function (namespace, root) {
                var namespaceParts = namespace.split('.'),
                    first;

                if (namespaceParts.length > 0) {
                    first = namespaceParts.shift();

                    if (typeof root[first] === 'undefined') {
                        root[first] = {};
                    }

                    if (namespaceParts.length > 0) {
                        return createNamespace(namespaceParts.join('.'), root[first]);
                    }

                    return root[first];
                }

                return null;
            },
                ns = createNamespace(namespace, window);

            ns[name] = object;
        }
    };
})();