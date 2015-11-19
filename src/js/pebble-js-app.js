function httpWrapper(url, verb, callback) {
  console.log("Making a " + verb + " request to: " + url);
  
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(verb, url);
  xhr.send();
}


Pebble.addEventListener('ready', function(event) {
  console.log("JS Ready");
});

Pebble.addEventListener("appmessage", function(event) {
  if (event.payload.fetchData) {
    navigator.geolocation.getCurrentPosition(
      function(position) {
        var url = "http://api.open-notify.org/iss-pass.json?lat=" + position.coords.latitude + "&lon=" + position.coords.longitude;
        httpWrapper(url, "GET", function(responseText) {
          var resp = JSON.parse(responseText);
          if (resp.response && resp.response.length > 0) {
            // in seconds
            var nextPass = resp.response[0].risetime - resp.request.datetime;
            
            var hours = Math.floor(nextPass / 3600);
            var minutes = Math.floor((nextPass % 3600) / 60);
            var seconds = nextPass % 60;
            
            var nextPassText = hours + ":" + minutes + ":" + seconds;
            
            Pebble.sendAppMessage({
              "nextPass": nextPassText
            });
          } else {
            Pebble.sendAppMessage({
              "nextPass": "error"
            })
          }
        });
      }, function(err) {
        console.log("Error getting Geolocation: " + JSON.stringify(err));
      }
    );
  } else {
    console.log("Unknown event :(");
  }
});