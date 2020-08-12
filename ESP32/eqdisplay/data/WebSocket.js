var temp = [0], audiodata;
// var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);
var connection = new WebSocket('ws://192.168.43.153:81/', ['arduino']);

connection.onopen = function () {
    connection.send('Connect ' + new Date());
};

connection.onerror = function (error) {
    console.log('WebSocket Error ', error);
};

connection.onmessage = function (e) { 
    audiodata = e.data;
    temp = audiodata.split(",");
    console.log('Server: ', temp);
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
    
    var en = false;
    if(m != 2){
        en = true;
    }
    document.getElementById('r' ).disabled = en;
    document.getElementById('g' ).disabled = en;
    document.getElementById('b' ).disabled = en;
    document.getElementById('h' ).disabled = en;
    document.getElementById('s' ).disabled = en;
}

let img;
function setup(){
    let cnv = createCanvas(550, 550);
    cnv.parent('p5js');
    frameRate(25);
    // img = loadImage('hue_square.png');
    img = loadImage('hue_circle.png');

    // colorMode(HSB, 255);
    // for(let i = 0; i < width; i++){
        // for(let j = 0; j < height/2; j++){
            // stroke(i*224.0/width, j, 255);
            // point(i, j);
            // stroke(i*224.0/width, 255, 255-j);
            // point(i, j+height/2);
        // }
    // }
}

let c = [0, 0, 0];
function draw(){
    // colorSquare();
    colorWheel();
}

function colorSquare(){
    colorMode(HSB, 255);
    for(let i = 0; i < 255; i++){
        stroke(255, i, 255);
        line(0, i+25, 25, i+25);
        stroke(255, 255, 255-i);
        line(0, i+275, 25, i+275);
        stroke(224, i, 255);
        line(525, i+25, 550, i+25);
        stroke(224, 255, 255-i);
        line(525, i+275, 550, i+275);
    }
    image(img, 25, 25);
    if(m == 2 && mouseIsPressed && mouseX <= width && mouseX >= 0 && mouseY <= height && mouseY >= 0){
        colorMode(RGB);
        c = get(mouseX, mouseY);
        fill(c); noStroke();
        rect(0, 0, width, 25);
        // ellipse(mouseX, mouseY, 50, 50);
        document.getElementById('r').value = c[0];
        document.getElementById('g').value = c[1];
        document.getElementById('b').value = c[2];
        document.getElementById('rval' ).innerHTML = (c[0]/255*100).toFixed(0) + '%';
        document.getElementById('gval' ).innerHTML = (c[1]/255*100).toFixed(0) + '%';
        document.getElementById('bval' ).innerHTML = (c[2]/255*100).toFixed(0) + '%';

        let rgbstr;
        _ll = document.getElementById('ll').checked;
        _rr = document.getElementById('rr').checked;
        if(_ll && _rr) { rgbstr = 'B'; }
        else if(!_ll && !_rr) { rgbstr = 'X'; }
        else{
                 if(_ll) { rgbstr = 'L'; }
            else if(_rr) { rgbstr = 'R'; }
        }
        connection.send('R'+ c[0] + rgbstr);
        connection.send('G'+ c[1] + rgbstr);
        connection.send('B'+ c[2] + rgbstr);
    }
    noStroke();
    fill(0);
    rect(0, height-25, width, height);
    fill(255);
    rect(0, 15, width, 10);
}

function colorWheel(){
    // colorMode(HSB, 255);
    // translate(width/2, height/2);
    // rotate(PI);
    // angleMode(DEGREES);
    // fill(255); noStroke();
    // let radius = 0.9*width/2;
    // circle(0, 0, radius*2/0.98);
    // strokeWeight(4);
    // let deg = 360;
    // for(let j = 0; j < deg; j++){
        // rotate(360/deg);
        // for(let i = 0; i < radius; i++){
            // let _hue = j/deg*255;
            // let _sat = (i < radius/2) ? 255 : 255-((i-radius/2)/(radius/2))*255;
            // let _val = (i > radius/2) ? 255 : (i/(radius/2))*255;
            // stroke(_hue, _sat, _val);
            // point(0, i);
        // }
    // }
    // noLoop();
    image(img, 0, 0);
    translate(width/2, height/2);
    noFill(); stroke(c); strokeWeight(184);
    circle(0, 0, height*1.25);
    if(m == 2 && mouseIsPressed && mouseX <= width && mouseX >= 0 && mouseY <= height && mouseY >= 0){
        colorMode(RGB);
        c = get(mouseX, mouseY);
        noFill(); stroke(c); strokeWeight(184);
        circle(0, 0, height*1.25);
        // ellipse(mouseX, mouseY, 50, 50);
        document.getElementById('r' ).value = c[0];
        document.getElementById('g' ).value = c[1];
        document.getElementById('b' ).value = c[2];
        document.getElementById('rval' ).innerHTML = (c[0]/255*100).toFixed(0) + '%';
        document.getElementById('gval' ).innerHTML = (c[1]/255*100).toFixed(0) + '%';
        document.getElementById('bval' ).innerHTML = (c[2]/255*100).toFixed(0) + '%';

        let rgbstr;
        _ll = document.getElementById('ll').checked;
        _rr = document.getElementById('rr').checked;
        if(_ll && _rr) { rgbstr = 'B'; }
        else if(!_ll && !_rr) { rgbstr = 'X'; }
        else{
                 if(_ll) { rgbstr = 'L'; }
            else if(_rr) { rgbstr = 'R'; }
        }
        connection.send('R'+ c[0] + rgbstr);
        connection.send('G'+ c[1] + rgbstr);
        connection.send('B'+ c[2] + rgbstr);
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