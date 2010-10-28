$(document).ready(function() {
    function resultsCallback(indices, keys) {
        var concat = "";
        $.each(indices, function(key, value) {
            concat += keys[value] + " ";
        });
       // console.log(concat)
        $("#liveSearchResults").text(concat);
    }

    var searchController = new SearchController(
            {"dataUrl": "./searchindex",
             "searchInput": $("#liveSearchInput"),
             "resultsCallback": resultsCallback
             });
});
