#pragma once
#include <cstdint>
struct MK64Input { int8_t stick_x=0, stick_y=0; bool A=false,B=false,Z=false,R=false,L=false; };
namespace InputGen { MK64Input random(uint64_t& seed); }
