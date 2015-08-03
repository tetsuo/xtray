var xtray = require('..');

xtray({
  name: 'Demo',
  icon: __dirname + '/icon.pdf'
}, function onlaunch () {
  console.log('launched');
  setInterval(function () {
    console.log('loop is safe');
  }, 1000);
}, function onquit () {
  console.log('will quit');
});