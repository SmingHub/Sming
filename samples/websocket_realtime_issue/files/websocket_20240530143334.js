var i=0;
function writeToScreen(message)
{
    let currentContent = $('#output').html();
    if (currentContent.length > 128) {
        $('#output').html(currentContent.substring(0, 128));
    }
    $('#output').prepend(message + '<br>');
}

var WebSocketManager = (function() {
    var websocket;

    function initWebSocket() {
        var wsUri = "ws://" + location.host + "/ws";
        websocket = new WebSocket(wsUri);
        websocket.onopen = function(evt) { onOpen(evt) };
        websocket.onclose = function(evt) { onClose(evt) };
        websocket.onmessage = function(evt) { onMessage(evt) };
        websocket.onerror = function(evt) { onError(evt) };
    }

    function onOpen(evt) {
        // Handle WebSocket open event
        writeToScreen("CONNECTED");
        console.log(evt);
    }

    function onClose(evt) {
        console.log(evt);
        writeToScreen("CLOSED");
        // Handle WebSocket close event
    }

    function onError(evt) {
        console.log(evt);
        writeToScreen("OnError");
        // Handle WebSocket error event
    }

    function onMessage(evt) {

        //var json = JSON.parse(evt.data);
        writeToScreen(evt.data);
        i=i+1;
        console.log('Data received from server:', i);

        //var cpu = json.Cpu;
        //$.each(json, function(code, msg) {
            //switch (code) {
            //case "AHRS":
                    //$("#roll").text(msg.roll);
                    //$("#pitch").text(msg.pitch);
                    //$("#yaw").text(msg.yaw);
                ////break;
            //}
        //});
    }
    return {
        init: initWebSocket,
        // Other functions or properties you might want to expose
    };
})();


