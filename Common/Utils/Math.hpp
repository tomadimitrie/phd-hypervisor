#pragma once

inline
int
MathPower(
	int Base,
	u32 Exponent
) {
	int result = 1;

	while (true) {
		if (Exponent & 1) {
			result *= Base;
		}
		Exponent >>= 1;
		if (Exponent == 0) {
			break;
		}
		Base *= Base;
	}
	return result;
}

template <typename TNumber>
struct Range {
	TNumber first;
	TNumber last;
	
	[[nodiscard]]
	constexpr bool Contains(
		_In_ const TNumber Index
	) const {
		return first <= Index && Index <= last;	
	}
};