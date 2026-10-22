#include "Common/Utils.hpp"

// MSVC references these functions in the scalar destructors, even though it doesn't call them at all.
// We do not need the `new` counterpart.
// We must declare them, and we call a fatal error because if these functions end up being called then something went wrong.

void __cdecl operator delete(  // NOLINT(misc-new-delete-overloads)
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void* Ptr
) noexcept {
    KeBugCheckEx(MANUALLY_INITIATED_CRASH, bit_cast<u64>(Ptr), 0, 0, 0);
}

void __cdecl operator delete( // NOLINT(misc-new-delete-overloads)
    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    void* Ptr, 
    // ReSharper disable once CppParameterMayBeConst
    size_t Size
) noexcept { 
    UNREFERENCED_PARAMETER(Size);
    KeBugCheckEx(MANUALLY_INITIATED_CRASH, bit_cast<u64>(Ptr), 0, 0, 0);
}
