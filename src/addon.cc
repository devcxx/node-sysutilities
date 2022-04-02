#include <napi.h>
#include <iostream>
#include <locale>
#include <codecvt>

#if defined(_WIN32)
#include "file_utilities_win.h"
#elif defined(__APPLE__)
#include "file_utilities_mac.h"
#endif

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

std::string wstringToUtf8(const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.to_bytes(str);
}

std::wstring utf8ToWstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.from_bytes(str);
}

using namespace Napi;

#define REQUIRE_ARGUMENT_STRING(i, var)                                                             \
    if (info.Length() <= (i) || !info[i].IsString()) {                                              \
        Napi::TypeError::New(env, "Argument " #i " must be a string").ThrowAsJavaScriptException(); \
        return env.Null();                                                                          \
    }                                                                                               \
    std::string var = info[i].As<Napi::String>();

Napi::Value unsafeShowOpenWith(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, path);
#if defined(_WIN32)
    Platform::File::UnsafeShowOpenWith(utf8ToWstring(path));
#elif defined(__APPLE__)
    Platform::File::UnsafeShowOpenWith(path);
#endif
    return env.Null();
}

Napi::Value unsafeOpenEmailLink(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, path);
#if defined(_WIN32)
    Platform::File::UnsafeOpenEmailLink(utf8ToWstring(path));
#elif defined(__APPLE__)
    Platform::File::UnsafeOpenEmailLink(path);
#endif
    return env.Null();
}

Napi::Value deviceId(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    std::string deviceId = Platform::SystemInfo::DeviceId();
    return String::New(env, deviceId);
}

Napi::Value unsafeLaunch(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, path);
#if defined(_WIN32)
    Platform::File::UnsafeLaunch(utf8ToWstring(path));
#elif defined(__APPLE__)
    Platform::File::UnsafeLaunch(path);
#endif
    return env.Null();
}

#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                     \
    Napi::Function var;                                                        \
    if (info.Length() > i && !info[i].IsUndefined()) {                         \
        if (!info[i].IsFunction()) {                                           \
            Napi::TypeError::New(env, "Argument " #i " must be a function").ThrowAsJavaScriptException(); \
            return env.Null(); \
        }                                                                      \
        var = info[i].As<Napi::Function>();                                    \
    }

// The Mac OS compiler complains when argv is NULL unless we
// first assign it to a locally defined variable.
#define TRY_CATCH_CALL(context, callback, argc, argv, ...)                     \
    Napi::Value* passed_argv = argv;\
    std::vector<napi_value> args;\
    if ((argc != 0) && (passed_argv != NULL)) {\
      args.assign(passed_argv, passed_argv + argc);\
    }\
    Napi::Value res = (callback).MakeCallback(Napi::Value(context), args);     \
    if (res.IsEmpty()) return env.Null();

Napi::Value httpGet(const Napi::CallbackInfo& info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, host);
    REQUIRE_ARGUMENT_STRING(1, path);
    OPTIONAL_ARGUMENT_FUNCTION(2, cb);
    std::cout << host << std::endl;
    std::cout << path << std::endl;
    httplib::Client cli(host);
    auto& res = cli.Get(path.c_str());

   /* if (!cb.IsUndefined() && cb.IsFunction()) {
        Napi::Object result = Napi::Object::New(env);
        (result).Set("status", res->status);
        (result).Set("reason", res->reason);
        (result).Set("body", res->body);
        Napi::Value argv[] = { result };
        TRY_CATCH_CALL(info.This(), cb, 1, argv);
    }*/
    Napi::Object result = Napi::Object::New(env);
    (result).Set("status", res->status);
    (result).Set("reason", res->reason);
    (result).Set("body", res->body);
    return result;
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "unsafeShowOpenWith"), Napi::Function::New(env, unsafeShowOpenWith));
    exports.Set(Napi::String::New(env, "unsafeOpenEmailLink"), Napi::Function::New(env, unsafeOpenEmailLink));
    exports.Set(Napi::String::New(env, "unsafeLaunch"), Napi::Function::New(env, unsafeLaunch));
    exports.Set(Napi::String::New(env, "deviceId"), Napi::Function::New(env, deviceId));
    exports.Set(Napi::String::New(env, "httpGet"), Napi::Function::New(env, httpGet));
  return exports;
}

NODE_API_MODULE(node_sysutilities, Init)
