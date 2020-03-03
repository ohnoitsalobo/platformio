var temp = [0], msgeq7;
var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
// var connection = new WebSocket('ws://192.168.43.123:81/', ['arduino']);
connection.onopen = function () {
    connection.send('Connect ' + new Date());
};
connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};
connection.onmessage = function (e) { 
    msgeq7 = e.data;
    temp = msgeq7.split(",");
    // console.log('Server: ', temp);
};
connection.onclose = function(){
    console.log('WebSocket connection closed');
};

var m = 0, r = 0, g = 0, b = 0, h = 0, s = 0, v = 0, g1 = 0, g2 = 0;

function nextPattern(){
    connection.send('next');
}

function sendRGB(temp) {
    process();
    var rgb, rgbstr;
    if(temp == "M") { rgb = m ; }
    if(temp == "R") { rgb = r ; }
    if(temp == "G") { rgb = g ; }
    if(temp == "B") { rgb = b ; }
    if(temp == "H") { rgb = h ; }
    if(temp == "S") { rgb = s ; }
    if(temp == "V") { rgb = v ; }
    if(temp == "Y") { rgb = g1; }
    if(temp == "Z") { rgb = g2; }
    rgbstr = temp + rgb.toString(16);    
    // var rgbstr = '#'+ rgb.toString(16);    
    console.log('RGB: ' + rgbstr); 
    connection.send(rgbstr);
}

function process(){
    m =  document.getElementById('m' ).value;
    r =  document.getElementById('r' ).value;
    g =  document.getElementById('g' ).value;
    b =  document.getElementById('b' ).value;
    h =  document.getElementById('h' ).value;
    s =  document.getElementById('s' ).value;
    v =  document.getElementById('v' ).value;
    g1 = document.getElementById('ga').value;
    g2 = document.getElementById('gb').value;
    var mode = "";
    if     (m==0) { mode = "Sound response"; }
    else if(m==1) { mode = "Patterns"; }
    else if(m==2) { mode = "Manual"; }
         document.getElementById('mval' ).innerHTML = mode;
         document.getElementById('rval' ).innerHTML = r ;
         document.getElementById('gval' ).innerHTML = g ;
         document.getElementById('bval' ).innerHTML = b ;
         document.getElementById('hval' ).innerHTML = h ;
         document.getElementById('sval' ).innerHTML = s ;
         document.getElementById('vval' ).innerHTML = v ;
         document.getElementById('gaval').innerHTML = g1;
         document.getElementById('gbval').innerHTML = g2;
    var en = false;
    if(m != 2){
        en = true;
    }
    document.getElementById('r' ).disabled = en;
    document.getElementById('g' ).disabled = en;
    document.getElementById('b' ).disabled = en;
    document.getElementById('h' ).disabled = en;
    document.getElementById('s' ).disabled = en;
    document.getElementById('ga').disabled = en;
    document.getElementById('gb').disabled = en;
}

function setup(){
    createCanvas(700, 255);
    // frameRate(2);
}

function draw(){
    background(0);
    if(temp[0] == 'E' && m == 0){
        for(var i = 0; i < 7; i++){
            var H = i/6*320;
            setHue(H);
            fill(color(rr, gg, bb));
            rect(0+i*100, height, 100, -int(temp[i+1])/1024*255);
            // rect(0+i*100, height, 100, -height+random(255));
        }
    }
}

var rr, gg, bb;
function setHue(hue) { // Set the RGB LED to a given hue (color) (0° = Red, 120° = Green, 240° = Blue)
  hue %= 360;                   // hue is an angle between 0 and 359°
  var radH = hue*3.142/180;   // Convert degrees to radians
  var rf, gf, bf;
  
  if(hue>=0 && hue<120){        // Convert from HSI color space to RGB              
    rf = cos(radH*3/4);
    gf = sin(radH*3/4);
    bf = 0;
  } else if(hue>=120 && hue<240){
    radH -= 2.09439;
    gf = cos(radH*3/4);
    bf = sin(radH*3/4);
    rf = 0;
  } else if(hue>=240 && hue<360){
    radH -= 4.188787;
    bf = cos(radH*3/4);
    rf = sin(radH*3/4);
    gf = 0;
  }
  rr = rf*rf*1023;
  gg = gf*gf*1023;
  bb = bf*bf*1023;
}
