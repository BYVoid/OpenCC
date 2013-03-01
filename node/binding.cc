#include <unistd.h>
#include <iostream>
#include <node.h>
#include <v8.h>
#include "../src/opencc.h"

using namespace v8;

bool initialized = false;

char* ToUtf8String(const Local<String>& str) {
  char* utf8 = new char[str->Utf8Length() + 1];
  utf8[str->Utf8Length()] = '\0';
  str->WriteUtf8(utf8);
  return utf8;
}

Handle<Value> ModuleInit(const Arguments& args) {
  HandleScope scope;
  if (args.Length() < 1 || !args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Wrong arguments")));
    return scope.Close(Undefined());
  }
  char* dir = ToUtf8String(args[0]->ToString());
  int res = chdir(dir);
  delete [] dir;
  if (res == -1) {
    ThrowException(
        Exception::Error(String::New("Can not change to target dir")));
    return scope.Close(Undefined());
  }
  initialized = true;
  return scope.Close(Undefined());
}

class Opencc : public node::ObjectWrap {
 public:
  explicit Opencc(const char * config_file) {
    handler_ = opencc_open(config_file);
  }

  virtual ~Opencc() {
    if (handler_ != (opencc_t) -1)
      opencc_close(handler_);
  }

  operator bool() const {
    return handler_ != (opencc_t) -1;
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    Opencc* opencc_instance;

    if (args.Length() >= 1 && args[0]->IsString()) {
      char* config_file = ToUtf8String(args[0]->ToString());
      opencc_instance = new Opencc(config_file);
      delete [] config_file;
    } else {
      const char* config_file = OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD;
      opencc_instance = new Opencc(config_file);
    }

    if (!*opencc_instance) {
      ThrowException(Exception::Error(
          String::New("Can not create opencc instance")));
      return scope.Close(Undefined());
    }
    opencc_instance->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> Convert(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    Opencc* opencc_instance = ObjectWrap::Unwrap<Opencc>(args.This());
    opencc_t opencc_handler = opencc_instance->handler_;
    char * input = ToUtf8String(args[0]->ToString());
    char * output = opencc_convert_utf8(opencc_handler, input, (size_t) -1);

    Local<String> converted = String::New(output);
    delete [] input;
    delete [] output;
    return scope.Close(converted);
  }

  static Handle<Value> SetConversionMode(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsInt32()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    Opencc* opencc_instance = ObjectWrap::Unwrap<Opencc>(args.This());
    opencc_t opencc_handler = opencc_instance->handler_;
    int conversion_mode = args[0]->ToInt32()->Value();
    if (conversion_mode < 0 || conversion_mode > 2) {
      ThrowException(Exception::Error(
          String::New("conversion_mode must between 0 and 2")));
      return scope.Close(Undefined());
    }

    opencc_set_conversion_mode(opencc_handler,
                               (opencc_conversion_mode) conversion_mode);
    return scope.Close(Boolean::New(true));
  }

  static void init(Handle<Object> target) {
    // Prepare constructor template
    Local<FunctionTemplate> tpl = FunctionTemplate::New(Opencc::New);
    tpl->SetClassName(String::NewSymbol("Opencc"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("convert"),
        FunctionTemplate::New(Convert)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("setConversionMode"),
        FunctionTemplate::New(SetConversionMode)->GetFunction());
    // Constructor
    Persistent<Function> constructor = Persistent<Function>::New(
        tpl->GetFunction());
    target->Set(String::NewSymbol("Opencc"), constructor);
  }

  opencc_t handler_;
};

void init(Handle<Object> target) {
  NODE_SET_METHOD(target, "init", ModuleInit);
  Opencc::init(target);
}

NODE_MODULE(binding, init);
