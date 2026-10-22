#pragma once

#include "Numeric.hpp"
#include <sal.h>

enum class BitOrder : u8 {
    Lsb0,
    Msb0,
};

template <BitOrder TBitOrder = BitOrder::Lsb0>
struct BitView {
    u8* bytes;
    u64 bitCount;
    
    BitView(
        void* Data,
        const u64 SizeInBytes
    ) noexcept :
        bytes(static_cast<u8*>(Data)),
        bitCount(SizeInBytes * 8)
    {}
    
    bool Test(
        _In_ const u64 Bit
    ) const noexcept {
        return Bit < bitCount && (bytes[Bit / 8] & Mask(Bit)) != 0;
    }
    
    bool Set(
        _In_ const u64 Bit,
        _In_ const bool Value
    ) const noexcept {
        if (Bit >= bitCount) {
            return false;
        }
        if (Value) {
            bytes[Bit / 8] |= Mask(Bit);
        } else {
            bytes[Bit / 8] &= ~Mask(Bit);
        }
        return true;
    }
    
    bool Clear(
        _In_ const u64 Bit
    ) const noexcept {
       return Set(Bit, false); 
    }
    
    bool Toggle(
        _In_ const u64 Bit
    ) const noexcept {
        if (Bit >= bitCount) {
            return false;
        }
        bytes[Bit / 8] ^= Mask(Bit);
        return true;
    }
private:
    static constexpr u8 Mask(
        _In_ const u64 Bit
    ) noexcept {
        const u32 position = static_cast<u32>(Bit % 8);
        const u32 shift = TBitOrder == BitOrder::Lsb0 ? position : 7 - position;
        return static_cast<u8>(1 << shift);
    }
};