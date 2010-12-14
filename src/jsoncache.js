JsonCache = function (params) {
    var baseUrl = params.baseUrl;
    var cache;

    function insert(url, json) {
        cache[url] = json;
    }

    function contains(url) {
        return (url in cache);
    }

    function load(url) {
        if ((url in cache) == false)
            cache[url] = ajaxLoad(baseUrl + url);
        return cache[url];
    }

    function axajLoad(url) {
        return {};
    }

    return {
        "insert" : insert,
        "contains" : contains,
        "load" : load
    };
};

var JsonCahce jsonCache; // The global cache object
