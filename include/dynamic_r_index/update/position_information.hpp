#pragma once
#include "../../dynamic_fm_index/tools.hpp"
namespace stool
{
    namespace dynamic_r_index
    {

        struct PositionInformation
        {
            uint64_t p = UINT64_MAX;
            SAValue value_at_p = UINT64_MAX;
            SAValue value_at_p_minus = UINT64_MAX;
            SAValue value_at_p_plus = UINT64_MAX;

            void update_for_deletion(uint64_t removed_position_on_text, SAIndex removed_position_on_sa, uint64_t upper_sa_value, uint64_t lower_sa_value)
            {
                if (this->value_at_p > removed_position_on_text)
                {
                    this->value_at_p--;
                }
                if (removed_position_on_sa < this->p)
                {
                    this->p--;
                }
                if (this->value_at_p_minus == removed_position_on_text)
                {
                    this->value_at_p_minus = upper_sa_value;
                }
                if (this->value_at_p_minus > removed_position_on_text)
                {
                    this->value_at_p_minus--;
                }

                if (this->value_at_p_plus == removed_position_on_text)
                {
                    this->value_at_p_plus = lower_sa_value;
                }
                if (this->value_at_p_plus > removed_position_on_text)
                {
                    this->value_at_p_plus--;
                }
            }
        };
    }
}
