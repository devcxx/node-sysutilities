#include "file_utilities_win.h"
#include <Windows.h>
#include <shlobj_core.h>
#include <algorithm>

namespace Platform {
namespace File {

    void UnsafeOpenEmailLink(const std::wstring& email)
    {
        auto url = std::wstring(L"mailto:") + email;
        OPENASINFO info;
        info.oaifInFlags = OAIF_ALLOW_REGISTRATION
            | OAIF_REGISTER_EXT
            | OAIF_EXEC
#if WINVER >= 0x0602
            | OAIF_FILE_IS_URI
#endif // WINVER >= 0x602
            | OAIF_URL_PROTOCOL;
        info.pcszClass = NULL;
        info.pcszFile = url.c_str();
        SHOpenWithDialog(0, &info);
    }

    void UnsafeShowOpenWith(const std::wstring& filepath)
    {
        OPENASINFO info;
        info.oaifInFlags = OAIF_ALLOW_REGISTRATION | OAIF_REGISTER_EXT | OAIF_EXEC;
        info.pcszClass = NULL;
        info.pcszFile = filepath.c_str();
        SHOpenWithDialog(0, &info);
    }

    void UnsafeLaunch(const std::wstring& filepath)
    {
        ShellExecuteW(0, L"open", filepath.c_str(), 0, 0, SW_SHOWNORMAL);
    }

    void UnsafeShowInFolder(const std::wstring& filepath)
    {
        std::wstring param(filepath);
        // replace all positive slash
        std::wstring positive(L"/");
        std::wstring backslant(L"\\");
        size_t pos = param.find(positive);
        while (pos != std::wstring::npos) {
            param.replace(pos, positive.size(), backslant);
            pos = param.find(positive, pos + backslant.size());
        }
        param = L"/select" + param;
        ShellExecuteW(NULL, L"open", L"explorer.exe", param.c_str(), NULL, SW_SHOWNORMAL);
    }

} // namespace File
} // namespace Platform
