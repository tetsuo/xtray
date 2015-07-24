#include <nan.h>
#include <uv.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <dispatch/dispatch.h>
#include <objc/message.h>
#include <functional>
#include <regex>
#include <vector>

using v8::Local;
using v8::Value;
using v8::Handle;
using v8::Object;
using v8::ObjectTemplate;
using v8::Function;
using v8::FunctionTemplate;

typedef std::function<void()> callback_t;

static int embed_closed = 0;
static uv_sem_t embed_sem;
static uv_thread_t embed_thread;

static void WrapCallback (void *ctx) {
  auto& callback = *reinterpret_cast<callback_t*>(ctx);
  callback();
}

static const char* ToCString (const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

class Bant : public node::ObjectWrap {
public:
  Bant();
  
  static void Init(v8::Handle<v8::Object> target);
  static v8::Handle<v8::Value> NewInstance ();
  
  static NAN_METHOD(New);
  static NAN_METHOD(SetToolTip);
  static NAN_METHOD(SetIcon);
  static NAN_METHOD(AddItem);
  static NAN_METHOD(AddSeparator);
  static NAN_METHOD(Run);
  static NAN_METHOD(Terminate);

private:
  void DidFinishLaunching (NanCallback* cb);
  void WillTerminate (NanCallback* cb);
  void OnMenuItemClick (NanCallback* cb);
  id facade;
  std::vector<callback_t> cbs;
};

static v8::Persistent<v8::FunctionTemplate> bant_ctor;

Class Facade(objc_getClass("app.Facade"));

Bant::Bant() {
  facade = objc_msgSend((id)Facade, sel_getUid("alloc"));
  objc_msgSend(facade, sel_getUid("init"));
}

NAN_METHOD(CreateNew) {
  NanScope();
  NanReturnValue(Bant::NewInstance());
}

void Bant::Init(Handle<Object> target) {
  Local<FunctionTemplate> tpl(NanNew<FunctionTemplate>(Bant::New));
  
  NanAssignPersistent(bant_ctor, tpl);
  
  tpl->SetClassName(NanNew<v8::String>("Bant"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  
  NODE_SET_PROTOTYPE_METHOD(tpl, "tooltip", Bant::SetToolTip);
  NODE_SET_PROTOTYPE_METHOD(tpl, "icon", Bant::SetIcon);
  NODE_SET_PROTOTYPE_METHOD(tpl, "addItem", Bant::AddItem);
  NODE_SET_PROTOTYPE_METHOD(tpl, "addSeparator", Bant::AddSeparator);
  NODE_SET_PROTOTYPE_METHOD(tpl, "run", Bant::Run);
  NODE_SET_PROTOTYPE_METHOD(tpl, "terminate", Bant::Terminate);

  Local<Function> create =
    NanNew<FunctionTemplate>(CreateNew)->GetFunction();
  target->Set(NanNew<v8::String>("create"), create);
}

Handle<Value> Bant::NewInstance () {
  Local<FunctionTemplate> handle(NanNew(bant_ctor));
  Local<Object> inst(handle->GetFunction()->NewInstance(0, NULL));
  return inst;
}

static bool uv_trip_timer_safety = false;
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

void Bant::DidFinishLaunching(NanCallback* cb) {
  embed_closed = 0;
  
  uv_sem_init(&embed_sem, 0);
  uv_thread_create(&embed_thread, uv_event, NULL);
  
  cb->Call(0, NULL);
}

void Bant::WillTerminate(NanCallback* cb) {
  embed_closed = 1;
  
  uv_loop_t* loop = uv_default_loop();
  if (uv_loop_alive(loop))
    uv_run(loop, UV_RUN_NOWAIT);
  
  cb->Call(0, NULL);
}

void Bant::OnMenuItemClick(NanCallback* cb) {
  cb->Call(0, NULL);
}

NAN_METHOD(Bant::Run) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  
  NanCallback* cb = new NanCallback(args[0].As<Function>());
  callback_t next = std::bind(&Bant::DidFinishLaunching, self, cb);
  objc_msgSend(self->facade, sel_getUid("run:ctx:"), WrapCallback, &next);
  
  NanReturnUndefined();
}

NAN_METHOD(Bant::Terminate) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  
  NanCallback* cb = new NanCallback(args[0].As<Function>());
  callback_t next = std::bind(&Bant::WillTerminate, self, cb);
  objc_msgSend(self->facade, sel_getUid("terminate:ctx:"), WrapCallback, &next);
  
  NanReturnUndefined();
}

NAN_METHOD(Bant::SetToolTip) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  
  v8::String::Utf8Value str(args[0]);
  const char* cstr = ToCString(str);
  objc_msgSend(self->facade, sel_getUid("setToolTip:"), cstr);
  
  NanReturnValue(args.This());
}

NAN_METHOD(Bant::SetIcon) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  
  std::string str(*NanUtf8String(args[0]));
  std::regex regex("^(.+)\\/(.+)\\.pdf$");
  std::smatch result;
  std::regex_search(str, result, regex);
  
  size_t len(result.size());
  if (len > 2) {
    std::string path = result.str(1);
    std::string name = result.str(2);
    objc_msgSend(self->facade, sel_getUid("setIcon:name:"),
                 path.c_str(), name.c_str());
  }
  
  NanReturnValue(args.This());
}

NAN_METHOD(Bant::AddItem) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  
  v8::String::Utf8Value str(args[0]);
  const char* cstr = ToCString(str);
  NanCallback* cb = new NanCallback(args[1].As<Function>());
  self->cbs.push_back(std::bind(&Bant::OnMenuItemClick, self, cb));
  objc_msgSend(self->facade, sel_getUid("addItem:cb:ctx:"), cstr, WrapCallback, &(self->cbs.back()));
  
  NanReturnValue(args.This());
}

NAN_METHOD(Bant::AddSeparator) {
  NanScope();
  Bant* self(node::ObjectWrap::Unwrap<Bant>(args.This()));
  objc_msgSend(self->facade, sel_getUid("addSeparator"));
  
  NanReturnValue(args.This());
}

NAN_METHOD(Bant::New) {
  NanScope();
  Bant *self = new Bant();
  self->Wrap(args.This());
  NanReturnValue(args.This());
}

NODE_MODULE(bant, Bant::Init);