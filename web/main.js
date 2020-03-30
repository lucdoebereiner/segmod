var audio = new Audio();
var data = [];
var theme;
var compile;
audio.loop = true;

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

      var wave = new RIFFWAVE();
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

    read();
    draw();
  };

  window.compile = (e, destination) => {
    let seq = cljs.core.clj__GT_js(wave_dsl.parser.parse(e.value));
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
