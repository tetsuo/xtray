var addon = require('./build/addon.node'),
    path = require('path');

module.exports = function (opts, onlaunch, onquit) {
  if (!opts) opts = {};

  var icon = null;
  if (opts.icon) icon = path.resolve(opts.icon);

  addon(opts.name || null, icon, onlaunch || noop, onquit || noop);
};

function noop () {}
