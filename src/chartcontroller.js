ChartController = function(params) {
    var data = params.data.data;
    //console.log(params.data.attributes);

    var dataParser = new JsonDataParser({ "jsonData" : params.data });
    var attributes = dataParser.attributes();
    var dataArray = convertData(data);

    function selectSeriesDimention(data)
    {
        // Select which dimentions becomes the series
        // and X axis (Y axis is the result/measurement)
        // Several approaches can be used:
        // 1) Hardcode index 0.
        return 0;
        // (TODO!)
        // 2) Auto-detect, use shortest dimention as series
        // 2.5) Use the non-continious dimention
        // 3) Manually specified by meta-data.
    }
    function selectXdimention(data)
    {
        // See seriesDimention above;
        return 1;
    }

    function selectResultDimention(data)
    {
        return 2;
    }

    function convertData(data)
    {
        // convert the column/row-based in-data to an
        // array of x-y arrays. [[x1,y1],[x2,y2], etc]
        //
        // Also converts the x axis from seconds-since-1970
        // to milliseconds-since-1970 if the timeChart attribute
        // is set.
        var seriesArray = [];
        var seriesDimention = selectSeriesDimention(data);
        var xDimention = selectXdimention(data);
        var resultDimention = selectResultDimention(data);
        $(data.dataTable).each(function(index, value) {
               // value is [0, 1, 100] (index1, index2, result)
               var seriesIndex = value[seriesDimention];
               if (seriesArray[seriesIndex] === undefined)
                   seriesArray[seriesIndex] = [];
               var xValue = data.columnValues[xDimention][value[xDimention]];
               if (attributes["timeChart"] == "true")
                    xValue*= 1000;
               var yValue = data.columnValues[resultDimention][value[resultDimention]];
               seriesArray[seriesIndex].push([xValue, yValue]);
         });
        return seriesArray;
    }

    function roundValue(value, unit)
    {
        var adjustedValue = Math.ceil(value / unit);
        var adjustedValueMagnitude = String(adjustedValue).length;

        var roundedValue = Math.ceil(adjustedValue / Math.pow(10, adjustedValueMagnitude -1))
                          * Math.pow(10, adjustedValueMagnitude -1);
        return roundedValue;
    }

    // Creates tick marks for the axis.
    function createTickMarks(dataArray, axis, tickCount, startFromZero) // ### fix API
    {
        // find min, max, range
        var min = Number.POSITIVE_INFINITY;
        var max = Number.NEGATIVE_INFINITY;
        $(dataArray).each(function(index, series) {
            $(series).each(function(index, pair) {
                var y = pair[axis];
                if (y < min)
                    min = y
                if (y > max)
                    max = y
            });
        });
        if (startFromZero)
            min = 0;

        var range = max - min;
        //console.log("raw " + range + " " + min + " " + max);

        var magnitude = String(max).length;

        var unit;
        var unitLabel;
        if (magnitude > 7) {
            unit = 1000000;
            unitLabel = "M"
        } else if (magnitude > 4) {
            unit = 1000;
            unitLabel = "K"
        } else {
            unit = 1;
            unitLabel = ""
        }

        var roundedRange = roundValue(range, unit);
        var roundedMin = roundValue(min, unit);


        // heuristics for padding the graph:
        var step;
        if (tickCount < 0) {
            // if the tick count is based on the array lenght:
            // keep the step accurate, add one tick to pad.
            tickCount = Math.min(dataArray[0].length, 20);

            //console.log("range " + roundedRange + "count " + tickCount)
            step = roundValue(range / (tickCount - 1), unit);
            tickCount += 1;
        } else {
              step = roundedRange / (tickCount - 1);
        }

        // Generate tick mark value, label pairs
        // format: [[value1, label1],[vale2, label2]]
        var ticks = [];
        for (var i = 0; i < tickCount; ++i) {
            var value = (roundedMin + i * step) * unit;
            var label = Math.floor(roundedMin + i * step) + unitLabel;
            ticks.push([value, label]);
        }
        return ticks;
    }


   function convertSeries(data)
   {
       var seriesDimention = selectSeriesDimention(data);
       var series = []
       $(data.columnValues[seriesDimention]).each(function(index, value) {
           series.push({ label: value,
                         "showMarker" : attributes["chartMarkers"] == "true"
                     });
       });
       return series;
   }
   function create(container) {
       $.jqplot.config.enablePlugins = true;
       var isTimeChart = (attributes["timeChart"] == "true");

       plot1 = $.jqplot(container.attr("id"), dataArray, {
           title: attributes["Title"],
           legend: {
               show:true,
               renderer:$.jqplot.EnhancedLegendRenderer
           },
           series: convertSeries(data),
           Highlighter: { show: true },

           cursor: {showTooltip:false, zoom:true, clickReset:true},

           axes: {
               yaxis: {
                   label : dataParser.indexDimentionTitle(selectResultDimention()),
                   labelRenderer: $.jqplot.CanvasAxisLabelRenderer,
                   ticks : createTickMarks(dataArray, 1, 6, true),
                   min: 0
               },
               xaxis: {
                   autoscale : true,
                   label : dataParser.indexDimentionTitle(selectXdimention()),
                   renderer: isTimeChart ? $.jqplot.DateAxisRenderer : undefined,
                   tickOptions : isTimeChart ? { formatString : " %B %Y" } : undefined,
                   labelRenderer: $.jqplot.CanvasAxisLabelRenderer,
                   ticks : isTimeChart ? undefined :createTickMarks(dataArray, 0, -1, false)
              }
           }

       });


       //plot1.axes.xaxis.renderer = $.jqplot.DateAxisRenderer;
       // Add description as tooltip if set
       if (attributes["Description"] !== undefined) {
           $(container).attr("title", "sdfsdf" +  attributes["Description"])
       }
   }
   return { "create" : create };
};

