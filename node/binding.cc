#include <iostream>
#include <node.h>
#include <v8.h>
#include <nan.h>

#include "Config.hpp"
#include "Converter.hpp"

using namespace v8;
using namespace opencc;
using namespace Nan;

string ToUtf8String(const Local<String>& str) {
  v8::String::Utf8Value utf8(str);
  return std::string(*utf8);
}

class OpenccBinding : public Nan::ObjectWrap {
  struct ConvertRequest {
    OpenccBinding* instance;
    string input;
    string output;
    Nan::Callback *callback;
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

  static NAN_METHOD(New) {
    Nan::HandleScope scope;
    OpenccBinding* instance;

    try {
      if (info.Length() >= 1 && info[0]->IsString()) {
        string configFile = ToUtf8String(info[0]->ToString());
        instance = new OpenccBinding(configFile);
      } else {
        instance = new OpenccBinding("s2t.json");
      }
    } catch (opencc::Exception& e) {
      Nan::ThrowError(e.what());
      return;
    }

    instance->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  }

  static NAN_METHOD(Convert) {
    Nan::HandleScope scope;
    if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsFunction()) {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->instance = Nan::ObjectWrap::Unwrap<OpenccBinding>(info.This());
    conv_data->input = ToUtf8String(info[0]->ToString());
    conv_data->callback = new Nan::Callback(info[1].As<v8::Function>());
    conv_data->ex = Optional<opencc::Exception>::Null();
    uv_work_t* req = new uv_work_t;
    req->data = conv_data;
    uv_queue_work(uv_default_loop(), req, DoConvert, (uv_after_work_cb)AfterConvert);

    return;
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
    Nan::HandleScope scope;
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    Local<Value> err = Nan::Undefined();
    Local<String> converted = Nan::New<v8::String>(conv_data->output.c_str()).ToLocalChecked();
    if (!conv_data->ex.IsNull()) {
      err = Nan::New<v8::String>(conv_data->ex.Get().what()).ToLocalChecked();
    }
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      err,
      converted
    };
    conv_data->callback->Call(argc, argv);
    delete conv_data;
    delete req;
  }

  static NAN_METHOD(ConvertSync) {
    Nan::HandleScope scope;
    if (info.Length() < 1 || !info[0]->IsString()) {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }

    OpenccBinding* instance = Nan::ObjectWrap::Unwrap<OpenccBinding>(info.This());

    string input = ToUtf8String(info[0]->ToString());
    string output;
    try {
      output = instance->Convert(input);
    } catch (opencc::Exception& e) {
      Nan::ThrowError(e.what());
      return;
    }

    Local<String> converted = Nan::New<v8::String>(output.c_str()).ToLocalChecked();
    info.GetReturnValue().Set(converted);
  }

  static NAN_MODULE_INIT(Init) {
    Nan::HandleScope scope;
    // Prepare constructor template
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(OpenccBinding::New);
    tpl->SetClassName(Nan::New<v8::String>("Opencc").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    Nan::SetPrototypeMethod(tpl, "convert", Convert);
    Nan::SetPrototypeMethod(tpl, "convertSync", ConvertSync);
    // Constructor
    Nan::Set(target, Nan::New<v8::String>("Opencc").ToLocalChecked(), tpl->GetFunction());
  }
};

NODE_MODULE(binding, OpenccBinding::Init);
