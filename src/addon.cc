#include <nan.h>
#include <uv.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <dispatch/dispatch.h>
#include <objc/message.h>
#include <regex>
#include <functional>

using namespace v8;

static int embed_closed = 0;
static bool uv_trip_timer_safety = false;
static uv_sem_t embed_sem;
static uv_thread_t embed_thread;

static id nsapp;

typedef std::function<void()> callback_t;

static void wrap_callback (void* ctx) {
  auto& callback = *reinterpret_cast<callback_t*>(ctx);
  callback();
}

static void uv_event (void* info) {
  int r, timeout, fd;
  struct kevent errors[1];
  
  while (!embed_closed) {
    uv_loop_t* loop = uv_default_loop();
    fd = uv_backend_fd(loop);
    timeout = uv_backend_timeout(loop);
    
    if (timeout < 0)
      timeout = 16;
    else if (timeout > 250)
      timeout = 250;
    else if (timeout == 0 && uv_trip_timer_safety) {
      timeout = 150;
      uv_trip_timer_safety = false;
    }
    
    do {
      struct timespec ts;
      ts.tv_sec = timeout / 1000;
      ts.tv_nsec = (timeout % 1000) * 1000000;
      r = kevent(fd, NULL, 0, errors, 1, &ts);
    } while (r == -1 && errno == EINTR);
    
    dispatch_async(dispatch_get_main_queue(), ^{
      uv_loop_t* loop = uv_default_loop();
      if (uv_run(loop, UV_RUN_NOWAIT) == false &&
          uv_loop_alive(loop) == false) {
        uv_trip_timer_safety = true;
      }
      uv_sem_post(&embed_sem);
    });
    
    uv_sem_wait(&embed_sem);
  }
}

static void handle_launch (NanCallback* cb) {
  embed_closed = 0;
  uv_sem_init(&embed_sem, 0);
  uv_thread_create(&embed_thread, uv_event, NULL);
  cb->Call(0, NULL);
}

static void handle_terminate (NanCallback* cb) {
  embed_closed = 1;
  uv_loop_t* loop = uv_default_loop();
  if (uv_loop_alive(loop)) {
    uv_run(loop, UV_RUN_NOWAIT);
  }
  cb->Call(0, NULL);
}

NAN_METHOD(Run) {
  NanScope();
  
  Class Application(objc_getClass("app.Application"));
  
  nsapp = objc_msgSend((id)Application, sel_getUid("alloc"));
  objc_msgSend(nsapp, sel_getUid("init"));
  
  if (!args[0]->IsNull()) {
    std::string tooltip(*NanUtf8String(args[0]));
    objc_msgSend(nsapp, sel_getUid("setToolTip:"), tooltip.c_str());
  }
  
  if (!args[1]->IsNull()) {
    std::string icon(*NanUtf8String(args[1]));
    std::regex regex("^(.+)\\/(.+)\\.pdf$");
    std::smatch result;
    std::regex_search(icon, result, regex);
    
    size_t len(result.size());
    if (len > 2) {
      std::string path = result.str(1);
      std::string name = result.str(2);
      objc_msgSend(nsapp, sel_getUid("setIcon:name:"), path.c_str(), name.c_str());
    }
  }
  
  NanCallback* onlaunch = new NanCallback(args[2].As<Function>());
  callback_t onlaunch_fn = std::bind(&handle_launch, onlaunch);
  objc_msgSend(nsapp, sel_getUid("setLaunchHandler:ctx:"), wrap_callback, &onlaunch_fn);
  
  NanCallback* onquit = new NanCallback(args[3].As<Function>());
  callback_t onquit_fn = std::bind(&handle_terminate, onquit);
  objc_msgSend(nsapp, sel_getUid("setTerminateHandler:ctx:"), wrap_callback, &onquit_fn);
  
  objc_msgSend(nsapp, sel_getUid("run"));
  
  NanReturnUndefined();
}

void Init(Handle<Object> exports, Handle<Object> module) {
  module->Set(NanNew("exports"), NanNew<FunctionTemplate>(Run)->GetFunction());
}

NODE_MODULE(addon, Init);