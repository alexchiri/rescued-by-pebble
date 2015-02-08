Pebble.addEventListener('showConfiguration', function (e) {
    // Show config page
    Pebble.openURL('http://0f8f28fe275e3a043777-67ab80ec00c7299bd1255995bf933a71.r1.cf2.rackcdn.com/settings_v2.html');
});

Pebble.addEventListener("webviewclosed", function (e) {
    var rt = typeof e.response,
        settings = (rt === "undefined" ? {} : JSON.parse(decodeURIComponent(e.response)));
    if (Object.keys(settings).length > 0) {
        console.log("Got settings: " + settings);
        //Pebble.sendAppMessage(settings);
    }
});

var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
        callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

var getRescueTimeProductivity = function () {
    var url = "https://www.rescuetime.com/anapi/data?key=" + localStorage.getItem("apiKey") + "&format=json&perspective=interval&restrict_kind=productivity&resolution_time=minute&restrict_kind=productivity";
    xhrRequest(url, 'GET',
        function (responseText) {
            // responseText contains a JSON object with RescueTime productivity data
            var json = JSON.parse(responseText);

            // looking only at the most recent productivity records, therefore storing max date
            var maxDate = null;
            // flag to end the loop
            var shouldStop = false;

            // will calculate the productivity as weighted mean, based on the number of seconds each productivity level was reached
            var sum = 0;
            var weight = 0;

            for (var i = json.rows.length - 1; i >= 0 && !shouldStop; i--) {
                var productivityData = json.rows[i];
                var dataDate = Date.parse(productivityData[0]);

                if (maxDate === null) {
                    maxDate = dataDate;
                }

                if (dataDate < maxDate) {
                    shouldStop = true;
                } else {
                    sum = sum + parseInt(productivityData[1]) * (parseInt(productivityData[3]) + 2) * 25;
                    weight = weight + parseInt(productivityData[1]);
                }
            }

            console.log("Sum is: " + sum);
            console.log("Weight is: " + weight);

            var productivity = Math.round(sum / weight);
            console.log("Calculated productivity level to: " + productivity);

            var dictionary = {
                "productivity": productivity
            };

            Pebble.sendAppMessage(dictionary,
                function (e) {
                    console.log("RescueTime productivity info sent to Pebble successfully!");
                },
                function (e) {
                    console.log("Error sending RescueTime productivity info to Pebble!");
                }
            );
        }
    );
};

Pebble.addEventListener("ready",
    function (e) {
        console.log("Hello world! - Sent from your javascript application.");

        getRescueTimeProductivity();
    }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function (e) {
        console.log("AppMessage received!");

        getRescueTimeProductivity();
    }
);