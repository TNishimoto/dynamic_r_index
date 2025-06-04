#pragma once
#include "../../dynamic_fm_index/tools.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        struct AdditionalInformationUpdatingRIndex
        {
        public:
            SAIndex y;
            //int64_t y_star;
            SAValue value_at_y;
            SAValue value_at_y_star;
            SAValue value_at_y_minus;
            SAValue value_at_y_plus;
            SAValue value_at_z_minus;
            SAValue value_at_z_plus;
            SAIndex z;

            AdditionalInformationUpdatingRIndex()
            {
                this->y = UINT64_MAX;
                //this->y_star = INT64_MAX;
                this->value_at_y = UINT64_MAX;
                //this->value_at_y_star = UINT64_MAX;
                this->value_at_y_minus = UINT64_MAX;
                this->value_at_y_plus = UINT64_MAX;
                this->value_at_z_minus = UINT64_MAX;
                this->value_at_z_plus = UINT64_MAX;
                this->z = UINT64_MAX;
            }
            void print() const
            {
                std::cout << "AdditionalInformationUpdatingRIndex[ y = " << y  << ", SA[y] = " << value_at_y   << ", z = " << z << std::endl;
                std::cout << "SA[y-1] = " << value_at_y_minus << ", SA[y+1] = " << value_at_y_plus << std::endl;
                std::cout << "SA[z-1] = " << value_at_z_minus << ", SA[z+1] = " << value_at_z_plus << " ]" << std::endl;

            }
            
        };
    }
}