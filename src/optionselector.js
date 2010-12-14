OptionSelector = function (params) {
    var title = params.title;
    var options = params.options;
    var callback = params.callback;
    var currentOption = options[0];

    function create(rootElement) {
        rootElement.empty();
        rootElement.append(createHtml(rootElement));
        callback(currentOption);
    }

    function createHtml(rootElement)
    {
        var container = $("<div>");
        container.append(document.createTextNode(title + " [ "));

        for (var j = 0; j < options.length; j++) {
            var option = options[j]

            if (option == currentOption) {
                container.append(document.createTextNode(option));
            } else {
                var link = $('<a href>');
                // Add click handler. Wrap in an anonymous,
                // self-calling function to capture variables.
                (function() {
                    var capturedOption = option;
                    var capturedRootElement = rootElement;
                    link.click(function(event) {
                       event.preventDefault();

                       currentOption = capturedOption;
                       rootElement.empty();
                       rootElement.append(createHtml(rootElement));

                       callback(capturedOption)
                   });
                 })();

                link.append(document.createTextNode(option));
                container.append(link);
            }

            container.append(document.createTextNode(" "));
        }

        container.append(document.createTextNode(" ]"));

        return container;
    }

    return { "create" : create }
}
