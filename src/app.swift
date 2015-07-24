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
  let menu = NSMenu()
  
  func addMenuItem (title: String, representedObject: Callback) {
    let item =
      NSMenuItem(title: title, action: Selector("dispatchMenuItemCallback:"), keyEquivalent: "")
    item.representedObject = representedObject
    menu.addItem(item)
  }
  
  func applicationDidFinishLaunching (notification: NSNotification) {
    statusItem.highlightMode = true
    statusItem.menu = menu
  }
  
  func dispatchMenuItemCallback (sender: AnyObject) {
    (sender.representedObject as? Callback)!.call()
  }
}

public class Facade: NSObject {
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
  
  public func addItem (title: COpaquePointer, cb: COpaquePointer, ctx: COpaquePointer) {
    delegate.addMenuItem(FromCString(title), representedObject: Callback(cb: cb, ctx: ctx))
  }
  
  public func addSeparator () {
    delegate.menu.addItem(NSMenuItem.separatorItem());
  }

  public func run (cb: COpaquePointer, ctx: COpaquePointer) {
    NSNotificationCenter.defaultCenter()
      .addObserver(self,
        selector: Selector("didFinishLaunching:"),
        name: NSApplicationDidFinishLaunchingNotification,
        object: nil)
    
    launchHandler = Callback(cb: cb, ctx: ctx)
    app.setActivationPolicy(NSApplicationActivationPolicy.Accessory)
    app.activateIgnoringOtherApps(true)
    app.delegate = delegate
    app.run()
  }
  
  public func terminate (cb: COpaquePointer, ctx: COpaquePointer) {
    NSNotificationCenter.defaultCenter()
      .addObserver(self,
        selector: Selector("willTerminate:"),
        name: NSApplicationWillTerminateNotification,
        object: nil)
    
    terminateHandler = Callback(cb: cb, ctx: ctx)
    app.terminate(0)
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