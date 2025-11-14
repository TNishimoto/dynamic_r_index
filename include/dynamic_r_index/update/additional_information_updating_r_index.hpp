#pragma once
#include "../../dynamic_fm_index/tools.hpp"

namespace stool
{
    namespace dynamic_r_index
    {
        /**
         * @brief A structure that stores additional information needed during R-Index updates [Unchecked AI comment].
         * \ingroup DynamicRIndexes
         */
         struct AdditionalInformationUpdatingRIndex
        {
        public:
            SAIndex y;              ///< Position y in the suffix array
            //int64_t y_star;       
            SAValue value_at_y;     ///< Value stored at position y in suffix array
            SAValue value_at_y_star;///< Value stored at position y* in suffix array  
            SAValue value_at_y_minus;///< Value stored at position y-1
            SAValue value_at_y_plus;///< Value stored at position y+1
            SAValue value_at_z_minus;///< Value stored at position z-1
            SAValue value_at_z_plus;///< Value stored at position z+1
            SAIndex z;              ///< Position z in the suffix array

            /**
             * @brief Default constructor that initializes all members to UINT64_MAX
             */
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

            /**
             * @brief Prints the current state of the structure to standard output
             */
            void print() const
            {
                std::cout << "AdditionalInformationUpdatingRIndex[ y = " << y  << ", SA[y] = " << value_at_y   << ", z = " << z << std::endl;
                std::cout << "SA[y-1] = " << value_at_y_minus << ", SA[y+1] = " << value_at_y_plus << std::endl;
                std::cout << "SA[z-1] = " << value_at_z_minus << ", SA[z+1] = " << value_at_z_plus << " ]" << std::endl;
            }
        };
    }
}