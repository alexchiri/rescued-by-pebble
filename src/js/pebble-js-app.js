Pebble.addEventListener("ready",
    function(e) {
        console.log("Hello world! - Sent from your javascript application.");
    }
);

Pebble.addEventListener('showConfiguration', function(e) {
    // Show config page
    Pebble.openURL('http://0f8f28fe275e3a043777-67ab80ec00c7299bd1255995bf933a71.r1.cf2.rackcdn.com/settings.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
    var rt = typeof e.response,
        settings = (rt === "undefined" ? {} : JSON.parse(decodeURIComponent(e.response)));
    if(Object.keys(settings).length > 0) {
        console.log("Got settings: " + settings);
        Pebble.sendAppMessage(settings);
    }
});