// Adapted from https://codepen.io/ContemporaryInsanity/pen/Mwvqpb

var AudioContext =
  window.AudioContext || // Default
  window.webkitAudioContext || // Safari and old versions of Chrome
  false;

var audio_ctx = new AudioContext();
var analyser = audio_ctx.createAnalyser();
var source = audio_ctx.createMediaElementSource(audio);
source.connect(audio_ctx.destination);
source.connect(analyser);

var canvas_ctx = document.getElementById("scope").getContext("2d");

function hexToRGB(hex, alpha) {
  var r = parseInt(hex.slice(1, 3), 16),
    g = parseInt(hex.slice(3, 5), 16),
    b = parseInt(hex.slice(5, 7), 16);

  if (alpha) {
    return "rgba(" + r + ", " + g + ", " + b + ", " + alpha + ")";
  } else {
    return "rgb(" + r + ", " + g + ", " + b + ")";
  }
}

function draw() {
  var width = canvas_ctx.canvas.width;
  var height = canvas_ctx.canvas.height;
  var timeData = new Uint8Array(analyser.frequencyBinCount);
  var scaling = height / 256;
  var risingEdge = 0;
  var edgeThreshold = 8;

  analyser.getByteTimeDomainData(timeData);

  canvas_ctx.fillStyle = hexToRGB(theme.active.background, 0.8);
  canvas_ctx.fillRect(0, 0, width, height);

  canvas_ctx.lineWidth = 1;
  canvas_ctx.strokeStyle = hexToRGB(theme.active.f_high);
  canvas_ctx.beginPath();

  // No buffer overrun protection
  while (timeData[risingEdge++] - 128 > 0 && risingEdge <= width);
  if (risingEdge >= width) risingEdge = 0;

  while (timeData[risingEdge++] - 128 < edgeThreshold && risingEdge <= width);
  if (risingEdge >= width) risingEdge = 0;

  for (var x = risingEdge; x < timeData.length && x - risingEdge < width; x++)
    canvas_ctx.lineTo(x - risingEdge, height - timeData[x] * scaling);

  canvas_ctx.stroke();

  requestAnimationFrame(draw);
}
