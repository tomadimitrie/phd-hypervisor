#pragma once

template <typename T>
inline constexpr bool g_isBitmaskEnum = false;

template <typename T>
concept Bitmask = g_isBitmaskEnum<T>;

template <Bitmask TEnum>
constexpr TEnum operator|(TEnum Lhs, TEnum Rhs) noexcept {
	using U = __underlying_type(TEnum);
	return static_cast<TEnum>(static_cast<U>(Lhs) | static_cast<U>(Rhs));
}
