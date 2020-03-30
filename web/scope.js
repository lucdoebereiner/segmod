var ctx = document.getElementById("scope").getContext("2d");

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
  var width = ctx.canvas.width;
  var height = ctx.canvas.height;
  var scaling = 30 / 32768; //(height + 32768) / (32768 * 2);
  var timeData = data;
  var risingEdge = 0;
  var edgeThreshold = 5;

  ctx.fillStyle = hexToRGB(theme.active.background);
  ctx.fillRect(0, 0, width, height);

  ctx.lineWidth = 3;
  ctx.strokeStyle = hexToRGB(theme.active.f_high);
  ctx.beginPath();

  // No buffer overrun protection
  while (timeData[risingEdge++] - 16000 > 0 && risingEdge <= width);
  if (risingEdge >= width) risingEdge = 0;

  while (timeData[risingEdge++] - 16000 < edgeThreshold && risingEdge <= width);
  if (risingEdge >= width) risingEdge = 0;

  for (var x = risingEdge; x < timeData.length && x - risingEdge < width; x++)
    ctx.lineTo(x - risingEdge, (height - timeData[x] * scaling) / 2);

  ctx.stroke();

  requestAnimationFrame(draw);
}
