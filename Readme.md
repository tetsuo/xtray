# xtray

create minimal tray apps on osx.

![screenshot](http://i.imgur.com/gVCZMN7.png)

# example

```js
var xtray = require('xtray');

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
```

# api

```js
var xtray = require('xtray')
```

## xtray(opts, [onlaunch, onquit])

Creates and launches the application.

`opts` object can have these properties:

- `icon` must be a pdf file, see [example/icon.pdf](example/icon.pdf) for an example
- `name` is the tooltip, the text which appears when you hover over the icon, and it is also appended to "Quit" text

You can provide the `onlaunch` callback that will be dispatched _after_ the application is finished launching. This doesn't block node.js i/o.

A `Quit [name]` menu item is added by default. There is no explicit terminate method; you can provide the `onquit` callback that will be dispatched _before_ the application quits.

# building

This is indeed a super minimal demonstration of building a simple node.js ⇆  Swift bridge using objc runtime. Requires a Xcode build (>=6.3-beta) which is capable of compiling Swift 1.2.

# packaging

Now, I haven't yet tested packaging a Swift framework, but it seems Xcode makes sure linked dependencies are also copied into `build` folder. I have tweaked search paths, so it should be safe to pack up `node_modules` as it is- greatly appreciated if you can open an issue if something goes wrong.

# license

mit