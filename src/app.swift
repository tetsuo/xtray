import Cocoa

class Callback {
  typealias callback_t =
    CFunctionPointer<(UnsafePointer<Void>) -> Void>
  
  typealias ctx_t = UnsafePointer<Void>
  
  var callback: callback_t
  var ctx: ctx_t
  
  init (cb: COpaquePointer, ctx: COpaquePointer) {
    self.callback = callback_t(cb)
    self.ctx = ctx_t(ctx)
  }
  
  func call () {
    next(self.callback, self.ctx)
  }
}

func FromCString (ptr: COpaquePointer) -> String {
  return String.fromCString(UnsafeMutablePointer<CChar>(ptr))!
}

class ApplicationDelegate: NSObject, NSApplicationDelegate {
  let statusItem = NSStatusBar.systemStatusBar().statusItemWithLength(-2)
  
  func applicationDidFinishLaunching (notification: NSNotification) {
    let menu = NSMenu()
    var title = "Quit"
    if ((statusItem.toolTip) != nil) {
      title += " " + statusItem.toolTip!
    }
    menu.addItem(NSMenuItem(title: title, action: Selector("terminate:"), keyEquivalent: ""))
    statusItem.menu = menu
    statusItem.highlightMode = true
  }
}

public class Application: NSObject {
  let app = NSApplication.sharedApplication()
  let delegate = ApplicationDelegate()
  
  var launchHandler: Callback?
  var terminateHandler: Callback?
  
  public func setToolTip (value: COpaquePointer) {
    delegate.statusItem.toolTip = FromCString(value)
  }
  
  public func setIcon (path: COpaquePointer, name: COpaquePointer) {
    if let button = delegate.statusItem.button {
      let bundle = NSBundle(path: FromCString(path))
      let imagePath = bundle?.pathForResource(FromCString(name), ofType: "pdf")
      button.image = NSImage(contentsOfFile: imagePath!)
      button.image!.setTemplate(true)
    }
  }
  
  public func setTerminateHandler (cb: COpaquePointer, ctx: COpaquePointer) {
    NSNotificationCenter.defaultCenter()
      .addObserver(self,
        selector: Selector("willTerminate:"),
        name: NSApplicationWillTerminateNotification,
        object: nil)
    terminateHandler = Callback(cb: cb, ctx: ctx)
  }
  
  public func setLaunchHandler (cb: COpaquePointer, ctx: COpaquePointer) {
    NSNotificationCenter.defaultCenter()
      .addObserver(self,
        selector: Selector("didFinishLaunching:"),
        name: NSApplicationDidFinishLaunchingNotification,
        object: nil)
    launchHandler = Callback(cb: cb, ctx: ctx)
  }

  public func run () {
    app.setActivationPolicy(NSApplicationActivationPolicy.Accessory)
    app.activateIgnoringOtherApps(true)
    app.delegate = delegate
    app.run()
  }
  
  func willTerminate (notification: NSNotification) {
    terminateHandler?.call()
    terminateHandler = nil
  }
  
  func didFinishLaunching (notification: NSNotification) {
    launchHandler?.call()
    launchHandler = nil
  }
}