var bant = require('..'),
    path = require('path');

setInterval(function () {
  console.log('loop is ok');
}, 1000);

var app = bant();

app
  .tooltip('demo')
  .icon(path.resolve(__dirname + '/icon.pdf'))
  .addItem("Print things", function () {
    console.log('printing things')
  })
  .addItem("Quit", function () {
    app.terminate(function () {
      console.log('app will terminate');
    });
  })
  .run(function () {
    console.log('app did launched');
  });