# xtray 🧃

Tiny macOS menubar utility for Go.

![screenshot](http://i.imgur.com/gVCZMN7.png)

Adds a system tray icon with a "Quit" menu item and simple callbacks.

## Install

```bash
go get github.com/tetsuo/xtray
```

Requires macOS. Uses `cgo` and the Cocoa framework.

## Usage

```go
package main

import (
	"fmt"
	"log"

	"github.com/tetsuo/xtray"
)

func main() {
	if err := xtray.Tray(
		xtray.WithTooltip("MyApp"),
		xtray.WithIcon("icon.pdf"),
		xtray.WithLaunchCallback(func() { fmt.Println("Launched!") }),
		xtray.WithQuitCallback(func() { fmt.Println("Quitting...") }),
	); err != nil {
		log.Fatal(err)
	}
}
```

## API

```go
func Tray(opts ...Option) error
```

Launches the macOS menubar app and blocks until it is quit.

### Options

| Option             | Description                                 |
| ------------------ | ------------------------------------------- |
| `Tooltip(string)`  | Sets the tooltip shown on hover             |
| `Icon(string)`     | Path to a `.pdf` tray icon                  |
| `OnLaunch(func())` | Called after the app has finished launching |
| `OnQuit(func())`   | Called before the app terminates            |

## License

MIT
