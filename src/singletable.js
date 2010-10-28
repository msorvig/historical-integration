$(document).ready(function() {
    var tableController = new TableController(
            {"dimentionalData": jsonData,
             "tableDiv": $("#table")});
    tableController.createTable();
});
