
$(document).ready(function() {

    var rootElement = $("#root");
    var selectorElement = $("<div>");
    var visualization = $("<div id='vsualization'>");
    rootElement.append(selectorElement);
    rootElement.append(visualization);

    var displayOptionSelector = new OptionSelector(
        {
            "title" : "Show",
            "options": ["Chart", "Table", "List"],
            "callback": createVisualization
        });
    displayOptionSelector.create(selectorElement);

    function createVisualization(type) {
        visualization.empty();

        if (type == "Chart") {
            var chartController = new ChartController(
                { "data" : jsonData,
                  "meta" : {} }
            );

            visualization.css("width", "90%").css("height", "300px")
                         .css("margin-top", "20px").css("margin-left", "20px");

            chartController.create(visualization);

        } else {
            var tableController = new TableController(
                    {"tableData": jsonData,
                     "tableDiv": visualization,
                     "tableType": type.toLowerCase()
                    });

            tableController.create(visualization);
        }
    }




});
