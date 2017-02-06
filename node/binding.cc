#include <iostream>
#include <nan.h>

#include "Config.hpp"
#include "Converter.hpp"
#include "DictConverter.hpp"

// For faster build
#include "BinaryDict.cpp"
#include "Config.cpp"
#include "Conversion.cpp"
#include "ConversionChain.cpp"
#include "Converter.cpp"
#include "DartsDict.cpp"
#include "Dict.cpp"
#include "DictConverter.cpp"
#include "DictEntry.cpp"
#include "DictGroup.cpp"
#include "MaxMatchSegmentation.cpp"
#include "Segmentation.cpp"
#include "TextDict.cpp"
#include "UTF8Util.cpp"

using namespace opencc;

string ToUtf8String(const v8::Local<v8::Value>& val) {
  Nan::Utf8String utf8(val);
  return string(*utf8);
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

  static NAN_METHOD(Version) {
    info.GetReturnValue().Set(Nan::New<v8::String>(VERSION).ToLocalChecked());
  }

  static NAN_METHOD(New) {
    OpenccBinding* instance;

    try {
      if (info.Length() >= 1 && info[0]->IsString()) {
        const string configFile = ToUtf8String(info[0]);
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
    if (info.Length() < 2 || !info[0]->IsString() || !info[1]->IsFunction()) {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->instance = Nan::ObjectWrap::Unwrap<OpenccBinding>(info.This());
    conv_data->input = ToUtf8String(info[0]);
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
    v8::Local<v8::Value> err = Nan::Undefined();
    v8::Local<v8::String> converted = Nan::New(conv_data->output.c_str()).ToLocalChecked();
    if (!conv_data->ex.IsNull()) {
      err = Nan::New(conv_data->ex.Get().what()).ToLocalChecked();
    }
    const unsigned argc = 2;
    v8::Local<v8::Value> argv[argc] = {
      err,
      converted
    };
    conv_data->callback->Call(argc, argv);
    delete conv_data;
    delete req;
  }

  static NAN_METHOD(ConvertSync) {
    if (info.Length() < 1 || !info[0]->IsString()) {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }

    OpenccBinding* instance = Nan::ObjectWrap::Unwrap<OpenccBinding>(info.This());

    const string input = ToUtf8String(info[0]);
    string output;
    try {
      output = instance->Convert(input);
    } catch (opencc::Exception& e) {
      Nan::ThrowError(e.what());
      return;
    }

    v8::Local<v8::String> converted = Nan::New(output.c_str()).ToLocalChecked();
    info.GetReturnValue().Set(converted);
  }

  static NAN_METHOD(GenerateDict) {
    if (info.Length() < 4 || !info[0]->IsString() || !info[1]->IsString()
       || !info[2]->IsString() || !info[3]->IsString()) {
      Nan::ThrowTypeError("Wrong arguments");
      return;
    }
    const string inputFileName = ToUtf8String(info[0]);
    const string outputFileName = ToUtf8String(info[1]);
    const string formatFrom = ToUtf8String(info[2]);
    const string formatTo = ToUtf8String(info[3]);
    try {
      opencc::ConvertDictionary(inputFileName, outputFileName, formatFrom, formatTo);
    } catch (opencc::Exception& e) {
      Nan::ThrowError(e.what());
    }
  }

  static NAN_MODULE_INIT(Init) {
    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(OpenccBinding::New);
    tpl->SetClassName(Nan::New("Opencc").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Methods
    Nan::SetMethod(tpl, "version", Version);
    Nan::SetMethod(tpl, "generateDict", GenerateDict);
    // Prototype
    Nan::SetPrototypeMethod(tpl, "convert", Convert);
    Nan::SetPrototypeMethod(tpl, "convertSync", ConvertSync);
    // Constructor
    v8::Local<v8::Function> cons = Nan::GetFunction(tpl).ToLocalChecked();
    Nan::Set(target, Nan::New("Opencc").ToLocalChecked(), cons);
  }
};

NODE_MODULE(binding, OpenccBinding::Init);
