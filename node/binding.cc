#include <iostream>
#include <node.h>
#include <v8.h>
#include <nan.h>

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

  static NAN_METHOD(New) {
    NanScope();
    OpenccBinding* instance;

    try {
      if (args.Length() >= 1 && args[0]->IsString()) {
        string configFile = ToUtf8String(args[0]->ToString());
        instance = new OpenccBinding(configFile);
      } else {
        instance = new OpenccBinding("s2t.json");
      }
    } catch (opencc::Exception& e) {
      NanThrowError(e.what());
      NanReturnUndefined();
    }

    instance->Wrap(args.This());
    NanReturnValue(args.This());
  }

  static NAN_METHOD(Convert) {
    NanScope();
    if (args.Length() < 2 || !args[0]->IsString() || !args[1]->IsFunction()) {
      NanThrowTypeError("Wrong arguments");
      NanReturnUndefined();
    }

    ConvertRequest* conv_data = new ConvertRequest;
    conv_data->instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());
    conv_data->input = ToUtf8String(args[0]->ToString());
    NanAssignPersistent(conv_data->callback, Local<Function>::Cast(args[1]));
    conv_data->ex = Optional<opencc::Exception>::Null();
    uv_work_t* req = new uv_work_t;
    req->data = conv_data;
    uv_queue_work(uv_default_loop(), req, DoConvert, (uv_after_work_cb)AfterConvert);

    NanReturnUndefined();
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
    NanScope();
    ConvertRequest* conv_data = static_cast<ConvertRequest*>(req->data);
    Local<Value> err = NanUndefined();
    Local<String> converted = NanNew<String>(conv_data->output.c_str());
    if (!conv_data->ex.IsNull()) {
      err = NanNew<String>(conv_data->ex.Get().what());
    }
    const unsigned argc = 2;
    Local<Value> argv[argc] = {
      err,
      NanNew(converted)
    };
    NanMakeCallback(NanGetCurrentContext()->Global(), NanNew(conv_data->callback), argc, argv);
    delete conv_data;
    delete req;
  }

  static NAN_METHOD(ConvertSync) {
    NanScope();
    if (args.Length() < 1 || !args[0]->IsString()) {
      NanThrowTypeError("Wrong arguments");
      NanReturnUndefined();
    }

    OpenccBinding* instance = ObjectWrap::Unwrap<OpenccBinding>(args.This());

    string input = ToUtf8String(args[0]->ToString());
    string output;
    try {
      output = instance->Convert(input);
    } catch (opencc::Exception& e) {
      NanThrowError(e.what());
      NanReturnUndefined();
    }

    Local<String> converted = NanNew<String>(output.c_str());
    NanReturnValue(converted);
  }

  static void init(Handle<Object> target) {
    NanScope();
    // Prepare constructor template
    Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(OpenccBinding::New);
    tpl->SetClassName(NanNew<String>("Opencc"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    NODE_SET_PROTOTYPE_METHOD(tpl, "convert", Convert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "convertSync", ConvertSync);
    // Constructor
    target->Set(NanNew<String>("Opencc"), tpl->GetFunction());
  }
};

void init(Handle<Object> target) {
  OpenccBinding::init(target);
}

NODE_MODULE(binding, init);
