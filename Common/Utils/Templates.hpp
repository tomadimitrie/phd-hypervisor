#pragma once

#include "Numeric.hpp"

template <typename TA, typename TB> 
inline constexpr bool g_isSame = false;

template <typename TA>
inline constexpr bool g_isSame<TA, TA> = true;

template <typename TTo, typename TFrom>
// ReSharper disable once CppInconsistentNaming
constexpr TTo bit_cast(const TFrom& From) {
	static_assert(sizeof(TTo) == sizeof(TFrom), "bit_cast requires equal size of both operands");

	return __builtin_bit_cast(TTo, From);
}

template <typename TFunc, typename... TArgs>
void Apply(TFunc&& Func, TArgs&&... Args) {
	(Func(Args), ...);
}

template <typename T, T...>
struct IntSeq {};

template <typename TFunc, typename... TArgs>
void ApplyIndexed(TFunc&& Func, TArgs&&... Args) {
	[&]<u64... TIs>(IntSeq<u64, TIs...>, auto&&... InnerArgs) {
		(Func(TIs, static_cast<decltype(InnerArgs)&&>(InnerArgs)), ...);
	}(
		__make_integer_seq<IntSeq, u64, sizeof...(Args)>{},
		static_cast<TArgs&&>(Args)...
	);
}

#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define WIDE_STRINGIFY_IMPL(x) L ## #x
#define WIDE_STRINGIFY(x) WIDE_STRINGIFY_IMPL(x)

#define CONCAT2(a, b) a##b
#define CONCAT(a, b) CONCAT2(a, b)