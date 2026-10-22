#pragma once

#ifdef _KERNEL_MODE
# include <ntddk.h>
#else
# include <Windows.h>
#endif

[[nodiscard]] constexpr u64 PageAlignDown(const u64 Address) {
    return Address & ~(PAGE_SIZE - 1);
}

template <typename... TArgs>
SIZE_T CopyPackedMemory(
    _Out_ unsigned char* Dest,
    _In_ const TArgs&... Args
) {
    static_assert((__is_trivially_copyable(TArgs) && ...));

    size_t offset = 0;

    auto copyOne = [&](const auto& Arg) {
        memcpy(&Dest[offset], &Arg, sizeof(Arg));

        offset += sizeof(Arg);
    };

    (copyOne(Args), ...);

    return offset;
}
