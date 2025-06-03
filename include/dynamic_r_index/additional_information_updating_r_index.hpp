#pragma once
#include "./dynamic_rlbwt_helper.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        struct AdditionalInformationUpdatingRIndex
        {
        public:
            SAIndex y;
            int64_t y_star;
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
                this->y_star = INT64_MAX;
                this->value_at_y = UINT64_MAX;
                this->value_at_y_star = UINT64_MAX;
                this->value_at_y_minus = UINT64_MAX;
                this->value_at_y_plus = UINT64_MAX;
                this->value_at_z_minus = UINT64_MAX;
                this->value_at_z_plus = UINT64_MAX;
                this->z = UINT64_MAX;
            }
            void print() const
            {
                std::cout << "AdditionalInformationUpdatingRIndex[ y = " << y << ", j* = " << y_star << ", SA[y] = " << value_at_y << ", SA[y*] = " << value_at_y_star << ", z = " << z << std::endl;
                std::cout << "SA[y-1] = " << value_at_y_minus << ", SA[y+1] = " << value_at_y_plus << std::endl;
                std::cout << "SA[z-1] = " << value_at_z_minus << ", SA[z+1] = " << value_at_z_plus << " ]" << std::endl;

            }
            /*
                        SAIndex get_z() const
            {
                throw std::logic_error("Error: get_z");
                //return _z;
                
                if (y_star < (int64_t)y)
                {
                    return y_star + 1;
                }
                else
                {
                    return y_star;
                }                
            }
            */
            
            /*
            SAIndex get_z2() const
            {
                return z2;
            }
            */

            void compute_and_set_y_and_y_star(int64_t _y, int64_t _z)
            {
                this->y = _y;
                this->y_star = _y < _z ? _z : (_z - 1);
                
                if (y_star < (int64_t)y)
                {
                    this->z = y_star + 1;
                }
                else
                {
                    this->z = y_star;
                }


            }
        };
    }
}