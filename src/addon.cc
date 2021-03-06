#include <napi.h>
#include <iostream>
#include <locale>
#include <codecvt>

#if defined(_WIN32)
#include "file_utilities_win.h"
#include "WinHttpClient/WinHttpClient.h"
#elif defined(__APPLE__)
#include "file_utilities_mac.h"
#include "restclient/restclient.h"
#endif

// #define CPPHTTPLIB_OPENSSL_SUPPORT
// #include "httplib.h"

std::string wstringToUtf8(const std::wstring &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.to_bytes(str);
}

std::wstring utf8ToWstring(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> strCnv;
    return strCnv.from_bytes(str);
}

using namespace Napi;

// A Napi substitute IsInt32()
inline bool OtherIsInt(Napi::Number source) {
    double orig_val = source.DoubleValue();
    double int_val = (double)source.Int32Value();
    if (orig_val == int_val) {
        return true;
    } else {
        return false;
    }
}

#define REQUIRE_ARGUMENT_STRING(i, var)                                                             \
    if (info.Length() <= (i) || !info[i].IsString())                                                \
    {                                                                                               \
        Napi::TypeError::New(env, "Argument " #i " must be a string").ThrowAsJavaScriptException(); \
        return env.Null();                                                                          \
    }                                                                                               \
    std::string var = info[i].As<Napi::String>();

#define REQUIRE_ARGUMENT_INTEGER(i, var)                                                              \
    if (info.Length() <= (i) || !info[i].IsNumber())                                                  \
    {                                                                                                 \
        Napi::TypeError::New(env, "Argument " #i " must be an integer").ThrowAsJavaScriptException(); \
        return env.Null();                                                                            \
    }                                                                                                 \
    int var(info[i].As<Napi::Number>().Int32Value());

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default)                                                    \
    int var;                                                                                          \
    if (info.Length() <= (i))                                                                         \
    {                                                                                                 \
        var = (default);                                                                              \
    }                                                                                                 \
    else if (info[i].IsNumber())                                                                      \
    {                                                                                                 \
        if (OtherIsInt(info[i].As<Number>()))                                                         \
        {                                                                                             \
            var = info[i].As<Napi::Number>().Int32Value();                                            \
        }                                                                                             \
    }                                                                                                 \
    else                                                                                              \
    {                                                                                                 \
        Napi::TypeError::New(env, "Argument " #i " must be an integer").ThrowAsJavaScriptException(); \
        return env.Null();                                                                            \
    }

Napi::Value unsafeShowOpenWith(const Napi::CallbackInfo &info)
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

Napi::Value unsafeOpenEmailLink(const Napi::CallbackInfo &info)
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

Napi::Value deviceId(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    std::string deviceId = Platform::SystemInfo::DeviceId();
    return String::New(env, deviceId);
}

Napi::Value unsafeLaunch(const Napi::CallbackInfo &info)
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

#define OPTIONAL_ARGUMENT_FUNCTION(i, var)                                                                \
    Napi::Function var;                                                                                   \
    if (info.Length() > i && !info[i].IsUndefined())                                                      \
    {                                                                                                     \
        if (!info[i].IsFunction())                                                                        \
        {                                                                                                 \
            Napi::TypeError::New(env, "Argument " #i " must be a function").ThrowAsJavaScriptException(); \
            return env.Null();                                                                            \
        }                                                                                                 \
        var = info[i].As<Napi::Function>();                                                               \
    }

// The Mac OS compiler complains when argv is NULL unless we
// first assign it to a locally defined variable.
#define TRY_CATCH_CALL(context, callback, argc, argv, ...)                 \
    Napi::Value *passed_argv = argv;                                       \
    std::vector<napi_value> args;                                          \
    if ((argc != 0) && (passed_argv != NULL))                              \
    {                                                                      \
        args.assign(passed_argv, passed_argv + argc);                      \
    }                                                                      \
    Napi::Value res = (callback).MakeCallback(Napi::Value(context), args); \
    if (res.IsEmpty())                                                     \
        return env.Null();

Napi::Value httpGet(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    REQUIRE_ARGUMENT_STRING(0, url);
    OPTIONAL_ARGUMENT_INTEGER(1, timeout, 3000);
    // REQUIRE_ARGUMENT_STRING(1, path);
    // OPTIONAL_ARGUMENT_FUNCTION(2, cb);
    // std::cout << host << std::endl;
    // std::cout << path << std::endl;
    // httplib::Client cli(host);
    // const httplib::Result&& res = cli.Get(path.c_str());
    // if (res) {
    //     Napi::Object result = Napi::Object::New(env);
    //     (result).Set("status", res->status);
    //     (result).Set("reason", res->reason);
    //     (result).Set("body", res->body);
    //     return result;
    // } else {
    //     return env.Null();
    // }
#if defined(_WIN32)
    WinHttpClient httpClient(utf8ToWstring(url).c_str());
    httpClient.SetTimeouts(0, timeout, timeout, 0);
    bool ret = httpClient.SendHttpRequest();
    if (ret)
    {
        std::wstring wResp = httpClient.GetResponseContent();
        std::wstring wStatusCode = httpClient.GetResponseStatusCode();
        Napi::Object result = Napi::Object::New(env);
        (result).Set("code", _wtoi(wStatusCode.c_str()));
        (result).Set("body", wstringToUtf8(wResp));
        return result;
    }
    else
    {
        return env.Null();
    }
#elif defined(__APPLE__)
    RestClient::Response res = RestClient::get(url);
    httpClient.SetTimeout(timeout / 1000);
    Napi::Object result = Napi::Object::New(env);
    (result).Set("code", res.code);
    (result).Set("body", res.body);
    return result;
#endif
    /* if (!cb.IsUndefined() && cb.IsFunction()) {
         Napi::Object result = Napi::Object::New(env);
         (result).Set("status", res->status);
         (result).Set("reason", res->reason);
         (result).Set("body", res->body);
         Napi::Value argv[] = { result };
         TRY_CATCH_CALL(info.This(), cb, 1, argv);
     }*/
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "unsafeShowOpenWith"), Napi::Function::New(env, unsafeShowOpenWith));
    exports.Set(Napi::String::New(env, "unsafeOpenEmailLink"), Napi::Function::New(env, unsafeOpenEmailLink));
    exports.Set(Napi::String::New(env, "unsafeLaunch"), Napi::Function::New(env, unsafeLaunch));
    exports.Set(Napi::String::New(env, "deviceId"), Napi::Function::New(env, deviceId));
    exports.Set(Napi::String::New(env, "httpGet"), Napi::Function::New(env, httpGet));
    return exports;
}

NODE_API_MODULE(node_sysutilities, Init)
