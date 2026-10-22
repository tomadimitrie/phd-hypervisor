#pragma once

consteval const char* FileBasename(const char* Path) {
	const char* file = Path;
	for (const char* ptr = Path; *ptr != '\0'; ptr += 1) {
		if (*ptr == '/' || *ptr == '\\') {
			file = ptr + 1;
		}
	}
	return file;
}

#define FILENAME FileBasename(__FILE__)

#define PREAMBLE "{Hypervisor} "
#define LOG_HEADER_KEYS "%s@(%s:%d) "
#define LOG_HEADER_VALS __FUNCTION__, FILENAME, __LINE__

#define HvFuncFail(func, status) DbgPrint(PREAMBLE "[-] " LOG_HEADER_KEYS "%s failed with status 0x%x\n", LOG_HEADER_VALS, func, status)
#define HvFuncFailBool(func) DbgPrint(PREAMBLE "[-] " LOG_HEADER_KEYS "%s failed\n", LOG_HEADER_VALS, func)
#define HvFail(fmt, ...) DbgPrint(PREAMBLE "[-] " LOG_HEADER_KEYS fmt "\n", LOG_HEADER_VALS, ##__VA_ARGS__)
#define HvInfo(fmt, ...) DbgPrint(PREAMBLE "[*] " LOG_HEADER_KEYS fmt "\n", LOG_HEADER_VALS, ##__VA_ARGS__)
#define HvSuccess(fmt, ...) DbgPrint(PREAMBLE "[+] " LOG_HEADER_KEYS fmt "\n", LOG_HEADER_VALS, ##__VA_ARGS__)