#include <napi.h>
#include <iostream>
#include <locale>
#include <codecvt>

#if defined(_WIN32)
#include "file_utilities_win.h"
#elif defined(__APPLE__)
#include "file_utilities_mac.h"
#endif

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

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "unsafeShowOpenWith"), Napi::Function::New(env, unsafeShowOpenWith));
    exports.Set(Napi::String::New(env, "unsafeOpenEmailLink"), Napi::Function::New(env, unsafeOpenEmailLink));
    exports.Set(Napi::String::New(env, "unsafeLaunch"), Napi::Function::New(env, unsafeLaunch));
    exports.Set(Napi::String::New(env, "deviceId"), Napi::Function::New(env, deviceId));
  return exports;
}

NODE_API_MODULE(node_sysutilities, Init)
