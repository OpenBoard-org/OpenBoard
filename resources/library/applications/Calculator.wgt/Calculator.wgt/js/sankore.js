/*jshint browser:true, devel:true*/
if (!('sankore' in window)) {
    window.sankore = {
        preferences: {
            state: ''
        },

        setPreference: function (name, value) {
            console.log('Preference "' + name + '" set to : ' + value);
            this.preferences[name] = value;
        },

        preference: function (name) {
            console.log('Accessing "' + name + '"');
            return this.preferences[name] || '';
        },

        locale: function () {
            return window.navigator.language;
        }
    };
}