var audio = new Audio();
var data = [];
var theme;
var compile;
var wave = new RIFFWAVE();
audio.loop = true;

const getQParam = (param) =>
  decodeURI(atob(new URL(window.location).searchParams.get(param)));

const setQParam = (param, val) => {
  let url = new URL(window.location);
  url.searchParams.set(param, encodeURI(btoa(val)));
  window.history.pushState({ path: url.href }, "", url.href);
};

window.onload = () => {
  theme = new Theme();
  theme.install(document.body);
  theme.start();

  Module.onRuntimeInitialized = (_) => {
    window.read = () => {
      tableFrews = document.getElementById("freqs").value;
      tableData = document.getElementById("index").value;
      waveformsData = document.getElementById("waveforms").value;
      vec = Module.gen(tableFrews, tableData, waveformsData);

      for (var i = 0; i < vec.size(); i++) {
        // TODO: figure out correct type over wasm->js embind
        data[i] = vec.get(i);
      }

      wave.header.bitsPerSample = 16;
      wave.header.sampleRate = 44100;
      wave.header.numChannels = 1;
      wave.Make(data);
      var continuePlaying;
      if (!audio.paused) {
        continuePlaying = true;
        audio.pause();
      }
      audio.src = wave.dataURI;
      if (continuePlaying) audio.play();
    };
    init();
  };

  window.setFreqs = (e) => {
    setQParam("frequencies", e.value);
    read();
  };

  window.init = () => {
    let f = (document.getElementById("freqs").value = getQParam("frequencies")),
      i = (document.getElementById("dslIndex").value = getQParam("index")),
      w = (document.getElementById("dslWaveforms").value = getQParam(
        "waveforms"
      ));
    compile(i, "index");
    compile(f, "waveforms");
    draw();
  };

  window.compile = (txt, destination) => {
    setQParam(destination, txt);
    let seq = wave_dsl.parser.parse__GT_js(txt);
    document.getElementById(destination).value = seq.join(" ");
    read();
  };
};

audio.onpause = () => {
  document.getElementById("playButton").className = "off";
};

const play = (e) => {
  e.className = !audio.paused ? "off" : "";
  if (!audio.paused) {
    audio.pause();
  } else {
    audio.play();
  }
};

const loopToggle = (e) => {
  audio.loop = !audio.loop;
  e.className = audio.loop ? "" : "off";
};

const dataURItoBlob = (dataURI) => {
  // convert base64/URLEncoded data component to raw binary data held in a string
  var byteString;
  if (dataURI.split(",")[0].indexOf("base64") >= 0)
    byteString = atob(dataURI.split(",")[1]);
  else byteString = unescape(dataURI.split(",")[1]);

  // separate out the mime component
  let mimeString = dataURI.split(",")[0].split(":")[1].split(";")[0];

  // write the bytes of the string to a typed array
  var ia = new Uint8Array(byteString.length);
  for (var i = 0; i < byteString.length; i++) {
    ia[i] = byteString.charCodeAt(i);
  }

  return new Blob([ia], { type: mimeString });
};

const download = () => saveAs(dataURItoBlob(wave.dataURI), "segmod.wav");

const share = () => {
  navigator.clipboard.writeText(window.location.href);
  alert("link copied to clipboard!");
};
