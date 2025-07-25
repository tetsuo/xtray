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
