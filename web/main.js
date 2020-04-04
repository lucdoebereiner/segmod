var audio = new Audio();
var data = [];
var theme;
var compile;
var wave = new RIFFWAVE();
audio.loop = true;
var randomSeed;

const getQParam = (param, noB64) => {
  let p = new URL(window.location).searchParams.get(param);
  if (noB64) {
    return p ? decodeURI(p) : "";
  } else {
    return p ? decodeURI(atob(p)) : "";
  }
};

const setQParam = (param, val, noB64) => {
  let url = new URL(window.location);
  if (noB64) {
    url.searchParams.set(param, encodeURI(val));
  } else {
    url.searchParams.set(param, encodeURI(btoa(val)));
  }
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
    let f = getQParam("frequencies");
    if (f) {
      document.getElementById("freqs").value = f;
    }

    let i = getQParam("index");
    if (i) {
      document.getElementById("dslIndex").value = i;
    } else {
      i = document.getElementById("dslIndex").value;
    }

    let w = getQParam("waveforms");
    if (w) {
      document.getElementById("dslWaveforms").value = w;
    } else {
      w = document.getElementById("dslWaveforms").value;
    }

    randomSeed = parseInt(getQParam("seed",true));
    if (!randomSeed) {
      randomSeed = Math.floor(Math.random() * 9998) + 1;
      setQParam("seed", randomSeed,true)
    }
    document.getElementById("seed").value = randomSeed;

    compile(i, "index", false);
    compile(w, "waveforms", true);

    draw();
  };

  window.compile = (txt, destination, shouldRender) => {
    setQParam(destination, txt);
    let seq = wave_dsl.parser.parse__GT_js(txt, randomSeed);
    document.getElementById(destination).value = seq.join(" ");
    shouldRender ? read() : undefined;
  };
};

audio.onpause = () => {
  document.getElementById("playButton").className = "off";
};

const play = (e) => {
  audio_ctx.resume();
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

const changeSeed = (e) => {
  randomSeed = e.value;
  setQParam("seed", randomSeed,true);
  compile(document.getElementById("dslWaveforms").value, "waveforms", true);
  compile(document.getElementById("dslIndex").value, "index", true);
}

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
