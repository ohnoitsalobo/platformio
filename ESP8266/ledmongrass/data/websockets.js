// var connection = new WebSocket('ws://'+location.hostname+':81',['arduino']);
var connection = new WebSocket('ws://192.168.137.4:81',['arduino']);
function openWebSocket(){
    connection = new WebSocket('ws://192.168.137.4:81',['arduino']);
}

if(connection.readyState === WebSocket.OPEN){
    document.getElementById('reconnect');
}
connection.onopen = function () {
    // connection.send('Connect ' + new Date());
};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

connection.onclose = function(){
    console.log('WebSocket connection closed');
};

