
TableController = function (params) {
    var tableData = params.tableData.data;
    var tableType = params.tableType; // "table" or "list"

    function create(tableElement) {
//           var tableTestData = {
//                "horizontalHeader" : ["Foo", "Bar"],
//                "verticalHeader" : ["Ralla", "Jalla" ],
//                "data" : [[1, "sdfsafsafsdfdsfsdf"], [3,4]]
//                }

        var interpretedData = interpretTableData(tableData);
        var actualTable = $(tableElement).populateTableWithObject(interpretedData);
    }

    jQuery.fn.createTableCellsWithArray = function(array, cellType) {
        if (array === undefined)
            return;
        return this.each(function(){
            var row = this;
            $(array).each(function(index, value){
                // console.log(this + index + value);

                var cellTypeTag = "<" + cellType + ">";
                var header = $(cellTypeTag);

                // The data cells are currently one-item arrays,
                // the headers are plain values. This should be
                // unified somehow.
                if (value instanceof Array) {
                    header.append(value[0]);
                } else {
                    // ### I'm doing something wrong, the append below
                    // fails for "constructor". Probably a keyword.
                    if (value == "constructor")
                        value = "constructor "; //add space

                    header.append(value);
                }

                var cellTypeCloseTag = "</" + cellType + ">";
                header.append(cellTypeCloseTag);
                $(row).append(header);
            });
        });
    };

    jQuery.fn.createTableRowsWithArrays = function(arrays, verticalHeaders) {
        return this.each(function(){
            var tbody = this;
            $(arrays).each(function(index, value){
                // console.log(this + index + value);
                var row = $("<tr>");
                    if (verticalHeaders !== undefined) {
                         $(row).createTableCellsWithArray([verticalHeaders[index]], "th")
                        //$(row).createTableCellsWithArray(["eh"], "th")

                    }
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
        // console.log("Hello.");
        // console.log(data.horizontalHeader);
        // console.log(data.data);

        var table = $("<table>");
            var thead = $("<thead>")
                var row = $("<tr>")
                //if (data.verticalHeader !== undefined && data.verticalHeader.length >  0)
                 //   data.horizontalHeader.unshift(" ");
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
        table.dataTable({
            "bPaginate": false
        });
    });
    };


    function interpretTableData(jsonData)
    {
        // console.log("tableType " + tableType);

        // Support two interpretations for now:
        // - (2D) table
        // - (1D) list
        if (tableType == "table") {
            return interpretTableDataAsGrid(jsonData);
        } else {
            return interpretTableDataAsList(jsonData);
        }
    }

     function interpretTableDataAsGrid(jsonData)
    {
        var horizontalHeader = jsonData.columnValues[0].slice(0);
        horizontalHeader.unshift(jsonData.columnNames[1]);
        // console.log("horizontal header " + horizontalHeader);

        var verticalHeader = jsonData.columnValues[1];
        // console.log("vertical header " + verticalHeader);

        // iterate over the database rows, generate table rows.
        var tableRows = []
            $(jsonData.dataTable).each(function(index, row) {
                var tableRowIndex = row[1];
                var tableColumnIndex = row[0];
                var value = jsonData.columnValues[2][row[2]];
                //console.log("row " + row);

                if (tableRows[tableRowIndex] === undefined)
                    tableRows[tableRowIndex] = [];
                tableRows[tableRowIndex][tableColumnIndex] = value;

            });

        var tableData = {
                "horizontalHeader" :horizontalHeader,
                "verticalHeader" : verticalHeader,
                "data" : tableRows
                }

           return tableData;
    }

    function interpretTableDataAsList(jsonData)
    {
        var header = jsonData.columnNames.slice(0);

        var data = [];
        var rowCount  = jsonData.dataTable.length;
        var indexCount = jsonData.columnNames.length;

        // console.log(rowCount + " " + indexCount + " " + dataCount);

        for (var i =0; i < rowCount; ++i) {
            var row = jsonData.dataTable[i].slice(0);
            for (var j = 0; j < indexCount; ++j) {
                var lookup = row[j];
                //
                if (lookup < jsonData.columnValues[j].length)
                   row[j] = jsonData.columnValues[j][lookup];
                //else
                //   console.log("row " + i + " col " +j + " lookup " + lookup + " in " + jsonData.columnValues[j].length);

            }
            data.push(row);
        }

        return  {
                "horizontalHeader" : header,
                "verticalHeader" : undefined,
                "data" : data
                }
    }

    return {
        "create" : create
    }
};
