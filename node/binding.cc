#include <iostream>
#include <node.h>
#include <v8.h>

#include "Config.hpp"
#include "ConversionChain.hpp"

using namespace v8;
using namespace Opencc;

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
  };

  Config config_;
  ConversionChainPtr conversionChain_;
 public:
  explicit OpenccBinding(const string configFileName) {
    config_.LoadFile(configFileName);
    conversionChain_ = config_.GetConversionChain();
  }

  virtual ~OpenccBinding() {
  }
  
  string Convert(const string& input) {
    return conversionChain_->Convert(input);
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
    } catch (std::exception e) {
      // TODO pass exception message
      ThrowException(Exception::Error(
          String::New("Can not create opencc instance")));
      return scope.Close(Undefined());
    }

    instance->Wrap(args.This());
    return args.This();
  }

  static Handle<Value> Convert(const Arguments& args) {
    HandleScope scope;
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
    conv_data->input = ToUtf8String(args[0]->ToString());
    conv_data->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));
    uv_work_t* req = new uv_work_t;
    req->data = conv_data;
    uv_queue_work(uv_default_loop(), req, DoConvert, (uv_after_work_cb)AfterConvert);

    return Undefined();
  }
  
  static void DoConvert(uv_work_t* req) {
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    OpenccBinding* instance = conv_data->instance;
    conv_data->output = instance->Convert(conv_data->input);
  }

  static void AfterConvert(uv_work_t* req) {
    HandleScope scope;
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    Local<String> converted = String::New(conv_data->output.c_str());
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      Local<Value>::New(Undefined()),
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
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
    }

    OpenccBinding* instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());

    string input = ToUtf8String(args[0]->ToString());
    string output = instance->Convert(input);

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
