#pragma once
#include "./position_information.hpp"
namespace stool
{
    namespace dynamic_r_index
    {
        struct PreprocessingResultForInsertion
        {
        public:
            PositionInformation ISA_i_PI;
            SAValue value_at_y_minus = UINT64_MAX;
            SAValue value_at_y_plus = UINT64_MAX;
            uint8_t old_char = UINT8_MAX;
            SAIndex i_minus = UINT64_MAX;
            //bool isSpecialLF = false;
        };
    }
}
