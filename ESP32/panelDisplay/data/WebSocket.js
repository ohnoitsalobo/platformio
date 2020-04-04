var temp = [0], audiodata;
// var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
var connection = new WebSocket('ws://192.168.43.34:81/', ['arduino']);

connection.onopen = function () {
    connection.send('Connect ' + new Date());
};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) { 
    audiodata = e.data;
    temp = audiodata.split(",");
    // console.log('Server: ', temp);
};

connection.onclose = function(){
    console.log('WebSocket connection closed');
};

var m = 0, r = 0, g = 0, b = 0, h = 0, s = 0, v = 0, _ll = false, _rr = false;
var rr, gg, bb;

function reset(){
    connection.send('reset');
    console.log('reset'); 
    location.reload();
}

function next(){
    connection.send('next');
    console.log('next'); 
}

function sendRGB(_temp) {
    process();
    var rgb, rgbstr;
    if(_temp == "M") {
        rgb = m ;
        _ll = document.getElementById('ll').checked = true;
        _rr = document.getElementById('rr').checked = true;
    }
    if(_temp == "R") { rgb = r ; }
    if(_temp == "G") { rgb = g ; }
    if(_temp == "B") { rgb = b ; }
    if(_temp == "H") { rgb = h ; }
    if(_temp == "S") { rgb = s ; }
    if(_temp == "V") { rgb = v ; }
    rgbstr = _temp + rgb.toString(16);
    if(_ll && _rr) { rgbstr += 'B'; }
    else if(!_ll && !_rr) { rgbstr += 'X'; }
    else{
             if(_ll) { rgbstr += 'L'; }
        else if(_rr) { rgbstr += 'R'; }
    }
    // var rgbstr = '#'+ rgb.toString(16);    
    console.log('RGB: ' + rgbstr); 
    connection.send(rgbstr);
}

function process(){
    m =  document.getElementById('m' ).value;
    r =  document.getElementById('r' ).value; var _r = r * 100/255;
    g =  document.getElementById('g' ).value; var _g = g * 100/255;
    b =  document.getElementById('b' ).value; var _b = b * 100/255;
    h =  document.getElementById('h' ).value;
    s =  document.getElementById('s' ).value; var _s = s * 100/255;
    v =  document.getElementById('v' ).value; var _v = v * 100/255;
    _ll = document.getElementById('ll').checked;
    _rr = document.getElementById('rr').checked;
    
    var mode = "";
    if     (m==0) { mode = "Sound response"; }
    else if(m==1) { mode = "Patterns";       }
    else if(m==2) { mode = "Manual";         }
    
    document.getElementById('mval' ).innerHTML = mode;
    document.getElementById('rval' ).innerHTML = _r.toFixed(0) + '%' ;
    document.getElementById('gval' ).innerHTML = _g.toFixed(0) + '%' ;
    document.getElementById('bval' ).innerHTML = _b.toFixed(0) + '%' ;
    document.getElementById('hval' ).innerHTML = h ;
    document.getElementById('sval' ).innerHTML = _s.toFixed(0) + '%' ;
    document.getElementById('vval' ).innerHTML = _v.toFixed(0) + '%' ;
    
    // var en = false;
    // if(m != 2){
        // en = true;
    // }
    // document.getElementById('r' ).disabled = en;
    // document.getElementById('g' ).disabled = en;
    // document.getElementById('b' ).disabled = en;
    // document.getElementById('h' ).disabled = en;
    // document.getElementById('s' ).disabled = en;
}

function setup(){
    // createCanvas(25*28, 255);
    createCanvas(1020, 255);
    // frameRate(35);
}

function draw(){
    background(0);
    if(m == 0){
        noStroke();
        var samples = 254.0;
        var xx = width/samples;
        var yy = height/255.0;
        var H = 316.0/samples;
        for(var i = 0; i < samples; i++){
            setHue(i*H);
            fill(color(rr, gg, bb));
            rect(i*xx, height, xx, -int(temp[i+1])*yy);
        }
    }
}

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