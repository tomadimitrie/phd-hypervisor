#pragma once

#define RESOLVE_FUNCTION(name) \
	decltype(name)* (name) = []() { \
        auto wstr = ENC_WSTR(WSTRINGIFY(name)); \
		UNICODE_STRING ustr = { 0 }; \
		RtlInitUnicodeString(&ustr, wstr); \
		return bit_cast<decltype(name)>(MmGetSystemRoutineAddress(&ustr)); \
	}();

struct DriverFunctions {

};

struct DriverData {
	DriverFunctions functions;
};

extern DriverData g_data;