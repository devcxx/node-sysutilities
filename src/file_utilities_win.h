#pragma once
#include <string>

namespace Platform {
namespace File {
    void UnsafeOpenEmailLink(const std::wstring& email);
    void UnsafeShowOpenWith(const std::wstring& filepath);
    void UnsafeLaunch(const std::wstring& filepath);
    void UnsafeShowInFolder(const std::wstring& filepath);
} // namespace File

namespace SystemInfo
{
    std::string DeviceId();
} // namespace SystemInfo

} // namespace Platform
