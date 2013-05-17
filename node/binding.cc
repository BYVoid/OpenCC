#include <iostream>
#include <node.h>
#include <v8.h>
#include "../src/opencc.h"

using namespace v8;

char* ToUtf8String(const Local<String>& str) {
  char* utf8 = new char[str->Utf8Length() + 1];
  utf8[str->Utf8Length()] = '\0';
  str->WriteUtf8(utf8);
  return utf8;
}

class OpenccBinding : public node::ObjectWrap {
  struct ConvertRequest {
    OpenccBinding* opencc_instance;
    char* input;
    char* output;
    Persistent<Function> callback;
  };
 public:
  explicit OpenccBinding(const char * config_file) {
    handler_ = opencc_open(config_file);
  }

  virtual ~OpenccBinding() {
    if (handler_ != (opencc_t) -1)
      opencc_close(handler_);
  }

  operator bool() const {
    return handler_ != (opencc_t) -1;
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    OpenccBinding* opencc_instance;

    if (args.Length() >= 1 && args[0]->IsString()) {
      char* config_file = ToUtf8String(args[0]->ToString());
      opencc_instance = new OpenccBinding(config_file);
      delete[] config_file;
    } else {
      const char* config_file = OPENCC_DEFAULT_CONFIG_SIMP_TO_TRAD;
      opencc_instance = new OpenccBinding(config_file);
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
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->opencc_instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
    conv_data->input = ToUtf8String(args[0]->ToString());
    conv_data->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
    uv_work_t* req = new uv_work_t;
    req->data = conv_data;
    uv_queue_work(uv_default_loop(), req, DoConnect, (uv_after_work_cb)AfterConvert);

    return Undefined();
  }
  
  static void DoConnect(uv_work_t* req) {
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    opencc_t opencc_handler = conv_data->opencc_instance->handler_;
    conv_data->output = opencc_convert_utf8(opencc_handler, conv_data->input, (size_t) -1);
  }

  static void AfterConvert(uv_work_t* req) {
    HandleScope scope;
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    Local<String> converted = String::New(conv_data->output);
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      Local<Value>::New(Undefined()),
      Local<Value>::New(converted)
    };
    conv_data->callback->Call(Context::GetCurrent()->Global(), argc, argv);
    conv_data->callback.Dispose();
    delete[] conv_data->input;
    opencc_convert_utf8_free(conv_data->output);
    delete conv_data;
    delete req;
  }

  static Handle<Value> ConvertSync(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    OpenccBinding* opencc_instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
    opencc_t opencc_handler = opencc_instance->handler_;
    char* input = ToUtf8String(args[0]->ToString());
    char* output = opencc_convert_utf8(opencc_handler, input, (size_t) -1);

    Local<String> converted = String::New(output);
    delete[] input;
    opencc_convert_utf8_free(output);
    return scope.Close(converted);
  }

  static Handle<Value> SetConversionMode(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsInt32()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    OpenccBinding* opencc_instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
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
    Local<FunctionTemplate> tpl = FunctionTemplate::New(OpenccBinding::New);
    tpl->SetClassName(String::NewSymbol("Opencc"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    tpl->PrototypeTemplate()->Set(String::NewSymbol("convert"),
        FunctionTemplate::New(Convert)->GetFunction());
    tpl->PrototypeTemplate()->Set(String::NewSymbol("convertSync"),
        FunctionTemplate::New(ConvertSync)->GetFunction());
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
  OpenccBinding::init(target);
}

NODE_MODULE(binding, init);
