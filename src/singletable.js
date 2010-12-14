$(document).ready(function() {
    var tableController = new TableController(
            {"tableData": jsonData,
             "tableDiv": $("#table"),
             "tableType": "grid"
            });
    tableController.createTable();
});
