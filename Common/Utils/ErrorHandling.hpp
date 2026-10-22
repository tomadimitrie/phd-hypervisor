#pragma once

#include "Templates.hpp"

template <typename T = void>
struct Ok {
    explicit Ok(const T Value) : value(Value) {}
    
    T value;
};

template <>
struct Ok<void> {
    Ok() = default;
};

Ok() -> Ok<>;

struct Err {
    explicit Err(const NTSTATUS Status) : status(Status) {}
    
    // ReSharper disable once CppNonExplicitConversionOperator
    operator NTSTATUS() const {
        return status;
    }
    
    NTSTATUS status;
};

template <typename T>
class [[nodiscard]] Expected {
    static_assert(__is_trivially_copyable(T), "Expected<T> needs trivially copyable fields");
    
public:
    template <typename U>
    requires (!g_isSame<U, void>)
    // ReSharper disable once CppNonExplicitConvertingConstructor
    Expected(const Ok<U> Success) : value(Success.value), status(STATUS_SUCCESS) {}
    
    // ReSharper disable once CppNonExplicitConvertingConstructor
    Expected(const Err Error) : none(), status(Error.status) {
        NT_ASSERT(!NT_SUCCESS(Error.status));
    }
    
    [[nodiscard]] bool IsOk() const {
        return NT_SUCCESS(status);
    }
    
    [[nodiscard]] bool IsErr() const {
        return !IsOk();
    }
    
    [[nodiscard]] NTSTATUS Status() const {
        return status;
    }
    
    [[nodiscard]] const T& Value() const {
        NT_ASSERT(IsOk());
        return value;
    }
    
    [[nodiscard]] T ValueOr(const T Fallback) const {
        return IsOk() ? value : Fallback;
    }
    
    static Expected FromPointer(
        _In_ const T Pointer,
        const NTSTATUS ErrorStatus = STATUS_INSUFFICIENT_RESOURCES
    ) {
        if (Pointer == nullptr) {
            return Err(ErrorStatus);
        }
        return Ok(Pointer);
    }
    
private:
    union {
        // ReSharper disable once CppInconsistentNaming
        u8 none;
        // ReSharper disable once CppInconsistentNaming
        T value;
    };
    NTSTATUS status;
};

template <>
class [[nodiscard]] Expected<void> {
public:
    // ReSharper disable once CppNonExplicitConvertingConstructor
    Expected(const Ok<void>) : status(STATUS_SUCCESS) {}
    
    // ReSharper disable once CppNonExplicitConvertingConstructor
    Expected(const Err Error) : status(Error.status) {
        NT_ASSERT(!NT_SUCCESS(Error.status));
    }
    
    template <typename TStatus>
    requires g_isSame<TStatus, NTSTATUS>
    static Expected FromStatus(const TStatus Status) {
        if (NT_SUCCESS(Status)) {
            return Ok();
        }
        return Err(Status);
    }
    
    [[nodiscard]] bool IsOk() const {
        return NT_SUCCESS(status);
    }
    
    [[nodiscard]] bool IsErr() const {
        return !IsOk();
    }
    
    [[nodiscard]] NTSTATUS Status() const {
        return status;
    }
private:
    NTSTATUS status;
};

namespace ExpectedDetail {
    inline constexpr u32 g_expectFailedBugCheck = 'TPXE'; // EXPT
    
    template <typename T>
    T ValueOrBugCheck(
        const Expected<T>& Result,
        const char* const Expression,
        const char* const File,
        const u32 Line
    ) {
        if (Result.IsErr()) {
            KeBugCheckEx(
                g_expectFailedBugCheck,
                static_cast<u32>(Result.Status()),
                bit_cast<u64>(Expression),
                bit_cast<u64>(File),
                Line
            );
        }
        return Result.Value();
    }
    
    
    inline void ValueOrBugCheck(
        const Expected<void>& Result,
        const char* const Expression,
        const char* const File,
        const u32 Line
    ) {
        if (Result.IsErr()) {
            KeBugCheckEx(
                g_expectFailedBugCheck,
                static_cast<u32>(Result.Status()),
                bit_cast<u64>(Expression),
                bit_cast<u64>(File),
                Line
            );
        }
    }
}

#define EXPECT(...) \
    ExpectedDetail::ValueOrBugCheck((__VA_ARGS__), #__VA_ARGS__, __FILE__, __LINE__)


#define TRY(...) \
    do { \
        const Expected<void> tryResult_ = (__VA_ARGS__); \
        if (tryResult_.IsErr()) { \
            HvFuncFail(#__VA_ARGS__, tryResult_.Status()); \
            return Err(tryResult_.Status()); \
        } \
    } while (0)

#define TRY_ASSIGN_IMPL(Temp, Declaration, ...) \
    const auto Temp = (__VA_ARGS__); \
    if (Temp.IsErr()) { \
        HvFuncFail(#__VA_ARGS__, Temp.Status()); \
        return Err(Temp.Status()); \
    } \
    Declaration = Temp.Value()

#define TRY_ASSIGN(Declaration, ...) \
    TRY_ASSIGN_IMPL(CONCAT(tryResult_, __COUNTER__), Declaration, __VA_ARGS__)


class Rollback {
public:
    Rollback() = default;
    Rollback(const Rollback&) = delete;
    Rollback& operator=(const Rollback&) = delete;
    
    [[nodiscard]] bool IsCommitted() const {
        return committed;
    }
    
    template <typename... TArgs>
    auto Ok(const TArgs... Args) {
        Commit();
        return ::Ok(Args...);
    }
private:
    void Commit() {
        committed = true; 
    }
    
    bool committed = false;
};

template <typename TFunc>
class ScopeExit {
public:
    explicit ScopeExit(const TFunc Func) : func(Func) {}
    
    ~ScopeExit() {
        func();
    }
    
    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    
private:
    TFunc func;
};

template <typename TFunc>
class OnRollback {
public:
    explicit OnRollback(const Rollback& Owner, const TFunc Func) : owner(Owner), func(Func) {}
    
    ~OnRollback() {
        if (!owner.IsCommitted()) {
            func();
        }
    }
    
    OnRollback(const OnRollback&) = delete;
    OnRollback& operator=(const OnRollback&) = delete;
    
private:
    const Rollback& owner;
    TFunc func;
};

namespace DeferDetail {
    struct DeferMaker {
        template <typename TFunc>
        ScopeExit<TFunc> operator+(const TFunc Func) const {
            return ScopeExit<TFunc>(Func);
        }
    };
    
    struct ErrDeferMaker {
        const Rollback& owner;
        
        template <typename TFunc>
        OnRollback<TFunc> operator+(const TFunc Func) const {
            return OnRollback<TFunc>(owner, Func);
        }
    };
}

#define DEFER \
    const auto CONCAT(defer_, __COUNTER__) = DeferDetail::DeferMaker{} + [&]()

#define ERRDEFER(Owner) \
    const auto CONCAT(errdefer_, __COUNTER__) = DeferDetail::ErrDeferMaker{Owner} + [&]()

struct UseRollbackOkInstead {
    template <typename... TArgs>
    void operator()(TArgs...) const = delete;
};

#define ROLLBACK_DEFINE(Name) \
    Rollback Name; \
    [[maybe_unused]] constexpr UseRollbackOkInstead Ok{}