
SearchController = function (params) {
    var dataUrl = params.dataUrl;   // location of the key index files on the server.
    var searchInput = params.searchInput; // the <input> dom element.
    var resultsCallback = params.resultsCallback; // user callback for search results.

    function searchTermKeyup() {
        remoteLookup(searchInput.val());
    }

    function searchTermChanged() {
        remoteLookup(searchInput.val());
    }
    searchInput.change(searchTermChanged);
    searchInput.keyup(searchTermKeyup);
   // searchInput.onsearch = "searchTermChanged()";

    // Searches for strings starting with the needle string
    // in the haystack array of strings. returns an array
    // of "hit" indices into the haystack.
    function getMatchIndicies(needle, haystack)
    {
        var matchIndices = [];
        $.each(haystack,  function(index, value) {
            if (needle == value) {
                matchIndices.unshift(index); // full-string match, push index to beginning.
            } else if (value.substr(0, needle.length).toLowerCase() == needle.toLowerCase()) {
                matchIndices.push(index); // prefic match, add to end.
            }
        });
        return matchIndices;
    }


    // A cache of search index files. Stores fileName -> file data object.
    var fileHash = {};
    // Looks up a string in the local cached search index data.
    // Calls resultsCallback if a match is found.
    function localLookup(string)
    {
        var lookupFileName = string.substr(0, 2);
        var fileData = fileHash[lookupFileName];
        if (fileData == null) {
            resultsCallback([], []);
            return;
        }
        var keyArray = fileData.indexValues[0];
       // console.log("local lookup of " + string + "in " + keyArray);
        var indices = getMatchIndicies(string, keyArray)
        resultsCallback(indices, keyArray);
   }

    var currentLookup;
    // Looks up a string in the search key index. Async requests
    // a key index file from the server if nessecary.
    // Calls resultsCallback if a match is found.
    function remoteLookup(string) {
        if (string == currentLookup)
            return;
        currentLookup = string;

        if (string.length < 2) {
            resultsCallback([], []);
            return;
        }

        var lookupFileName = string.substr(0, 2);
        var lookupFilePath = dataUrl + "/" + lookupFileName;

        console.log("lookup " + string);

        if (fileHash[lookupFileName] != null) {
            localLookup(string);
            return;
        }

       // $.getScript("foo.txt", function(data, textStatus) { alert (data, textStatus); } );

        $.getJSON(lookupFilePath,
            function(data){
                 if (data != null) {
                    fileHash[lookupFileName] = data;
                    localLookup(string);
                 }
            }
        );
    }

    return {
        // no public functions!
    }
};
