(function($) {
    $.fn.oembed = function(url, options) {		
	
		options = $.extend({}, $.fn.oembed.defaults, options);
		
        return this.each(function() {      
			
			var container = $(this), 
				target = (url != null) ? url : container.attr("href"), 
				provider;		
			
			if (target != null) {			
				
				provider = getOEmbedProvider(target);

				if (provider != null) {
					provider.maxWidth = options.maxWidth;
					provider.maxHeight = options.maxHeight;               
					
					provider.embedCode(target, function(code) { container.html(code); });		
				}
			}
        });
    };

    // Plugin defaults
    $.fn.oembed.defaults = {
        maxWidth: 500,
        maxHeight: 400
    };

    $.fn.oembed.getPhotoCode = function(url, data) {
        var code = '<div><a href="' + url + '" target="_blank"><img src="' + data.url + '"/></a></div>';
        if (data.html)
            code += "<div>" + data.html + "</div>";
        return code;
    };

    $.fn.oembed.getVideoCode = function(url, data) {
        var code = data.html;
        return code;
    };

    $.fn.oembed.getRichCode = function(url, data) {
        var code = data.html;
        return code;
    };

    $.fn.oembed.getGenericCode = function(url, data) {
        var title = (data.title != null) ? data.title : url,
			code = '<a href="' + url + '">' + title + '</a>';			
        if (data.html)
            code += "<div>" + data.html + "</div>";
		return code;
    };

    $.fn.oembed.isAvailable = function(url) {
        var provider = getOEmbedProvider(url);
        return (provider != null);
    };
	
    /* Private Methods */
    function getOEmbedProvider(url) {
        for (var i = 0; i < providers.length; i++) {
            if (providers[i].matches(url))
                return providers[i];
        }
        return null;
    }

    var providers = [
        new OEmbedProvider("fivemin", "5min.com"),
        new OEmbedProvider("amazon", "amazon.com"),
        new OEmbedProvider("flickr", "flickr", "http://flickr.com/services/oembed", "jsoncallback"),
        new OEmbedProvider("googlevideo", "video.google."),
        new OEmbedProvider("hulu", "hulu.com"),
        new OEmbedProvider("imdb", "imdb.com"),
        new OEmbedProvider("metacafe", "metacafe.com"),
        new OEmbedProvider("qik", "qik.com"),
        new OEmbedProvider("revision3", "slideshare"),
        new OEmbedProvider("slideshare", "5min.com"),
        new OEmbedProvider("twitpic", "twitpic.com"),
        new OEmbedProvider("viddler", "viddler.com"),
        new OEmbedProvider("vimeo", "vimeo.com", "http://vimeo.com/api/oembed.json"),
        new OEmbedProvider("wikipedia", "wikipedia.org"),
        new OEmbedProvider("wordpress", "wordpress.com"),
        new OEmbedProvider("youtube", "youtube.com")
    ];

    function OEmbedProvider(name, urlPattern, oEmbedUrl, callbackparameter) {
        this.name = name;
        this.urlPattern = urlPattern;
        this.oEmbedUrl = (oEmbedUrl != null) ? oEmbedUrl : "http://oohembed.com/oohembed/";
        this.callbackparameter = (callbackparameter != null) ? callbackparameter : "callback";
        this.maxWidth = 500;
        this.maxHeight = 400;

        this.matches = function(externalUrl) {
            // TODO: Convert to Regex
            return externalUrl.indexOf(this.urlPattern) >= 0;
        };

        this.getRequestUrl = function(externalUrl) {

            var url = this.oEmbedUrl;

            if (url.indexOf("?") <= 0)
                url = url + "?";

            url += "maxwidth=" + this.maxWidth + 
						"&maxHeight=" + this.maxHeight + 
						"&format=json" + 
						"&url=" + escape(externalUrl) + 
						"&" + this.callbackparameter + "=?";						
            return url;
        }

        this.embedCode = function(externalUrl, embedCallback) {

            var request = this.getRequestUrl(externalUrl);

            $.getJSON(request, function(data) {
                
				var code, type = data.type;

                switch (type) {
                    case "photo":
                        code = $.fn.oembed.getPhotoCode(externalUrl, data);
                        break;
                    case "video":
                        code = $.fn.oembed.getVideoCode(externalUrl, data);
                        break;
                    case "rich":
                        code = $.fn.oembed.getRichCode(externalUrl, data);
                        break;
                    default:
                        code = $.fn.oembed.getGenericCode(externalUrl, data);
                        break;
                }

                embedCallback(code);
            });
        }
    }
})(jQuery);

