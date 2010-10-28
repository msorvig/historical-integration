
TableController = function (params) {
    var dimentionalData = params.dimentionalData;
    var tableDiv = params.tableDiv;

    function createTable() {
        tableDiv.html = "foo oo";
        console.log(tableData);
        console.log(tableDiv);

           var tableData = {
                "horizontalHeader" : ["Foo", "Bar"],
                "verticalHeader" : ["Ralla", "Jalla" ],
                "data" : [[1, "sdfsafsafsdfdsfsdf"], [3,4]]
                }

        var convertedData = convertDimentionalData(dimentionalData);
        $(tableDiv).populateTableWithObject(convertedData);
    }

    jQuery.fn.createTableCellsWithArray = function(array, cellType) {
        return this.each(function(){
            var row = this;
            $(array).each(function(index, value){
                console.log(this + index + value);
                var cellTypeTag = "<" + cellType + ">";
                var header = $(cellTypeTag);

                // The data cells are currently one-item arrays,
                // the headers are plain values. This should be
                // unified somehow.
                if (value instanceof Array) {
                    $(header).append(value[0]);
                } else {
                    $(header).append(value);
                }

                var cellTypeCloseTag = "</" + cellType + ">";
                $(header).append(cellTypeCloseTag);
                $(row).append(header);
            });
        });
    };

    jQuery.fn.createTableRowsWithArrays = function(arrays, verticalHeaders) {
        return this.each(function(){
            var tbody = this;
            $(arrays).each(function(index, value){
                console.log(this + index + value);
                var row = $("<tr>");
                    $(row).createTableCellsWithArray([verticalHeaders[index]], "th")
                    $(row).createTableCellsWithArray(value, "td")
                $(row).append("</tr>");
                $(tbody).append(row);
            });
        });
    };

/*
    Creates a DOM table structure from a data object.
    The data object should be formated like this:
    data.Header : array of header cells
    data.Data :array of arrays of data cells.
*/
    jQuery.fn.populateTableWithObject = function(data) {
    return this.each(function(){
        console.log("Hello.");
        console.log(data.horizontalHeader);
        console.log(data.data);

        var table = $("<table>");
            var thead = $("<thead>")
                var row = $("<tr>")
                if (data.verticalHeader.length >  0)
                    data.horizontalHeader.unshift(" ");
                $(row).createTableCellsWithArray(data.horizontalHeader, "th");
                $(row).append("</tr>");
            $(thead).append(row);
            $(thead).append("</thead>");
        $(table).append(thead);
            var tbody = $("<tbody>")
               $(tbody).createTableRowsWithArrays(data.data, data.verticalHeader);
            $(tbody).append("</tbody>");
        $(table).append(tbody);
        $(table).append("</table>");
        $(this).append(table);
    });
    };


    /*
        Creates a data object usable by createTableRowsWithArrays
        from a dimentinal data structure.
    */
    function convertDimentionalData(dimentionalData)
    {
           var tableData = {
                "horizontalHeader" : dimentionalData.indexValues[0],
                "verticalHeader" :   dimentionalData.indexValues[1],
                "data" : dimentionalData.dataTable
                }

           return tableData;
    }

    return {
        "createTable" : createTable
    }
};
