JsonDataParser = function (params) { // ### bad name
    var data = params.jsonData.data;
    var attributesData = params.jsonData.attributes;
    var parsedAttributes = parseAttributes(attributesData);
    console.log(parsedAttributes);

    function parseAttributes(attributesData)
    {
        var parsedAttributes = {};
        $(attributesData.dataTable).each(function(index, row) {
            // console.log(index + " " + row);
            var key = attributesData.columnValues[0][row[0]];
            var value = attributesData.columnValues[1][row[1]];

            // console.log("attribute " + key + " " + value);
            parsedAttributes[key] = value;
        })
        return parsedAttributes;
    }

    function indexDimentionName(index)
    {
        return data.columnName[index];
    }

    function indexDimentionTitle(index)
    {
        console.log("DimentionTitle" + data.columnNames[index]);
        return parsedAttributes["DimentionTitle" + data.columnNames[index]];
    }

    function attributes()
    {
        return parsedAttributes;
    }

    return { "attributes" : attributes,
             "indexDimentionTitle" : indexDimentionTitle };
}


