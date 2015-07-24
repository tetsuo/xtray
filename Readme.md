# bant

create menubar apps on osx.

(_work in progress_. bant on npm registry is something else, gonna replace it.)

# example

```js
var bant = require('bant'),
    path = require('path');

var menu = bant();

menu
  .tooltip('example')
  .icon(path.resolve(__dirname + '/icon.pdf'))
  .addItem('print', function () {
    console.log('bla');
  })
  .addSeparator()
  .addItem("quit", function () {
    menu.terminate();
  })
  .run();
```

# api

```js
var bant = require('bant')
```

## var menu = bant()

## menu.tooltip(text)

## menu.icon(filepath)

## menu.addItem(title, cb)

## menu.addSeparator()

## menu.run([cb])

## menu.terminate([cb])

# license

mit