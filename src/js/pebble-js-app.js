function httpWrapper(url, verb, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(verb, url);
  xhr.send();
}

function loop() {
  navigator.geolocation.getCurrentPosition(function(position) {
    var url = "http://api.open-notify.org/iss-pass.json?lat=" + position.coords.latitude + "&lon=" + position.coords.longitude;
    console.log("Fetching data from: " + url);
    httpWrapper(url, "GET", function(responseText) {
      // Parse the result
      var resp = JSON.parse(responseText);
      if (resp.response && resp.response.length > 0) {
        // if it was successful, determine how far away the ISS is
        var timeToPass = resp.response[0].risetime - resp.request.datetime;
        var duration = resp.response[0].duration;
        console.log("ISS is " + timeToPass + " seconds away");
        
        // If ISS is > 15 minutes and 30 seconds away
        if (timeToPass > 930) {
          // Grap pass times in 15 minutes
          setTimeout(loop, 900000);
          return;
        } else {
          // If ISS is < 15 minutes away, send a message to the watch
          Pebble.sendAppMessage({
            "timeToPass": timeToPass,
            "duration": duration
          });
            
          // Check for pass times 1 minute after the end of the current pass
          setTimeout(loop, (timeToPass+duration+60)*1000);
          return;
        }
      } else {
        // If there wasn't a response array
        console.log("Error getting ISS pass times");
      }
    });
  }, function(err) {
    // if we couldn't get location
    console.log("Could not get location");
  });
}

// When JS is ready, start the pass
Pebble.addEventListener('ready', function(event) {
  loop();
});