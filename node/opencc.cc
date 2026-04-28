#include <napi.h>
#include <string>

#include "src/Config.hpp"
#include "src/Converter.hpp"
#include "src/DictConverter.hpp"
#include "src/Exception.hpp"

using namespace opencc;

std::string ToUtf8String(const Napi::Value& val) {
  return val.As<Napi::String>().Utf8Value();
}

class OpenccBinding : public Napi::ObjectWrap<OpenccBinding> {
  class ConvertWorker : public Napi::AsyncWorker {
    OpenccBinding* instance_;
    std::string input_;
    std::string output_;

  public:
    ConvertWorker(OpenccBinding* instance, const std::string& input,
                  const Napi::Function& callback)
        : Napi::AsyncWorker(callback, "opencc:convert-async-cb"),
          instance_(instance), input_(input) {
      instance_->Ref();
    }

    ~ConvertWorker() override {
      instance_->Unref();
    }

    void Execute() override {
      try {
        output_ = instance_->Convert(input_);
      } catch (opencc::Exception& e) {
        SetError(e.what());
      }
    }

    void OnOK() override {
      Callback().Call({Env().Undefined(), Napi::String::New(Env(), output_)});
    }

    void OnError(const Napi::Error& e) override {
      Callback().Call({Napi::String::New(Env(), e.Message()),
                       Napi::String::New(Env(), "")});
    }
  };

  Config config_;
  ConverterPtr converter_;

public:
  explicit OpenccBinding(const Napi::CallbackInfo& info)
      : Napi::ObjectWrap<OpenccBinding>(info), config_(), converter_() {
    Napi::Env env = info.Env();
    std::string configFile = "s2t.json";
    if (info.Length() >= 1) {
      if (!info[0].IsString()) {
        Napi::TypeError::New(env, "Wrong arguments")
            .ThrowAsJavaScriptException();
        return;
      }
      configFile = ToUtf8String(info[0]);
    }

    try {
      converter_ = config_.NewFromFile(configFile);
    } catch (opencc::Exception& e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    }
  }

  ~OpenccBinding() override {}

  std::string Convert(const std::string& input) {
    return converter_->Convert(input);
  }

  static Napi::Value Version(const Napi::CallbackInfo& info) {
    return Napi::String::New(info.Env(), VERSION);
  }

  Napi::Value Convert(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsFunction()) {
      Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    ConvertWorker* worker =
        new ConvertWorker(this, ToUtf8String(info[0]),
                          info[1].As<Napi::Function>());
    worker->Queue();
    return env.Undefined();
  }

  Napi::Value ConvertSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1 || !info[0].IsString()) {
      Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
      return env.Undefined();
    }

    const std::string input = ToUtf8String(info[0]);
    std::string output;
    try {
      output = Convert(input);
    } catch (opencc::Exception& e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
      return env.Undefined();
    }

    return Napi::String::New(env, output);
  }

  static Napi::Value GenerateDict(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 4 || !info[0].IsString() || !info[1].IsString() ||
        !info[2].IsString() || !info[3].IsString()) {
      Napi::TypeError::New(env, "Wrong arguments").ThrowAsJavaScriptException();
      return env.Undefined();
    }
    const std::string inputFileName = ToUtf8String(info[0]);
    const std::string outputFileName = ToUtf8String(info[1]);
    const std::string formatFrom = ToUtf8String(info[2]);
    const std::string formatTo = ToUtf8String(info[3]);
    try {
      opencc::ConvertDictionary(inputFileName, outputFileName, formatFrom,
                                formatTo);
    } catch (opencc::Exception& e) {
      Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
    }
    return env.Undefined();
  }

  static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    Napi::Function cons = DefineClass(
        env, "Opencc",
        {
            StaticMethod("version", &OpenccBinding::Version),
            StaticMethod("generateDict", &OpenccBinding::GenerateDict),
            InstanceMethod("convert", &OpenccBinding::Convert),
            InstanceMethod("convertSync", &OpenccBinding::ConvertSync),
        });
    exports.Set("Opencc", cons);
    return exports;
  }
};

Napi::Object InitAll(Napi::Env env, Napi::Object exports) {
  return OpenccBinding::Init(env, exports);
}

NODE_API_MODULE(opencc, InitAll);
