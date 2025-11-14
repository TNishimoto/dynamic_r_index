#pragma once
#include "./position_information.hpp"
namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief PreprocessingResultForDeletion [Unchecked AI comment].
         * \ingroup DynamicRIndexes
         */
        struct PreprocessingResultForDeletion
        {
        public:
            PositionInformation ISA_v_PI;
            PositionInformation ISA_u_minus_PI;

            // SAIndex i = UINT64_MAX;
            //  uint8_t i_char = UINT8_MAX;
            // SAIndex j = UINT64_MAX;
            uint8_t old_char = UINT8_MAX;
            uint8_t new_char = UINT8_MAX;

            uint64_t LF_v = UINT64_MAX;
            // uint64_t v_on_sa = UINT64_MAX;
            // RunPosition v_on_rlbwt;

            // SAIndex i_minus = UINT64_MAX;
            // bool isSpecialLF = false;
        };
    }
}
