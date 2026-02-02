var ch = new QWebChannel(qt.webChannelTransport, function (channel) {
    // expose channel
    window.channel = channel.objects;

    // expose widget
    if (channel.objects.widget) {
        window.widget = channel.objects.widget;
    }

    // expose sankore
    if (channel.objects.sankore) {
        window.sankore = channel.objects.sankore;
        window.sankore.async = new Object;
        window.sankore.async.preference = function (key, def){
            return new Promise((resolve,reject) => {
                window.sankore.preference(key,def,function(result){
                    resolve(result);
                });
            });
        };
        window.sankore.async.preferenceKeys = function (){
            return new Promise((resolve,reject) => {
                window.sankore.preferenceKeys(function(result){
                    resolve(result);
                });
            });
        };
    }
});
