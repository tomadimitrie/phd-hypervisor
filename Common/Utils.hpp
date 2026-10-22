#pragma once 

#ifdef _KERNEL_MODE
# include <ntddk.h>
#else
# include <Windows.h>
#endif

#include "Utils/Bits.hpp"
#include "Utils/Enums.hpp"
#include "Utils/ErrorHandling.hpp"
#include "Utils/Math.hpp"
#include "Utils/Memory.hpp"
#include "Utils/Numeric.hpp"
#include "Utils/Templates.hpp"