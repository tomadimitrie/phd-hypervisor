#pragma once

#include "Common/Utils.hpp"

namespace Crt {
    Expected<void> Init();
    
    void Uninit();
}