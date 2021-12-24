#include "file_utilities_win.h"
#include <Windows.h>
#include <shlobj_core.h>
#include <algorithm>
#include "registry_win.h"
#include <iostream>

#include "wmi/wmi.hpp"
#include "wmi/wmiclasses.hpp"
#include "wmi/unistd.h"

#pragma comment(lib, "wbemuuid.lib")

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

namespace SystemInfo {

    const char kMicrosoftCryptographyRegKey[] = "SOFTWARE\\Microsoft\\Cryptography";
    const char kMicrosoftCryptographyMachineGuidRegKey[] = "MachineGuid";

    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process;
    BOOL IsWow64()
    {
        BOOL bIsWow64 = FALSE;
        //IsWow64Process is not available on all supported versions of Windows.
        //Use GetModuleHandle to get a handle to the DLL that contains the function
        //and GetProcAddress to get a pointer to the function if available.
        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

        if (NULL != fnIsWow64Process) {
            if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
                //handle error
            }
        }
        return bIsWow64;
    }
    std::string DeviceId()
    {
        std::string machineGuid;
        RegKey key;
        // !!!NOTE 32bit application MUST add
        ACCESS_MASK access = KEY_READ;
        if (IsWow64())
            access |= KEY_WOW64_64KEY;
        LONG sts = key.Open(HKEY_LOCAL_MACHINE, kMicrosoftCryptographyRegKey, access);
        if (sts == ERROR_SUCCESS) {
            sts = key.ReadValue(kMicrosoftCryptographyMachineGuidRegKey, &machineGuid);
        }
        else {
            try {
                Wmi::Win32_ComputerSystemProduct product = Wmi::retrieveWmi<Wmi::Win32_ComputerSystemProduct>();
                machineGuid = product.UUID;
            } catch (const Wmi::WmiException& ex) {
                std::cout << "Wmi error: " << ex.errorMessage << ", Code: " << ex.hexErrorCode();
            }
        }

        return machineGuid;
    }
}
} // namespace Platform
