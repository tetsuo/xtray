//go:build darwin
// +build darwin

package xtray

/*
#cgo darwin CFLAGS:  -fobjc-arc -x objective-c
#cgo darwin LDFLAGS: -framework Cocoa
#include <stdlib.h>
#include "tray_darwin.h"
*/
import "C"

import (
	"fmt"
	"os"
	"runtime"
	"unsafe"
)

type cfg struct {
	tooltip  string
	iconPath string
	launch   func()
	quit     func()
}

//export goOnLaunch
func goOnLaunch() {
	if current.launch != nil {
		current.launch()
	}
}

//export goOnQuit
func goOnQuit() {
	if current.quit != nil {
		current.quit()
	}
}

var current cfg // holds the active tray's callbacks

func runCore(c cfg) error {
	if c.iconPath != "" {
		if _, err := os.Stat(c.iconPath); err != nil {
			return fmt.Errorf("icon path: %w", err)
		}
	}

	current = c // make callbacks reachable from C

	// c-strings
	var cTip, cIcon *C.char
	if c.tooltip != "" {
		cTip = C.CString(c.tooltip)
		defer C.free(unsafe.Pointer(cTip))
	}
	if c.iconPath != "" {
		cIcon = C.CString(c.iconPath)
		defer C.free(unsafe.Pointer(cIcon))
	}

	// Cocoa must live on the main OS thread
	runtime.LockOSThread()
	C.runTray(cTip, cIcon) // blocks until NSApp.terminate:
	runtime.UnlockOSThread()

	return nil
}
