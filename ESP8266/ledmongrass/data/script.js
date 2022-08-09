/**
 * Quick & easy spectrum analyzer with audioMotion!
 *
 * For audioMotion-analyzer documentation and
 * more demos, visit https://audiomotion.dev
 */

// load module from Skypack CDN
import AudioMotionAnalyzer from './audioMotion-analyzer.js';

// audio source
const audioEl = document.getElementById('audio');

var maxValue = 255;
// instantiate analyzer
const audioMotion = new AudioMotionAnalyzer(
  document.getElementById('container'),
  {
    source: audioEl,
    // height: window.innerHeight - 50,
    // you can set other options below - check the docs!
    mode: 7,
    loRes: true,
    fftSize: 2048,
    // maxFreq: 18000,
    // smoothing: 0.9,
    barSpace: .1,
    showLeds: false,
    showFPS: true,
    useCanvas: true,
    width: 640,
    height: 270,
    lumiBars: true,
    mirror: -1,
    gradient: 'rainbow',

    onCanvasDraw: instance => {
        let eq_str = "";
            eq_str += instance.getBars().length;
        // get analyzer bars data
        for ( const bar of instance.getBars() ) {
            const value = bar.value[0];
            eq_str += ",";
            var temp = Math.round(value*value*1024);
            if (maxValue < temp)
                maxValue = temp;
            eq_str += Math.round((temp/maxValue)*255);
            if (maxValue > 255)
                maxValue--;
        }
        if ( micButton.checked) {
            if (connection.readyState === WebSocket.OPEN) {
                // connection.send(instance.getBars()[0].value[0]);
                // console.log(eq_str);
                connection.send(eq_str);
            }
        }
    }
  }
);

// display module version
document.getElementById('version').innerText = `v${AudioMotionAnalyzer.version}`;

// play stream
document.getElementById('live').addEventListener( 'click', () => {
  audioEl.src = 'https://icecast2.ufpel.edu.br/live';
  audioEl.play();
});

// file upload
// document.getElementById('upload').addEventListener( 'change', e => {
	// const fileBlob = e.target.files[0];

	// if ( fileBlob ) {
		// audioEl.src = URL.createObjectURL( fileBlob );
		// audioEl.play();
	// }
// });

const micButton = document.getElementById('mic');

micButton.addEventListener( 'change', () => {
  if ( micButton.checked ) {
    if ( navigator.mediaDevices ) {
      navigator.mediaDevices.getUserMedia( { audio: true, video: false } )
      .then( stream => {
        // create stream using audioMotion audio context
        const micStream = audioMotion.audioCtx.createMediaStreamSource( stream ); 
        // connect microphone stream to analyzer
        audioMotion.connectInput( micStream );
        // mute output to prevent feedback loops from the speakers
        audioMotion.volume = 0;
        if (connection.readyState != WebSocket.OPEN) {
            connection = new WebSocket('ws://192.168.137.4:81',['arduino']);
        }
        sound.play();
      })
      .catch( err => {
        // alert('Microphone access denied by user');
      });
    }
    else {
      alert('User mediaDevices not available');
    }
  }
  else {
    // disconnect all input audio sources
    audioMotion.disconnectInput();
    connection.close();
    sound.stop();
  }
});
