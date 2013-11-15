var initialized = false;

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
  Pebble.openURL('https://dl.dropboxusercontent.com/u/1623607/configurable.html');
});

Pebble.addEventListener("webviewclosed", function(e) {
  console.log("configuration closed");
  // webview closed
  var options = JSON.parse(decodeURIComponent(e.response));
  var transactionId = Pebble.sendAppMessage( { "CellNum": options.CellNum },
	  function(e) {
	    console.log("Successfully delivered message with transactionId="
	      + e.data.transactionId);
	  },
	  function(e) {
	    console.log("Unable to deliver message with transactionId="
	      + e.data.transactionId
	      + " Error is: " + e.error.message);
	  }
  );
  console.log("Options = " + JSON.stringify(options));
});