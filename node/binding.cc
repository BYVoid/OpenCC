#include <iostream>
#include <node.h>
#include <v8.h>

#include "Config.hpp"
#include "Converter.hpp"

using namespace v8;
using namespace opencc;

string ToUtf8String(const Local<String>& str) {
  v8::String::Utf8Value utf8(str);
  return std::string(*utf8);
}

class OpenccBinding : public node::ObjectWrap {
  struct ConvertRequest {
    OpenccBinding* instance;
    string input;
    string output;
    Persistent<Function> callback;
    Optional<opencc::Exception> ex;

    ConvertRequest()
        : instance(nullptr), ex(Optional<opencc::Exception>::Null()) {
    }
  };

  Config config_;
  const ConverterPtr converter_;
 public:
  explicit OpenccBinding(const string configFileName)
    : config_(),
      converter_(config_.NewFromFile(configFileName)) {}

  virtual ~OpenccBinding() {
  }

  string Convert(const string& input) {
    return converter_->Convert(input);
  }

  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    OpenccBinding* instance;

    try {
      if (args.Length() >= 1 && args[0]->IsString()) {
        string configFile = ToUtf8String(args[0]->ToString());
        instance = new OpenccBinding(configFile);
      } else {
        instance = new OpenccBinding("s2t.json");
      }
    } catch (opencc::Exception& e) {
      ThrowException(v8::Exception::Error(
          String::New(e.what())));
      return scope.Close(Undefined());
    }

    instance->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> Convert(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
      ThrowException(v8::Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
    conv_data->input = ToUtf8String(args[0]->ToString());
    conv_data->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
    conv_data->ex = Optional<opencc::Exception>::Null();
    uv_work_t* req = new uv_work_t;
    req->data = conv_data;
    uv_queue_work(uv_default_loop(), req, DoConvert, (uv_after_work_cb)AfterConvert);

    return Undefined();
  }

  static void DoConvert(uv_work_t* req) {
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    OpenccBinding* instance = conv_data->instance;
    try {
      conv_data->output = instance->Convert(conv_data->input);
    } catch (opencc::Exception& e) {
      conv_data->ex = Optional<opencc::Exception>(e);
    }
  }

  static void AfterConvert(uv_work_t* req) {
    HandleScope scope;
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    Local<Value> err = Local<Value>::New(Undefined());
    Local<String> converted = String::New(conv_data->output.c_str());
    if (!conv_data->ex.IsNull()) {
      err = String::New(conv_data->ex.Get().what());
    }
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      err,
      Local<Value>::New(converted)
    };
    conv_data->callback->Call(Context::GetCurrent()->Global(), argc, argv);
    conv_data->callback.Dispose();
    delete conv_data;
    delete req;
  }

  static Handle<Value> ConvertSync(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsString()) {
      ThrowException(v8::Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    OpenccBinding* instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());

    string input = ToUtf8String(args[0]->ToString());
    string output;
    try {
      output = instance->Convert(input);
    } catch (opencc::Exception& e) {
      ThrowException(v8::Exception::Error(
          String::New(e.what())));
      return scope.Close(Undefined());
    }

    Local<String> converted = String::New(output.c_str());
    return scope.Close(converted);
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
    // Constructor
    Persistent<Function> constructor = Persistent<Function>::New(
        tpl->GetFunction());
    target->Set(String::NewSymbol("Opencc"), constructor);
  }
};

void init(Handle<Object> target) {
  OpenccBinding::init(target);
}

NODE_MODULE(binding, init);
