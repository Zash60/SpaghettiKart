#include "InputGen.h"
#include "Utils.h"
namespace InputGen {
MK64Input random(uint64_t& seed){
    Utils::xoro_r(&seed);
    MK64Input i;
    i.stick_x = int8_t((seed % 161) - 80);
    Utils::xoro_r(&seed);
    i.stick_y = int8_t((seed % 41) - 20);
    Utils::xoro_r(&seed);
    i.A = (seed % 100) < 95; Utils::xoro_r(&seed);
    i.R = (seed % 100) < 10; Utils::xoro_r(&seed);
    i.B = (seed % 100) < 2; Utils::xoro_r(&seed);
    i.Z = false; i.L = false;
    return i;
}
}
