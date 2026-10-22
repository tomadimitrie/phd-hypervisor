#pragma once

#ifndef __PLACEMENT_NEW_INLINE
// ReSharper disable once CppInconsistentNaming
#define __PLACEMENT_NEW_INLINE  // NOLINT(clang-diagnostic-reserved-macro-identifier, bugprone-reserved-identifier)
inline void* __cdecl operator new(size_t, void* Where) noexcept {
    return Where;
}

inline void __cdecl operator delete(void*, void*) noexcept {}
#endif // __PLACEMENT_NEW_INLINE

template <typename T, typename... TArgs>
T* ConstructAt(T* Where, const TArgs... Args) {
    return ::new (static_cast<void*>(Where)) T(Args...);
}

template <typename T>
void DestroyAt(T* Object) {
    Object->~T();
}
