Pebble.addEventListener("ready",
    function (e) {
        console.log("Hello world! - Sent from your javascript application.");

        getRescueTimeProductivity();
    }
);

Pebble.addEventListener('showConfiguration', function (e) {
    // Show config page
    Pebble.openURL('http://192.168.2.12:8000/settings.html');
});

Pebble.addEventListener("webviewclosed", function (e) {
    var rt = typeof e.response,
        settings = (rt === "undefined" ? {} : JSON.parse(decodeURIComponent(e.response)));
    if (Object.keys(settings).length > 0) {
        console.log("Got settings: " + settings);
        Pebble.sendAppMessage(settings);
    }
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function (e) {
        console.log("AppMessage received!");
        getRescueTimeProductivity();
    }
);

var getRescueTimeProductivity = function () {
    var url = "https://www.rescuetime.com/anapi/data?key=" + localStorage.getItem("apiKey") + "&format=json&perspective=interval&restrict_kind=productivity&resolution_time=minute&restrict_kind=productivity";
    xhrRequest(url, 'GET',
        function(responseText) {
            // responseText contains a JSON object with RescueTime productivity data
            var json = JSON.parse(responseText);

            // looking only at the most recent productivity records, therefore storing max date
            var maxDate = null;
            // flag to end the loop
            var shouldStop = false;

            // will calculate the productivity as weighted mean, based on the number of seconds each productivity level was reached
            var sum = 0;
            var weight = 0;

            for(var i = json.rows.length - 1; i >= 0 && !shouldStop; i--) {
                var productivityData = json.rows[i];
                var dataDate = Date.parse(productivityData[0]);

                if(maxDate == null) {
                    maxDate = dataDate;
                } else {
                    if(dataDate < maxDate) {
                        shouldStop = true;
                    } else {
                        sum = sum + parseInt(productivityData[1]) * parseInt(productivityData[3]);
                        weight = weight + parseInt(productivityData[1]);
                    }
                }
            }

            var productivity = sum / weight;
            console.log("Calculated productivity level to: " + productivity);
        }
    );
};

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};