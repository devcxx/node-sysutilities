#pragma once
#include <string>

namespace Platform {
namespace File {
bool UnsafeShowOpenWith(const std::string& url);
void UnsafeOpenEmailLink(const std::string& email);
void UnsafeLaunch(const std::string& filepath);
void UnsafeShowInFolder(const std::string& filepath);
} // namespace File

namespace SystemInfo
{
    std::string DeviceId();
} // namespace SystemInfo

} // namespace Platform
