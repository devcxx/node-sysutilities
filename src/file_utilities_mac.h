#pragma once
#include <string>

namespace Platform {
namespace File {

bool UnsafeShowOpenWith(const std::string& url);
void UnsafeOpenEmailLink(const std::string& email);
void UnsafeLaunch(const std::string& filepath);

} // namespace File
} // namespace Platform
