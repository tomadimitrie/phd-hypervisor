#include "CRT.hpp"

#include "HypervisorKM/Logging.hpp"

#pragma section(".CRT$XIA", long, read)
#pragma section(".CRT$XIZ", long, read)
#pragma section(".CRT$XCA", long, read)
#pragma section(".CRT$XCZ", long, read)

#pragma comment(linker, "/merge:.CRT=.rdata")

namespace {
    using p_retVoid  = void(__cdecl*)();
    using p_retInt = int(__cdecl*)();
    
    constexpr u32 g_maxAtExit = 64;
    p_retVoid g_atExit[g_maxAtExit]{};
    u32 g_atExitCount = 0;
    
    __declspec(allocate(".CRT$XIA")) p_retInt g_xiA[] = { nullptr };
    __declspec(allocate(".CRT$XIZ")) p_retInt g_xiZ[] = { nullptr };
    __declspec(allocate(".CRT$XCA")) p_retVoid g_xcA[] = { nullptr };
    __declspec(allocate(".CRT$XCZ")) p_retVoid g_xcZ[] = { nullptr };
    
    int InitC(const p_retInt* PfBegin, const p_retInt* PfEnd) {
        int ret = 0;

        while (PfBegin < PfEnd && ret == 0) {
            if (*PfBegin != nullptr) {
                ret = (**PfBegin)();
            }
            ++PfBegin;
        }

        return ret;
    }

    void InitCpp(const p_retVoid* Pfbegin, const p_retVoid* Pfend) {
        while (Pfbegin < Pfend) {
            if (*Pfbegin != nullptr) {
                (**Pfbegin)();
            }
            ++Pfbegin;
        }
    }
}

// ReSharper disable once CppInconsistentNaming
extern "C" int __cdecl atexit(const p_retVoid Func) {
    if (g_atExitCount == g_maxAtExit) {
        NT_ASSERTMSG("Too many globals destructors", false);
        return 1;
    }
    
    g_atExit[g_atExitCount] = Func;
    g_atExitCount += 1;
    
    return 0;
}

namespace Crt {
    Expected<void> Init() {
        if (const int error = InitC(g_xiA, g_xiZ); error != 0) {
            HvFail("C initializers failed with error %d", error);
            return Err(STATUS_UNSUCCESSFUL);
        }
        InitCpp(g_xcA, g_xcZ);
        return Ok();
    }
    
    void Uninit() {
        while (g_atExitCount > 0) {
            g_atExitCount -= 1;
            g_atExit[g_atExitCount]();
        }
    }
}