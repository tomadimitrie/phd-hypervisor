#include <iostream>
#include <print>
#include <Windows.h>

#include "Common/Driver.hpp"

int main() {
    bool success = false;
    auto device = INVALID_HANDLE_VALUE;

    device = CreateFileW(
        g_deviceNameUm,
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );
    if (device == INVALID_HANDLE_VALUE) {
        std::println("[-] CreateFileW {}", GetLastError());
        if (GetLastError() == ERROR_NOT_SUPPORTED) {
			std::println("[-] VMX not allowed");
        }
        goto end;
    }
    
    std::println("[+] VMX ok");

    success = true;

    system("pause");

end:
    if (device != INVALID_HANDLE_VALUE) {
        CloseHandle(device);
    }

    return !success;
}
